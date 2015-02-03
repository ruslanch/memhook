#!/bin/sh

set -xe

TOOLCHAIN_ARCH=x86_64
TOOLCHAIN_HOST="$TOOLCHAIN_ARCH-pc-linux-gnu"
TOOLCHAIN_ID="${TOOLCHAIN_HOST}_gcc4.4.3_glibc2.5"
TOOLCHAIN_ROOT="/usr/toolchain/$TOOLCHAIN_ID"
export PATH="$TOOLCHAIN_ROOT/bin:$PATH"

BOOST_VERSION=1.57.0
BOOST_URL="http://sourceforge.net/projects/boost/files/boost/$BOOST_VERSION/boost_${BOOST_VERSION//./_}.tar.bz2"
LIBUNWIND_GIT_URL="git://git.sv.gnu.org/libunwind.git"
LIBUNWIND_GIT_BRANCH="master"
MEMHOOK_DIR="$(readlink -f `dirname $0`/..)"
PATCHES_DIR="$(readlink -f `dirname $0`/../patches)"

BUILD_ROOT="$PWD/$TOOLCHAIN_HOST"
rm -rvf "$BUILD_ROOT"
mkdir -p "$BUILD_ROOT"

LIBUNWIND_BUILD_ROOT="$BUILD_ROOT/libunwind"
BOOST_BUILD_ROOT="$BUILD_ROOT/boost_${BOOST_VERSION//./_}"

function fetch_unpack_file {
    set +x
    local file_url=$1
    local filename=
    if (( $# >= 2 ));      then filename=$2; fi
    if [ -z "$filename" ]; then filename=$(basename "$file_url"); fi
    if [ -f "$filename" ]; then rm -vf "$filename"; fi
    wget "$file_url" -O "$filename" && tar -xvpf "$filename" && rm -vf "$filename"
    set -x
}

pushd "$BUILD_ROOT"

git clone "$LIBUNWIND_GIT_URL" "$LIBUNWIND_BUILD_ROOT" && pushd "$LIBUNWIND_BUILD_ROOT"
git checkout $LIBUNWIND_GIT_BRANCH
git apply "$PATCHES_DIR/libunwind/$LIBUNWIND_GIT_BRANCH/dwarf-memleak.patch"
CC=${TOOLCHAIN_HOST}-gcc ./autogen.sh
CC=${TOOLCHAIN_HOST}-gcc ./configure --enable-static --disable-shared --with-pic
CC=${TOOLCHAIN_HOST}-gcc make -j
popd # LIBUNWIND_BUILD_ROOT

fetch_unpack_file "$BOOST_URL" && pushd "$BOOST_BUILD_ROOT"
./bootstrap.sh --without-icu
./b2 tools/bcp
BOOST_PRIVATE_BUILD_ROOT="${BOOST_BUILD_ROOT}_private"
mkdir -pv "$BOOST_PRIVATE_BUILD_ROOT"
./dist/bin/bcp --boost=`pwd` \
    build config system thread chrono program_options inspect wave filesystem \
    atomic interprocess fusion multi_index range spirit ptr_container scope_exit \
    typeof lambda asio context coroutine \
    --namespace=boost_private \
    "$BOOST_PRIVATE_BUILD_ROOT" && pushd "$BOOST_PRIVATE_BUILD_ROOT"
cp -vf "$BOOST_BUILD_ROOT/boostcpp.jam" "$BOOST_PRIVATE_BUILD_ROOT/boostcpp.jam"
echo "using gcc : $TOOLCHAIN_ARCH : $TOOLCHAIN_HOST-g++ : <root>$TOOLCHAIN_ROOT <compileflags>-fPIC ;" \
   >> project-config.jam
"$BOOST_BUILD_ROOT/b2" -sBOOST_ROOT="$BOOST_BUILD_ROOT" \
    toolset="gcc-$TOOLCHAIN_ARCH" \
    link=static \
    runtime-link=shared \
    threading=multi \
    variant=release \
    --host=x86_64-pc-linux-gnu \
    --with-system \
    --with-thread \
    --with-chrono \
    --with-program_options
popd # BOOST_PRIVATE_BUILD_ROOT
popd # BOOST_BUILD_ROOT

MEMHOOK_BUILD_ROOT="${BUILD_ROOT}/memhook"
mkdir -pv $MEMHOOK_BUILD_ROOT && pushd "$MEMHOOK_BUILD_ROOT"
cat <<EOF >./toolchain.cmake
SET(TOOLCHAIN_HOST $TOOLCHAIN_HOST)
SET(TOOLCHAIN_ROOT $TOOLCHAIN_ROOT)
SET(CMAKE_SYSTEM_PROCESSOR $TOOLCHAIN_ARCH)
SET(CMAKE_SYSTEM_NAME    Linux)
SET(CMAKE_SYSTEM_VERSION 2.6.18)
SET(CMAKE_C_COMPILER   ${TOOLCHAIN_HOST}-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_HOST}-c++)
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_ROOT} $BUILD_ROOT)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF
cmake -GNinja $MEMHOOK_DIR \
    -DBOOST_ROOT="$BOOST_PRIVATE_BUILD_ROOT" \
    -DLIBUNWIND_ROOT="$LIBUNWIND_BUILD_ROOT" \
    -DCMAKE_BUILD_TYPE=Release \
    -DMEMHOOK_USE_BOOST_PRIVATE=1 \
    -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake && ninja
popd # MEMHOOK_BUILD_ROOT

popd # BUILD_ROOT
