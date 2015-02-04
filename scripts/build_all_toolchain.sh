#!/bin/sh
TOOLCHAIN_HOST="x86_64-pc-linux-gnu" "$(readlink -f `dirname $0`)/build_all.sh"
