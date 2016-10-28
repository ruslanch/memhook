#!/bin/bash

set -e

if [ -z "$SCRIPT_ROOT" ]; then
  SCRIPT_ROOT="$(readlink -f `dirname $0`)"
fi

. "$SCRIPT_ROOT/functions.sh"
. "$SCRIPT_ROOT/variables.sh"

pushd "$BUILD_ROOT"

if [ -z "$BOOST_BUILD_VARIANT" ]; then
  BOOST_BUILD_VARIANT=release
fi

BOOST_URL="http://sourceforge.net/projects/boost/files/boost/$BOOST_VERSION/boost_${BOOST_VERSION//./_}.tar.bz2"

rm_nofail "$BOOST_BUILD_ROOT"
fetch_unpack_file "$BOOST_URL" && pushd "$BOOST_BUILD_ROOT"

./bootstrap.sh --without-icu

echo "using gcc : $TOOLCHAIN_ARCH : $TOOLCHAIN_HOST-g++ : <root>$TOOLCHAIN_ROOT <compileflags>-fPIC ;" \
    >> project-config.jam
BOOST_B2_ARGS=toolset="gcc-$TOOLCHAIN_ARCH"

"$BOOST_BUILD_ROOT/b2" -sBOOST_ROOT="$BOOST_BUILD_ROOT" $BOOST_B2_ARGS \
    link=static runtime-link=shared threading=multi variant=$BOOST_BUILD_VARIANT \
    --host=$TOOLCHAIN_HOST --with-system --with-thread --with-chrono --with-filesystem --with-program_options

popd # BOOST_BUILD_ROOT
popd # BUILD_ROOT
