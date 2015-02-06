#!/bin/sh
set -e

gdb -ex "set env MEMHOOK_NET_HOST=127.0.0.1" \
    -ex "set exec-wrapper env 'LD_PRELOAD=$1'" \
    -ex "set follow-fork-mode child" \
    ${*:2}
