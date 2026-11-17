#!/bin/bash
#
# Prepares an MSYS2 environment for building LoopAuditioneer natively on
# Windows: installs the needed packages and builds a static wxWidgets (once).
# Run it from the MSYS2 UCRT64 (or MINGW64) shell, then run build-msys2.sh.
# See BUILD.md.
#
# Optional environment variables:
#   WX_PREFIX      wxWidgets install prefix (default: /opt/wx-<version>-<msystem>-static)
#   JOBS           number of parallel jobs (default: number of CPUs)
#   SKIP_PACMAN=1  do not install packages with pacman
#   LA_CROSS_HOST  only for testing these scripts from Linux with mingw-w64,
#                  e.g. LA_CROSS_HOST=x86_64-w64-mingw32

set -euo pipefail

WX_VERSION=3.2.11
WX_URL="https://github.com/wxWidgets/wxWidgets/releases/download/v${WX_VERSION}/wxWidgets-${WX_VERSION}.tar.bz2"
WX_SHA256=6a129015bce2e914e4bf61ec4411854ad962801d47e92f2eb8340adb6a90af08

die()  { printf '\n*** ERROR: %s\n' "$*" >&2; exit 1; }
step() { printf '\n======== %s ========\n' "$*"; }

# Path as seen by native Windows programs (C:/...), unchanged outside MSYS2.
mixpath() {
  if command -v cygpath >/dev/null 2>&1; then cygpath -m "$1"; else printf '%s\n' "$1"; fi
}

CROSS_HOST=${LA_CROSS_HOST:-}
if [ -z "$CROSS_HOST" ]; then
  case "${MSYSTEM:-}" in
    UCRT64|MINGW64) ;;
    *) die "run this from an MSYS2 UCRT64 or MINGW64 shell (MSYSTEM is '${MSYSTEM:-unset}')" ;;
  esac
  MSYS_TAG=$(printf '%s' "$MSYSTEM" | tr '[:upper:]' '[:lower:]')
  HOST_ARGS=()
  CC=gcc
else
  MSYS_TAG=cross
  HOST_ARGS=(--host="$CROSS_HOST")
  CC=$CROSS_HOST-gcc
  SKIP_PACMAN=1
fi

WX_PREFIX=${WX_PREFIX:-/opt/wx-$WX_VERSION-$MSYS_TAG-static}
JOBS=${JOBS:-$(nproc 2>/dev/null || echo 4)}

# Autotools and libtool cannot handle spaces in paths.
for p in "$WX_PREFIX" "$(mixpath "$WX_PREFIX")" "$(mixpath /tmp)"; do
  case "$p" in
    *[[:space:]]*) die "path contains a space, which autotools cannot handle: '$p'" ;;
  esac
done

# ---------------------------------------------------------------------------
# MSYS2 packages
# ---------------------------------------------------------------------------
if [ "${SKIP_PACMAN:-0}" != 1 ]; then
  step "Installing MSYS2 packages"
  pacman -S --needed --noconfirm \
    git make zip tar bzip2 curl python \
    "${MINGW_PACKAGE_PREFIX}-toolchain" \
    "${MINGW_PACKAGE_PREFIX}-cmake" \
    "${MINGW_PACKAGE_PREFIX}-autotools"
fi

for t in git make zip tar curl sha256sum autoreconf cmake cpack "$CC"; do
  command -v "$t" >/dev/null 2>&1 || die "required tool '$t' not found"
done

# The Windows link line of LoopAuditioneer (src/CMakeLists.txt) contains -lssp.
WORK=$(mktemp -d /tmp/la-prepare.XXXXXX)
printf 'int main(void){return 0;}\n' > "$WORK/ssp.c"
"$CC" "$WORK/ssp.c" -Wl,--as-needed -lssp -o "$WORK/ssp.exe" \
  || die "'$CC' cannot link with -lssp, which the Windows link line of LoopAuditioneer requires"

# ---------------------------------------------------------------------------
# Static wxWidgets
# ---------------------------------------------------------------------------
if [ -f "$WX_PREFIX/.la-build-complete" ]; then
  step "wxWidgets $WX_VERSION is already installed in $WX_PREFIX"
  rm -rf "$WORK"
  exit 0
fi

step "Building static wxWidgets $WX_VERSION into $WX_PREFIX (takes a while)"
echo "Work directory: $WORK (kept if the build fails)"
curl -fL --retry 3 -o "$WORK/wx.tar.bz2" "$WX_URL"
echo "$WX_SHA256  $WORK/wx.tar.bz2" | sha256sum -c - || die "checksum mismatch for $WX_URL"
tar -xjf "$WORK/wx.tar.bz2" -C "$WORK"
mkdir -p "$WORK/wxWidgets-$WX_VERSION/build-static"
cd "$WORK/wxWidgets-$WX_VERSION/build-static"
# --disable-sys-libs: use the zlib/png/jpeg/tiff/expat/regex bundled with wx,
# so that a -static link does not pull in MSYS2 libraries and their own
# dependencies. The disabled sub-libraries are not used by LoopAuditioneer.
# -std=gnu17 for C code: recent GCC defaults to C23, which older C code may reject.
../configure "${HOST_ARGS[@]}" \
  --prefix="$(mixpath "$WX_PREFIX")" \
  --with-msw --disable-shared --disable-sys-libs \
  --without-liblzma --without-libcurl --without-opengl \
  --disable-webview --disable-stc --disable-richtext --disable-propgrid \
  --disable-ribbon --disable-aui --disable-xrc --disable-mediactrl \
  CFLAGS="-O2 -std=gnu17" CXXFLAGS="-O2"
make -j"$JOBS"
make install
[ -f "$WX_PREFIX/bin/wx-config" ] || die "$WX_PREFIX/bin/wx-config was not installed"
touch "$WX_PREFIX/.la-build-complete"

cd /
rm -rf "$WORK"
step "wxWidgets $WX_VERSION installed in $WX_PREFIX"
