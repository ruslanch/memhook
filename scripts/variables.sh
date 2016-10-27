#!/bin/bash

set -e

if [ -z "$TOOLCHAIN_HOST" ]; then
    TOOLCHAIN_HOST=$(gcc -dumpmachine)
fi

TOOLCHAIN_ARCH=$(echo ${TOOLCHAIN_HOST//-/ } | awk '{ print $1; }')

if [ -z "$BUILD_ROOT" ]; then
  BUILD_ROOT="$PWD/$TOOLCHAIN_HOST"
fi

if [ ! -d "$BUILD_ROOT" ]; then
  mk_dir "$BUILD_ROOT"
fi

if [ -z "$BOOST_VERSION" ]; then
  BOOST_VERSION=1.61.0
fi

if [ -z "$BOOST_BUILD_ROOT" ]; then
  BOOST_BUILD_ROOT="$BUILD_ROOT/boost_${BOOST_VERSION//./_}"
fi

if [ -z "$MEMHOOK_BUILD_ROOT" ]; then
  MEMHOOK_BUILD_ROOT="$BUILD_ROOT/memhook"
fi

if [ -z "$LIBUNWIND_BUILD_ROOT" ]; then
  LIBUNWIND_BUILD_ROOT="$BUILD_ROOT/libunwind"
fi

if [ -z "$LEVELDB_BUILD_ROOT" ]; then
    LEVELDB_BUILD_ROOT="$BUILD_ROOT/leveldb"
fi
