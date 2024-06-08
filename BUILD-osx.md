# Build (compilation) instructions for LoopAuditioneer under macOS

## **Caution: This is currently a work in progress!**

The build process currently does not yet work. These instructions have been modified from [dhilowitz/How To Compile LoopAuditioneer on Mac](https://gist.github.com/dhilowitz/bf12ac1f6931068a92211850fdde1f81).

You'll need standard development tools for C++ and wxWidgets 3.0.

- Install XCode and the XCode command-line development tools.
- Install [Homebrew](https://brew.sh).
- Install various packages with Homebrew:

```
brew install git cmake wxwidgets libsndfile rtaudio libsamplerate pkgconfig autoconf
```

If these dependencies have already been installed when libsndfile was installed, the next command might not be necessary.

Install the dependencies required for [libsndfile](https://github.com/libsndfile/libsndfile):

```
brew install autoconf autogen automake flac libogg libtool libvorbis opus mpg123 pkg-config
```

Create a local copy of the **LoopAudtioneer** source code in a folder of your choice:

```
git clone --recurse-submodules https://github.com/vpoguru/LoopAuditioneer.git
```

Compile and prepare _libsndfile_:

```
cd submodules/libsndfile
autoreconf -vif
./configure --disable-external-libs
make
cd ..
ln -sf libsndfile/src/libsndfile.la .
```

Compile and prepare _rtaudio_:

```
cd rtaudio
autoreconf -vif
./configure --with-core
make
cd ..
ln -sf rtaudio/.libs/librtaudio.a .
```

Compile and prepare _libsamplerate_:

```
cd libsamplerate
autoreconf -vif
./configure
make
cd ..
ln -sf libsamplerate/src/libsamplerate.la .
```

## The next steps are not working yet!

Amy help is very welcome!

Go back out to the source directory and compile the program:

```
cd ../src
g++ -D__MACOSX_CORE__ -o LoopAuditioneer LoopAuditioneer.cpp MyFrame.cpp MyPanel.cpp CueMarkers.cpp LoopMarkers.cpp FileHandling.cpp MySound.cpp WaveformDrawer.cpp LoopParametersDialog.cpp BatchProcessDialog.cpp AutoLoopDialog.cpp AutoLooping.cpp PitchDialog.cpp CrossfadeDialog.cpp LoopOverlay.cpp FFT.cpp StopHarmonicDialog.cpp CutNFadeDialog.cpp MyListCtrl.cpp MyResampler.cpp ListInfoDialog.cpp -I../submodules/libsndfile/src -I../submodules/rtaudio -I../submodules/libsamplerate/src -I../build/src/FreePixelIcons -I../resources/icons ../submodules/libsndfile.la ../submodules/librtaudio.a ../submodules/libsamplerate.la -framework CoreAudio -framework AudioToolbox -lpthread -lm `wx-config --cxxflags --unicode=yes --libs`
```

Once this completes, you can run this on the command line like this:

```
WXSUPPRESS_SIZER_FLAGS_CHECK=1
./LoopAuditioneer
```

To reduce executable size:

```
strip --strip-all LoopAuditioneer
```