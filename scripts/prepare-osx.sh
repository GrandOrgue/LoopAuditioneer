#!/bin/zsh

brew install git cmake wxwidgets libsndfile rtaudio libsamplerate pkgconfig autoconf

# I do not yet know whether we need these packages. They are required by libsndfile,
# but I assume that they are automatically installed, when you install libsndfile
# brew install autoconf autogen automake flac libogg libtool libvorbis opus mpg123 pkg-config