#!/bin/sh
set -e

BOOST_URL="http://sourceforge.net/projects/boost/files/boost/$BOOST_VERSION/boost_${BOOST_VERSION//./_}.tar.bz2"

rm_nofail "$BOOST_BUILD_ROOT"
fetch_unpack_file "$BOOST_URL" && pushd "$BOOST_BUILD_ROOT"
./bootstrap.sh --without-icu
./b2 tools/bcp

clean_dir "$BOOST_PRIVATE_BUILD_ROOT"
./dist/bin/bcp --boost=`pwd` \
    build config system thread chrono program_options inspect wave filesystem \
    atomic interprocess fusion multi_index range spirit ptr_container scope_exit \
    typeof lambda asio context coroutine --namespace=boost_private \
    "$BOOST_PRIVATE_BUILD_ROOT" > /dev/null && pushd "$BOOST_PRIVATE_BUILD_ROOT"

cp -vf "$BOOST_BUILD_ROOT/boostcpp.jam" "$BOOST_PRIVATE_BUILD_ROOT/boostcpp.jam"

if [ ! "$TOOLCHAIN_HOST" = "" ]; then
    echo "using gcc : $TOOLCHAIN_ARCH : $TOOLCHAIN_HOST-g++ : <root>$TOOLCHAIN_ROOT <compileflags>-fPIC ;" \
        >> project-config.jam
    BOOST_B2_ARGS=toolset="gcc-$TOOLCHAIN_ARCH"
fi

"$BOOST_BUILD_ROOT/b2" -sBOOST_ROOT="$BOOST_BUILD_ROOT" $BOOST_B2_ARGS \
    link=static runtime-link=shared threading=multi variant=release \
    --host=x86_64-pc-linux-gnu --with-system --with-thread --with-chrono \
    --with-program_options

popd # BOOST_PRIVATE_BUILD_ROOT
popd # BOOST_BUILD_ROOT
