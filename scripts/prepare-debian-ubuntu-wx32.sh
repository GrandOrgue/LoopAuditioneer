#!/bin/bash

set -e

sudo add-apt-repository ppa:sjr/wx32
sudo apt update

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake g++ pkg-config \
  libjack-dev libasound2-dev libpulse-dev libwxgtk3.2-dev \
  autoconf autogen automake build-essential \
  libtool python-is-python3 autoconf-archive

