#!/bin/bash
#
# Builds LoopAuditioneer natively on Windows with MSYS2. Run it from the MSYS2
# UCRT64 (or MINGW64) shell, after prepare-msys2.sh. See BUILD.md.
#
#   scripts/build-msys2.sh [path-to-LoopAuditioneer-sources]
#
# 1. builds libsndfile, libsamplerate and RtAudio from the submodules as static
#    libraries into the build directory, when they are missing or when their
#    submodule commit changed. CMakeLists.txt does not build them itself when
#    it finds them there; its execute_process() calls cannot run the autotools
#    scripts from the native Windows cmake.
# 2. configures with -DCMAKE_SYSTEM_NAME=Windows. Setting the system name makes
#    CMAKE_CROSSCOMPILING true, which selects the Windows branches of the CMake
#    files, so the result matches the cross compiled Windows release.
# 3. builds LoopAuditioneer and creates the ZIP package.
#
# Optional environment variables:
#   BUILD_DIR      build directory (default: <sources>/build-msys2)
#   WX_PREFIX      wxWidgets prefix (default: newest /opt/wx-*-<msystem>-static)
#   JOBS           number of parallel jobs (default: number of CPUs)
#   FORCE_DEPS=1   rebuild libsndfile, libsamplerate and RtAudio even if they
#                  are up to date
#   LA_CROSS_HOST  only for testing these scripts from Linux with mingw-w64

set -euo pipefail

die()  { printf '\n*** ERROR: %s\n' "$*" >&2; exit 1; }
step() { printf '\n======== %s ========\n' "$*"; }

# Path as seen by native Windows programs (C:/...), unchanged outside MSYS2.
mixpath() {
  if command -v cygpath >/dev/null 2>&1; then cygpath -m "$1"; else printf '%s\n' "$1"; fi
}

SRC=$(cd "${1:-$(dirname "$0")/..}" && pwd)
if [ ! -f "$SRC/CMakeLists.txt" ] || [ ! -f "$SRC/src/LoopAuditioneer.cpp" ]; then
  die "$SRC does not look like the LoopAuditioneer sources"
fi

CROSS_HOST=${LA_CROSS_HOST:-}
if [ -z "$CROSS_HOST" ]; then
  case "${MSYSTEM:-}" in
    UCRT64|MINGW64) ;;
    *) die "run this from an MSYS2 UCRT64 or MINGW64 shell (MSYSTEM is '${MSYSTEM:-unset}')" ;;
  esac
  MSYS_TAG=$(printf '%s' "$MSYSTEM" | tr '[:upper:]' '[:lower:]')
  HOST_ARGS=()
  OBJDUMP=objdump
  CMAKE_ARGS=(-G "MSYS Makefiles" -DCMAKE_SYSTEM_NAME=Windows)
else
  MSYS_TAG=cross
  HOST_ARGS=(--host="$CROSS_HOST")
  OBJDUMP=$CROSS_HOST-objdump
  CMAKE_ARGS=(-G "Unix Makefiles" -DCMAKE_SYSTEM_NAME=Windows
              -DCMAKE_C_COMPILER="$CROSS_HOST-gcc" -DCMAKE_CXX_COMPILER="$CROSS_HOST-g++"
              -DCMAKE_RC_COMPILER="$CROSS_HOST-windres" -DCMAKE_FIND_ROOT_PATH="/usr/$CROSS_HOST")
fi

BUILD_DIR=${BUILD_DIR:-$SRC/build-msys2}
JOBS=${JOBS:-$(nproc 2>/dev/null || echo 4)}
DEPS=$BUILD_DIR/_deps

if [ -z "${WX_PREFIX:-}" ]; then
  WX_PREFIX=$(printf '%s\n' /opt/wx-*-"$MSYS_TAG"-static | sort -V | tail -n 1)
fi
if [ ! -f "$WX_PREFIX/.la-build-complete" ]; then
  die "no static wxWidgets found, run scripts/prepare-msys2.sh first (or set WX_PREFIX)"
fi
WX_CONFIG=$WX_PREFIX/bin/wx-config

for t in git make zip tar autoreconf cmake cpack "$OBJDUMP"; do
  command -v "$t" >/dev/null 2>&1 || die "required tool '$t' not found, run scripts/prepare-msys2.sh first"
done

# Autotools and libtool cannot handle spaces in paths.
for p in "$SRC" "$BUILD_DIR" "$(mixpath "$SRC")" "$(mixpath "$BUILD_DIR")"; do
  case "$p" in
    *[[:space:]]*) die "path contains a space, which autotools cannot handle: '$p'" ;;
  esac
done

mkdir -p "$DEPS"
exec > >(tee "$BUILD_DIR/build-msys2.log") 2>&1
echo "Sources   : $SRC"
echo "Build dir : $BUILD_DIR"
echo "wxWidgets : $WX_PREFIX"
echo "Log       : $BUILD_DIR/build-msys2.log"

