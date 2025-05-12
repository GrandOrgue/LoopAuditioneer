#!/bin/bash

set -e

sudo apt update

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake g++ pkg-config \
  libasound2-dev libpulse-dev gcc-mingw-w64-x86-64 \
  g++-mingw-w64-x86-64 autoconf autogen automake build-essential \
  libtool python-is-python3 mingw-w64-tools
