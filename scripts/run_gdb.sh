#!/bin/bash
set -e

gdb -ex "set env MEMHOOK_FILE=memhook.db" \
    -ex "set env MEMHOOK_LIBUNWIND_CACHE_POLICY=n" \
    -ex "set exec-wrapper env 'LD_PRELOAD=$1'" \
    -ex "set follow-fork-mode child" \
    ${*:2}
