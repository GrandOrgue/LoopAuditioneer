#!/bin/sh
# simple script to pass configure command with quotes while cross compiling libsndfile
# first argument should be cmake_source_dir and the second cmake_binary_dir
$1/submodules/libsndfile/configure --host=i686-w64-mingw32 --prefix=$2 --enable-static --disable-external-libs --disable-full-suite --disable-sqlite --disable-shared LIBS="-Wl,--as-needed -lssp"

