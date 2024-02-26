#!/bin/bash

set -e

sudo apt update

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake g++ pkg-config \
  libasound2-dev libpulse-dev libwxgtk3.0-gtk3-dev gcc-mingw-w64-x86-64 \
  g++-mingw-w64-x86-64 autoconf autogen automake build-essential \
  libtool python mingw-w64-tools dpkg-dev

export PKG_CONFIG_LIBDIR=/usr/x86_64-w64-mingw32/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_LIBDIR