# ---------------------------------------------------------------------------
# 1. libsndfile, libsamplerate and RtAudio
# ---------------------------------------------------------------------------
# A library is rebuilt when its .a file is missing, or when the commit of its
# submodule or its build arguments differ from those of its last build, which
# are recorded in _deps/<name>.stamp. FORCE_DEPS=1 rebuilds all of them.
# The committed state of the submodule (HEAD) is exported with git archive and
# LF line endings, so autotools also work in a checkout made with
# core.autocrlf=true and the submodule working tree is never modified.
build_dep() {   # $1 = submodule name, $2 = library file, $3... = configure arguments
  local name=$1 lib=$2
  shift 2
  local sm=$SRC/submodules/$name
  local make_args=()
  case $name in
    # BUILT_SOURCES only lists test sources generated with the 'autogen' tool,
    # they are not needed for the library.
    libsndfile) make_args=(BUILT_SOURCES=) ;;
    # Library only: the test programs do not compile with recent GCC
    # (missing #include <cstdint>).
    rtaudio) make_args=(SUBDIRS=.) ;;
  esac

  if [ ! -e "$sm/.git" ]; then
    git -C "$SRC" -c core.autocrlf=false submodule update --init "submodules/$name"
  fi
  if [ "$(cd "$(git -C "$sm" rev-parse --show-toplevel)" && pwd)" != "$(cd "$sm" && pwd)" ]; then
    die "submodules/$name is not an initialized git submodule"
  fi
  case $(git -C "$SRC" submodule status -- "submodules/$name") in
    +*) echo "WARNING: submodules/$name is not checked out at the commit recorded by the" \
             "repository, its checked out commit is used ('git submodule update' fixes this)." ;;
  esac

  local commit stamp stamp_file=$DEPS/$name.stamp
  commit=$(git -C "$sm" rev-parse HEAD)
  stamp="$commit ${HOST_ARGS[*]} $* ${make_args[*]}"
  if [ "${FORCE_DEPS:-0}" != 1 ] && [ -f "$BUILD_DIR/lib/$lib" ] \
     && [ "$(cat "$stamp_file" 2>/dev/null)" = "$stamp" ]; then
    step "$name is up to date (${commit:0:12})"
    return
  fi

  step "Building $name (${commit:0:12})"
  rm -f "$BUILD_DIR/lib/$lib" "$stamp_file"
  rm -rf "${DEPS:?}/${name:?}" && mkdir -p "$DEPS/$name"
  git -C "$sm" -c core.autocrlf=false -c core.eol=lf archive --format=tar HEAD \
    | tar -x -C "$DEPS/$name"
  (
    cd "$DEPS/$name"
    if [ "$name" = rtaudio ]; then sh ./autogen.sh --no-configure; else autoreconf -fi; fi
    ./configure "${HOST_ARGS[@]}" --prefix="$BUILD_DIR" --enable-static --disable-shared "$@"
    make -j"$JOBS" "${make_args[@]}"
    make install "${make_args[@]}"
  )
  [ -f "$BUILD_DIR/lib/$lib" ] || die "$BUILD_DIR/lib/$lib was not built"
  printf '%s\n' "$stamp" > "$stamp_file"
}

build_dep libsndfile libsndfile.a \
  --disable-external-libs --disable-mpeg --disable-full-suite --disable-sqlite --disable-alsa \
  CFLAGS="-O2 -std=gnu17" LIBS="-Wl,--as-needed -lssp"
build_dep libsamplerate libsamplerate.a \
  --disable-sndfile --disable-fftw --disable-alsa CFLAGS="-O2 -std=gnu17"
build_dep rtaudio librtaudio.a \
  --with-dsound --with-wasapi
[ -f "$BUILD_DIR/include/rtaudio/RtAudio.h" ] || die "RtAudio.h was not installed"
mkdir -p "$BUILD_DIR/tmp"

# ---------------------------------------------------------------------------
# 2. and 3. LoopAuditioneer
# ---------------------------------------------------------------------------
# Configure only when needed: every configure rewrites the embedded icons
# header (scripts/CreateResources.cmake), which forces a recompile and relink.
# cmake --build reconfigures by itself when a CMake file has changed.
CMAKE_CFG=("${CMAKE_ARGS[@]}" -DCMAKE_BUILD_TYPE=Release
           -DwxWidgets_CONFIG_EXECUTABLE="$(mixpath "$WX_CONFIG")")
CMAKE_STAMP=$DEPS/cmake.stamp
if [ -f "$BUILD_DIR/CMakeCache.txt" ] && [ "$(cat "$CMAKE_STAMP" 2>/dev/null)" = "${CMAKE_CFG[*]}" ]; then
  step "LoopAuditioneer is already configured"
else
  step "Configuring LoopAuditioneer"
  rm -f "$CMAKE_STAMP"
  cmake -S "$SRC" -B "$BUILD_DIR" "${CMAKE_CFG[@]}"
  printf '%s\n' "${CMAKE_CFG[*]}" > "$CMAKE_STAMP"
fi

step "Building LoopAuditioneer"
cmake --build "$BUILD_DIR" -j "$JOBS"

step "Packaging"
(cd "$BUILD_DIR" && cpack -G ZIP)

EXE=$BUILD_DIR/bin/LoopAuditioneer.exe
[ -f "$EXE" ] || die "$EXE was not built"

step "DLLs imported by LoopAuditioneer.exe"
"$OBJDUMP" -p "$EXE" | sed -n 's/^[[:space:]]*DLL Name: //p' | sort -u | tee "$DEPS/dlls.txt"
if grep -qiE '^(lib|wx)' "$DEPS/dlls.txt"; then
  die "LoopAuditioneer.exe depends on non-system DLLs (listed above)"
fi
echo "Only Windows system DLLs: the executable is self-contained."

step "Done"
echo "Executable : $(mixpath "$EXE")"
for z in "$BUILD_DIR"/LoopAuditioneer-*.zip; do
  [ -f "$z" ] && echo "Package    : $(mixpath "$z")"
done
echo "Incremental rebuild after code changes: cmake --build $(mixpath "$BUILD_DIR") -j"
