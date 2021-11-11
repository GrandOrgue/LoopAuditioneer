#!/bin/bash

set -e

sudo apt update

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake g++ pkg-config \
  libjack-dev libasound2-dev libpulse-dev libwxgtk3.0-gtk3-dev
  

