#!/bin/bash
set -e

gdb -ex "set env MEMHOOK_FILE=memhook.db" \
    -ex "set exec-wrapper env 'LD_PRELOAD=$1'" \
    -ex "set follow-fork-mode child" \
    ${*:2}
