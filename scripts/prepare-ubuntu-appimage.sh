#!/bin/bash

set -e

sudo apt update

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake g++ pkg-config \
  libasound2-dev libpulse-dev libwxgtk3.0-gtk3-dev libudev-dev patchelf \
  libgtk-3-dev librsvg2-dev autoconf autogen automake build-essential \
  libtool python-is-python3 libfuse2
  
# linuxdeploy, linuxdeploy-plugin-gtk and appimagetool must be downloaded
sudo wget -P /usr/local/bin https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh
sudo wget -P /usr/local/bin https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
sudo wget -P /usr/local/bin https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage

# make them executable
sudo chmod +x /usr/local/bin/linuxdeploy-plugin-gtk.sh
sudo chmod +x /usr/local/bin/linuxdeploy-x86_64.AppImage
sudo chmod +x /usr/local/bin/appimagetool-x86_64.AppImage

