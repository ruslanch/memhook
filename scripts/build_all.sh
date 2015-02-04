#!/bin/sh
set -e

export SCRIPT_ROOT="$(readlink -f `dirname $0`)"
source "$SCRIPT_ROOT/utils.sh"

if [ "$TOOLCHAIN_HOST" = "" ]; then
    export TOOLCHAIN_HOST=$(gcc -dumpmachine)
else
    export TOOLCHAIN_ROOT="/usr/toolchain/${TOOLCHAIN_HOST}_gcc4.4.3_glibc2.5"
    export PATH="$TOOLCHAIN_ROOT/bin:$PATH"
fi
export TOOLCHAIN_ARCH=$(echo ${TOOLCHAIN_HOST//-/ } | awk '{ print $1; }')

export BUILD_ROOT="$PWD/$TOOLCHAIN_HOST"
mk_dir "$BUILD_ROOT"
pushd  "$BUILD_ROOT"

export BOOST_VERSION=1.57.0
export BOOST_BUILD_ROOT="$BUILD_ROOT/boost_${BOOST_VERSION//./_}"
export BOOST_PRIVATE_BUILD_ROOT="${BOOST_BUILD_ROOT}_private"
export LIBUNWIND_BUILD_ROOT="$BUILD_ROOT/libunwind"
export MEMHOOK_BUILD_ROOT="$BUILD_ROOT/memhook"

"$SCRIPT_ROOT/build_boost.sh"
"$SCRIPT_ROOT/build_libunwind.sh"
"$SCRIPT_ROOT/build_memhook.sh"

popd # BUILD_ROOT
