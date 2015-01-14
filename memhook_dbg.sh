#!/bin/sh

set -xe

gdb -ex "set env MEMHOOK_FILE=./debug.memhook" \
    -ex "set env MEMHOOK_SIZE_GB=2" \
    -ex "set exec-wrapper env 'LD_PRELOAD=./libmemhook.so.1.0'" \
    -ex "set follow-fork-mode child" \
    $@
