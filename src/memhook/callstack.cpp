#include <memhook/common.hpp>
#include <memhook/callstack.hpp>
#include "scoped_use_count.hpp"
#include "error_msg.hpp"
#include <boost/unordered_map.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/array.hpp>
#include <boost/scope_exit.hpp>

#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <dlfcn.h>

#define MEMHOOK_CHECK_UNW(call) \
    if (BOOST_UNLIKELY(call != 0)) { error_msg(#call " failed"); }

namespace memhook
{

struct procname_info {
    string_t   procname;
    unw_word_t offp;
    unw_word_t shl_addr;
    procname_info() : procname(), offp(), shl_addr() {}
};

typedef unordered_map<unw_word_t, procname_info> procname_info_map_t;
typedef unordered_map<unw_word_t, string_t> shl_path_map_t;

struct callstack_internal {
    mutex               map_mutex;
    procname_info_map_t procname_info_map;
    shl_path_map_t      shl_path_map;
};

static callstack_internal *callstack_pctx = NULL;
static ssize_t             callstack_pctx_use_count = 0;
static const char unknown_tag[] = "<unknown>";

void init_callstack() {
    MEMHOOK_CHECK_UNW(unw_set_caching_policy(unw_local_addr_space, UNW_CACHE_PER_THREAD));
    movelib::unique_ptr<callstack_internal> ctx(movelib::make_unique<callstack_internal>());
    MEMHOOK_CAS(&callstack_pctx, NULL, ctx.get());
    ctx.release();
}

void fini_callstack() {
    movelib::unique_ptr<callstack_internal> ctx(MEMHOOK_CAS(&callstack_pctx, callstack_pctx, NULL));
    if (ctx) {
        while (MEMHOOK_CAS(&callstack_pctx_use_count, 0, 0) != 0)
            pthread_yield();
    }
}

static void get_callstack_procinfo(callstack_internal *ctx, unw_word_t ip, unw_cursor_t cursor,
        string_t &shl_path, unw_word_t &shl_addr, string_t &procname, unw_word_t &offp) {
    unique_lock<mutex> lock(ctx->map_mutex);
    procname_info_map_t::const_iterator iter = ctx->procname_info_map.find(ip);
    if (iter != ctx->procname_info_map.end()) {
        shl_path_map_t::const_iterator iter2 = ctx->shl_path_map.find(iter->second.shl_addr);
        if (iter2 != ctx->shl_path_map.end())
            shl_path = iter2->second;
        else
            shl_path = unknown_tag;
        lock.unlock();
        shl_addr = iter->second.shl_addr;
        procname = iter->second.procname;
        offp     = iter->second.offp;
    } else {
        lock.unlock();
        array<char, 1024> buf;
        procname_info pni;
        if (unw_get_proc_name(&cursor, buf.data(), buf.size(), &offp) == 0) {
            pni.procname = buf.data();
            pni.offp     = offp;
        }
        else
            pni.procname = unknown_tag;
        lock.lock();

        Dl_info dl_info;
        if (::dladdr(reinterpret_cast<void *>(ip), &dl_info) != 0) {
            pni.shl_addr = (unw_word_t)dl_info.dli_fbase;
            if (dl_info.dli_fname == NULL)
                dl_info.dli_fname = unknown_tag;
            shl_path = dl_info.dli_fname;
            ctx->shl_path_map.emplace(pni.shl_addr, shl_path);
        } else {
            shl_path = unknown_tag;
        }

        shl_addr = pni.shl_addr;
        procname = pni.procname;
        offp     = pni.offp;

        ctx->procname_info_map.emplace(ip, pni);
    }
}

void get_callstack(callstack_container &callstack) {
    if (BOOST_UNLIKELY(MEMHOOK_CAS(&callstack_pctx, NULL, NULL) == NULL))
        return;

    const scoped_use_count use_count(&callstack_pctx_use_count);
    callstack_internal *const ctx = MEMHOOK_CAS(&callstack_pctx, NULL, NULL);
    if (BOOST_UNLIKELY(ctx == NULL))
        return;

    callstack.clear();
    callstack.reserve(16);

    unw_cursor_t cursor; unw_context_t uc;
    unw_word_t ip, sp, offp, shl_addr;
    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);
    if (unw_step(&cursor) <= 0) // skip first
        return;

    string_t shl_path, procname;
    while (unw_step(&cursor) > 0) {
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        if (BOOST_UNLIKELY(ip == 0))
            break;

        unw_get_reg(&cursor, UNW_REG_SP, &sp);
        get_callstack_procinfo(ctx, ip, cursor, shl_path, shl_addr, procname, offp);
        callstack.push_back(callstack_record());
        callstack.back().shl_path.swap(shl_path);
        callstack.back().procname.swap(procname);
        callstack.back().shl_addr = shl_addr;
        callstack.back().ip       = ip;
        callstack.back().sp       = sp;
        callstack.back().offp     = offp;
    }
}

} // memtrace
