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

Go to the folder _submodules/libsndfile_ and compile libsndfile.a:

```
autoreconf -vif
./configure --enable-static --disable-shared --disable-external-libs --disable-full-suite
make
```

Go to the folder _submodules/rtaudio_ and compile librtaudio.a:

```
autoreconf -vif
./configure --enable-static --disable-shared --with-core
make
```

Go to the folder _submodules/libsamplerate_ and compile libsamplerate.a:

```
autoreconf -vif
./configure --enable-static --disable-sndfile --disable-fftw --disable-shared
make
```

Go back out to the source directory and compile the program:

```
cd ../src
g++ -D__MACOSX_CORE__ -o LoopAuditioneer LoopAuditioneer.cpp MyFrame.cpp MyPanel.cpp CueMarkers.cpp LoopMarkers.cpp FileHandling.cpp MySound.cpp WaveformDrawer.cpp LoopParametersDialog.cpp BatchProcessDialog.cpp AutoLoopDialog.cpp AutoLooping.cpp PitchDialog.cpp CrossfadeDialog.cpp LoopOverlay.cpp FFT.cpp StopHarmonicDialog.cpp CutNFadeDialog.cpp MyListCtrl.cpp MyResampler.cpp ListInfoDialog.cpp SpectrumDialog.cpp SpectrumPanel.cpp AudioSettingsDialog.cpp -I../submodules/libsndfile/include -I../submodules/rtaudio -I../submodules/libsamplerate/include -I../../FreePixelIcons -I../resources/icons ../submodules/libsndfile/src/.libs/libsndfile.a ../submodules/rtaudio/.libs/librtaudio.a ../submodules/libsamplerate/src/.libs/libsamplerate.a -framework CoreAudio -framework AudioToolbox -lpthread -lm `wx-config --cxxflags --unicode=yes --libs` --std=c++11
```

Once this is complete, you can start LoopAuditioneer from the command line as follows:

```
WXSUPPRESS_SIZER_FLAGS_CHECK=1
./LoopAuditioneer
```
