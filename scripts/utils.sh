#!/bin/sh
set -e

function fetch_unpack_file {
    local file_url=$1
    local filename=
    if (( $# >= 2 ));      then filename=$2; fi
    if [ -z "$filename" ]; then filename=$(basename "$file_url"); fi
    if [ -f "$filename" ]; then rm -vf "$filename"; fi
    echo "Downloading and unpacking file $filename from url $file_url..."
    wget "$file_url" -O "$filename" && tar -xpf "$filename" && rm -f "$filename"
    echo "...downloading and unpacking file $filename from url $file_url completed OK"
}
export -f fetch_unpack_file

function rm_nofail {
    local path=$1
    echo "Removing $path..."
    if [ -d "$path" ]; then
        rm -rf "$path"
    elif [ -f "$path" ]; then
        rm -f "$path"
    fi
    echo "...removing $path completed OK"
}
export -f rm_nofail

function mk_dir {
    local path=$1
    echo "Create directory $path"
    mkdir -p "$path"
}
export -f mk_dir

function clean_dir {
    local path=$1
    echo "Cleaning $path..."
    if [ -d "$path" ]; then
        find "$path" -maxdepth 1 -mindepth 1 -exec /bin/sh -c 'rm_nofail "$0"' {} \;
    else
        mkdir -p "$path"
    fi
    echo "...cleaning $path completed OK"
}
export -f clean_dir
