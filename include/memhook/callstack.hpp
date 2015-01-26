#ifndef MEMHOOK_CALLSTACK_HPP_INCLUDED
#define MEMHOOK_CALLSTACK_HPP_INCLUDED

#include "common.hpp"
#include <boost/container/string.hpp>
#include <boost/container/vector.hpp>

namespace memhook
{

struct callstack_record
{
    container::string shl_path;
    container::string procname;
    uintptr_t shl_addr;
    uintptr_t ip;
    uintptr_t sp;
    uintptr_t offp;
};

typedef container::vector<callstack_record> callstack_container;

} // memhook

#endif // MEMHOOK_CALLSTACK_HPP_INCLUDED
