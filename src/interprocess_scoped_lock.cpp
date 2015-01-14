#include "interprocess_scoped_lock.hpp"

namespace memhook {
namespace mapped_view_detail {

static bool volatile stop_flag = false;
static void on_ctrlc_signal(int, siginfo_t *, void *) BOOST_NOEXCEPT_OR_NOTHROW {
    stop_flag = true;
}

bool is_interrupted() {
    return stop_flag;
}

} // mapped_view_detail

interprocess_scoped_lock::interprocess_scoped_lock(interprocess::interprocess_mutex &mutex, bool no_lock)
        : signal_block_(SIGINT)
        , signal_(SIGINT, &mapped_view_detail::on_ctrlc_signal)
        , plock_() {
    if (!no_lock)
        plock_.reset(new interprocess_mutex_scoped_lock(mutex));
}

} // memhook
