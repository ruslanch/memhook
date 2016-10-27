#ifndef MEMHOOK_INCLUDE_CALLSTACK_H_INCLUDED
#define MEMHOOK_INCLUDE_CALLSTACK_H_INCLUDED

#include <memhook/common.h>

#include <boost/container/string.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace memhook
{

struct CallStackInfoItem
{
    boost::container::string shl_path;
    boost::container::string procname;
    uintptr_t shl_addr;
    uintptr_t ip;
    uintptr_t sp;
    uintptr_t offp;

    CallStackInfoItem()
        : shl_path()
        , procname()
        , shl_addr()
        , ip()
        , sp()
        , offp()
    {}

    CallStackInfoItem(const CallStackInfoItem &item)
        : shl_path(item.shl_path)
        , procname(item.procname)
        , shl_addr(item.shl_addr)
        , ip(item.ip)
        , sp(item.sp)
        , offp(item.offp)
    {}

    CallStackInfoItem(BOOST_RV_REF(CallStackInfoItem) item)
        : shl_path(boost::move(item.shl_path))
        , procname(boost::move(item.procname))
        , shl_addr(item.shl_addr)
        , ip(item.ip)
        , sp(item.sp)
        , offp(item.offp)
    {}

    CallStackInfoItem &operator=(const CallStackInfoItem &item)
    {
        CallStackInfoItem(item).swap(*this);
        return *this;
    }

    CallStackInfoItem &operator=(BOOST_RV_REF(CallStackInfoItem) item)
    {
        item.swap(*this);
        return *this;
    }

    void swap(CallStackInfoItem &item)
    {
        using std::swap;

        shl_path.swap(item.shl_path);
        procname.swap(item.procname);
        swap(shl_addr, item.shl_addr);
        swap(ip, item.ip);
        swap(sp, item.sp);
        swap(offp, item.offp);
    }

    BOOST_COPYABLE_AND_MOVABLE(CallStackInfoItem);
};

typedef boost::container::vector<CallStackInfoItem> CallStackInfo;

} // memhook

BOOST_FUSION_ADAPT_STRUCT(
    memhook::CallStackInfoItem,
    (boost::container::string, shl_path)
    (boost::container::string, procname)
    (uintptr_t, shl_addr)
    (uintptr_t, ip)
    (uintptr_t, sp)
    (uintptr_t, offp)
);

#endif
