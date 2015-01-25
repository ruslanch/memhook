#include "mapped_view_base.hpp"
#include "mapped_view_req.hpp"
#include <boost/bind.hpp>
#include <boost/range/algorithm.hpp>
#include <cxxabi.h>
#include <signal.h>

namespace memhook {
namespace mapped_view_detail {
    static bool volatile stop_flag = false;
    void on_ctrlc_signal(int, siginfo_t *, void *) BOOST_NOEXCEPT_OR_NOTHROW {
        stop_flag = true;
    }
} // mapped_view_detail

void unique_char_buf_null_free(const char *) BOOST_NOEXCEPT_OR_NOTHROW {
    // nothing
}

void unique_char_buf_full_free(const char *mem) BOOST_NOEXCEPT_OR_NOTHROW {
    free(const_cast<char *>(mem));
}

unique_char_buf_t cxx_symbol_demangle(const char *source) {
    int ret = 0;
    unique_char_buf_t res(abi::__cxa_demangle(source, NULL, NULL, &ret),
            unique_char_buf_full_free);
    if (ret == 0 && res)
        return move(res);
    return unique_char_buf_t(source, unique_char_buf_null_free);
}

mapped_view_base::mapped_view_base()
    : options_(opt_sort_by_time)
{}

bool mapped_view_base::get_option(uint32_t option) const {
    return !!(options_ & option);
}

void mapped_view_base::set_option(uint32_t option, bool setval) {
    const uint32_t sortmask = 0x00f0;
    if (option & sortmask)
        options_ &= ~sortmask;

    if (setval)
        options_ |= option;
    else
        options_ &= ~option;
}

void mapped_view_base::add_req(mapped_view_req *req) {
    reqs_.push_back(req);
}

bool mapped_view_base::check_traceinfo_reqs(const traceinfo_base &tinfo) const {
    return find_if(reqs_, !bind(&mapped_view_req::invoke, _1, cref(tinfo))) == reqs_.end();
}

bool mapped_view_base::is_interrupted() {
    return mapped_view_detail::stop_flag;
}

} // memhook
