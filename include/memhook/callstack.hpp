#ifndef MEMHOOK_CALLSTACK_HPP_INCLUDED
#define MEMHOOK_CALLSTACK_HPP_INCLUDED

#include "common.hpp"
#include <boost/container/string.hpp>
#include <boost/container/vector.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace memhook
{

struct callstack_record
{
    boost::container::string shl_path;
    boost::container::string procname;
    uintptr_t shl_addr;
    uintptr_t ip;
    uintptr_t sp;
    uintptr_t offp;
};

typedef boost::container::vector<callstack_record> callstack_container;

} // memhook

BOOST_FUSION_ADAPT_STRUCT(
    memhook::callstack_record,
    (boost::container::string, shl_path)
    (boost::container::string, procname)
    (uintptr_t, shl_addr)
    (uintptr_t, ip)
    (uintptr_t, sp)
    (uintptr_t, offp)
);

#endif // MEMHOOK_CALLSTACK_HPP_INCLUDED
