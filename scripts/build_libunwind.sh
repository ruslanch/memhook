#!/bin/bash
set -xe

if [ -z "$SCRIPT_ROOT" ]; then
  SCRIPT_ROOT="$(readlink -f `dirname $0`)"
fi

. "$SCRIPT_ROOT/functions.sh"
. "$SCRIPT_ROOT/variables.sh"

pushd "$BUILD_ROOT"

LIBUNWIND_GIT_URL="git://git.sv.gnu.org/libunwind.git"
LIBUNWIND_GIT_BRANCH="master"
LIBUNWIND_PATCHES_DIR="$(readlink -f $SCRIPT_ROOT/../patches/libunwind)"

rm_nofail "$LIBUNWIND_BUILD_ROOT"
git clone "$LIBUNWIND_GIT_URL" "$LIBUNWIND_BUILD_ROOT" && pushd "$LIBUNWIND_BUILD_ROOT"
git checkout $LIBUNWIND_GIT_BRANCH
git apply "$LIBUNWIND_PATCHES_DIR/$LIBUNWIND_GIT_BRANCH/dwarf-memleak.patch"

LIBUNWIND_CC=gcc
if [ -n "$TOOLCHAIN_HOST" ]; then
    LIBUNWIND_CC=$TOOLCHAIN_HOST-$LIBUNWIND_CC
fi

CUSTOM_FLAGS="-fno-builtin-malloc -fno-builtin-free -fno-builtin-realloc -fno-builtin-calloc"
CUSTOM_FLAGS="$CUSTOM_FLAGS -fno-builtin-cfree -fno-builtin-memalign -fno-builtin-posix_memalign"
CUSTOM_FLAGS="$CUSTOM_FLAGS -fno-builtin-valloc -fno-builtin-pvalloc"

CC=$LIBUNWIND_CC ./autogen.sh
CC=$LIBUNWIND_CC CFLAGS="-O2 $CUSTOM_FLAGS" ./configure --enable-static --disable-shared --with-pic
CC=$LIBUNWIND_CC make -j

popd # LIBUNWIND_BUILD_ROOT
popd # BUILD_ROOT
