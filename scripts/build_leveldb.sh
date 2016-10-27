#!/bin/bash

set -xe

if [ -z "$SCRIPT_ROOT" ]; then
  SCRIPT_ROOT="$(readlink -f `dirname $0`)"
fi

. "$SCRIPT_ROOT/functions.sh"
. "$SCRIPT_ROOT/variables.sh"

pushd "$BUILD_ROOT"

LEVELDB_GIT_URL="git://github.com/google/leveldb.git"
LEVELDB_GIT_BRANCH="master"
LEVELDB_PATCHES_DIR="$(readlink -f $SCRIPT_ROOT/../patches/leveldb)"

rm_nofail "$LEVELDB_BUILD_ROOT"
git clone "$LEVELDB_GIT_URL" "$LEVELDB_BUILD_ROOT" && pushd "$LEVELDB_BUILD_ROOT"
git checkout $LEVELDB_GIT_BRANCH

LEVELDB_CC=gcc
LEVELDB_CXX=g++
if [ -n "$TOOLCHAIN_HOST" ]; then
    LEVELDB_CC=$TOOLCHAIN_HOST-$LEVELDB_CC
    LEVELDB_CXX=$TOOLCHAIN_HOST-$LEVELDB_CXX
fi

CC=$LEVELDB_CC CXX=$LEVELDB_CXX make -j
