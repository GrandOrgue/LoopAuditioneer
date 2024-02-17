#!/bin/bash

set -e

sudo apt update

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake g++ pkg-config \
  libasound2-dev libpulse-dev libwxgtk3.0-gtk3-dev gcc-mingw-w64-i686 \
  g++-mingw-w64-i686 autoconf autogen automake build-essential \
  libtool python
  
