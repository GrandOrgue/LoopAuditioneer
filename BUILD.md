# Build (compilation) instructions for LoopAuditioneer under Linux

You'll need standard development tools for c, c++, wxWidgets 3.0+, cmake, ALSA,
JACK and PULSEAUDIO. Also be sure to check the external libraries installation
files for more details on what might be needed. (apt-get build-dep for the
external programs can be used) There's a script in the scripts/ directory named
prepare-debian-ubuntu.sh that can install the needed tools.

The source code can be obtained (if git is available) with:

```
git clone --recurse-submodules https://github.com/GrandOrgue/LoopAuditioneer.git
```

When all build dependencies are satisfied the build process is simple. In the
dowloaded source code root directory issue the following commands:

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

If no errors occur, the binary will be created in the bin/ subdirectory of
build/ and can be run directly from that directory.

Optionally, the svn revision version can be specified with -DVERSION_REVISION=
to cmake and the build type can be set to Debug if that's desired.

If a TGZ package is desired, issue the following command in the build/
directory:

```
cpack
```

If it's so desired it's possible to install the program with (in build/
directory):

```
make install
```

## AppImage

For an AppImage target you have to have some additional tools installed, the
prepare-ubuntu-appimage.sh in scripts/ directory have the details. The main
difference to the normal build is that you have to drop Jack support which
results in this set of commands:

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DRTAUDIO_USE_JACK=OFF -DCMAKE_INSTALL_PREFIX=/usr
make
make install DESTDIR=AppDir
linuxdeploy-x86_64.AppImage --appdir AppDir -e ./bin/LoopAuditioneer -d ./share/LoopAuditioneer/applications/LoopAuditioneer.desktop -i ./share/icons/hicolor/256x256/apps/LoopAuditioneer.png -o appimage -p gtk
```

## Cross compilation to windows from Linux

Basic needs are the same as above, plus you need mingw-w64 (x86_64-w64-mingw32)
and the source for wxWidgets. There's a script in the scripts/ directory named
prepare-ubuntu-cross.sh that can install the needed tools.

First cross compile wxWidgets. In the extracted wx source directory that you've
downloaded (https://www.wxwidgets.org/):

```
mkdir win-ur-static
cd win-ur-static
../configure --host=x86_64-w64-mingw32 --prefix=/PATH_TO_WX_SOURCE/win-ur-static/inst --enable-unicode --disable-shared
make
make install
```

Then you modify the toolchain.def file in the root directory of LoopAuditioneer
source code so that the CMAKE_FIND_ROOT_PATH variable points to your cross
compiled wx installation.

Another possibility is to just get a pre-(cross)compiled static .deb package from
https://github.com/GrandOrgue/WxWidgetsCross and install that.

The next steps are similar to how building is done on Linux. In the root of
LoopAuditioneer sources:

```
mkdir crossbuild
cd crossbuild
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../toolchain.def
make
```

If a ZIP package is desired, issue the following command in the crossbuild/
directory:

```
cpack
```

## Native compilation on Windows with MSYS2

LoopAuditioneer can also be built natively on Windows with
[MSYS2](https://www.msys2.org/). The result is a statically linked executable
that only depends on Windows system DLLs, like the cross compiled Windows
release.

### Setup (once)

Install MSYS2 and open the "MSYS2 UCRT64" shell from the Start menu. All the
commands below are entered in that shell, where a Windows path such as
C:\LoopAuditioneer is written /c/LoopAuditioneer. First update MSYS2 (repeat
the command if the window closes during the update):

```
pacman -Syu
```

The source code must be in a path without spaces, since autotools cannot
handle them. Get the source code as described at the top of this file, then
issue the following command in its root directory:

```
bash scripts/prepare-msys2.sh
```

prepare-msys2.sh installs the needed MSYS2 packages and builds a static
wxWidgets into /opt, which takes a while. When run again, it only checks the
packages and keeps the existing wxWidgets.

### Building

In the root directory of the source code, issue:

```
bash scripts/build-msys2.sh
```

build-msys2.sh builds libsndfile, libsamplerate and RtAudio from the
submodules as static libraries, then LoopAuditioneer itself, in the
build-msys2/ directory. The binary is created in build-msys2/bin/, a ZIP
package in build-msys2/, and the output of the script is saved in
build-msys2/build-msys2.log.

The script can simply be run again after each change: it only rebuilds what
is needed. A library is rebuilt when the commit of its submodule has changed,
for example after `git submodule update`. The committed state of a submodule
is used; uncommitted changes inside a submodule are ignored. To rebuild the
three libraries anyway:

```
FORCE_DEPS=1 bash scripts/build-msys2.sh
```

Close LoopAuditioneer before building again, since Windows does not allow a
running executable to be replaced. A slightly quicker rebuild, which skips the
ZIP package, is:

```
cmake --build build-msys2 -j
```

### Options

The MINGW64 shell can be used instead of the UCRT64 shell. The scripts accept
the following optional environment variables, given before the command (for
example `JOBS=4 bash scripts/build-msys2.sh`):

* WX_PREFIX: wxWidgets directory, for both scripts (default:
  /opt/wx-VERSION-ucrt64-static, or -mingw64- in the MINGW64 shell)
* JOBS: number of parallel compile jobs, for both scripts (default: number of
  processors)
* SKIP_PACMAN=1: prepare-msys2.sh does not install packages
* BUILD_DIR: build directory of build-msys2.sh (default: build-msys2)
* FORCE_DEPS=1: build-msys2.sh rebuilds the three libraries

## Compiling LoopAuditioneer for macOS

Install [XCode](https://developer.apple.com/xcode) and the [XCode Command Line Tools](https://developer.apple.com/xcode/resources).

Install the required packages with [Homebrew](https://brew.sh):

```
brew install autoconf autoconf-archive autogen automake libtool pkg-config wxwidgets
```

Once all build dependencies are fulfilled, the build process is simple. Enter the following commands in the downloaded source code root directory:

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

If no errors occur, the binary will be created in the bin/ subdirectory of build/ and can be run directly from that directory.