#!/bin/bash
# Qt 4.8.7 aarch64 Linux build. Runs inside an ubuntu:24.04 container with
# /work bind-mounted to the repo's build/linux-arm64/. Memory/CPU caps are
# enforced by docker (--memory=2g --cpus=4), not here; compile parallelism is
# deliberately low (-j2) to stay inside the 2 GB cap (host has ~5 GB free).
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive
echo "== $(date -u +%FT%TZ) container build start =="
free -m || true

apt-get update -qq
apt-get install -y -qq --no-install-recommends \
  build-essential perl patch pkg-config \
  libx11-dev libxext-dev libxrender-dev libxfixes-dev libxcursor-dev \
  libxinerama-dev libxi-dev libxrandr-dev libfontconfig1-dev \
  libfreetype6-dev libgl1-mesa-dev libglu1-mesa-dev

WORK=/work
SRC=$WORK/src
BUILD=$WORK/qt-src
PREFIX=$WORK/Qt-4.8.7-aarch64

rm -rf "$BUILD" "$PREFIX"
mkdir -p "$BUILD"

tarball="$SRC/qt4-x11_4.8.7+dfsg.orig.tar.xz"
echo "extracting $(basename "$tarball")"
tar -xJf "$tarball" -C "$BUILD"

cd "$BUILD"/*/

# Apply Ubuntu's patch series (the exact set used for its aarch64 packages).
SERIES="$SRC/debian/debian/patches/series"
while IFS= read -r p; do
  [ -z "$p" ] && continue
  case "$p" in \#*) continue;; esac
  if ! patch -p1 --silent < "$SRC/debian/debian/patches/$p"; then
    echo "PATCH_FAILED: $p"
    exit 1
  fi
  echo "patch ok: $p"
done < "$SERIES"

ls mkspecs/ | grep -i aarch64 || echo "no aarch64 mkspec; relying on linux-g++"

# The repo's required uic fix (see build/patches/): uic's DomWidget::read
# iterates the live QXmlStreamAttributes vector, which reallocates during
# parsing and silently drops widget members (Ui::QPageSetupWidget.topMargin
# etc.). Without it src/gui fails to compile. The patch file was regenerated
# in proper unified-diff format for this build (same intent as the original).
patch -p1 --silent < "$SRC/qt-4.8.7-uic-widget-attributes.patch"
echo "patch ok: qt-4.8.7-uic-widget-attributes (uic DomWidget::read)"

# Qt 4.8.7 predates C++11; modern GCC defaults to gnu++17 where std::tr1 and
# std::auto_ptr are gone (the WTF headers pulled in by bootstrap/pcre fail to
# compile). Build the whole tree as the C++98 it was written for.
sed -i 's|^load(qt_config)$|QMAKE_CXXFLAGS += -std=gnu++98\nload(qt_config)|' mkspecs/linux-g++/qmake.conf
echo "mkspec: forced -std=gnu++98"

echo "== configure =="
./configure -opensource -confirm-license -release \
  -prefix "$PREFIX" \
  -platform linux-g++ \
  -nomake examples -nomake demos -no-webkit -no-phonon -no-declarative \
  -no-scripttools -no-openssl -no-openvg -no-dbus -no-glib -no-gtkstyle \
  -no-cups -no-nis -no-icu -qt-sql-sqlite -qt-libpng -qt-libjpeg -qt-zlib \
  -opengl desktop -R "$PREFIX/lib"

# Configure's syncqt forwards every upstream arch header into include/QtCore
# but not the Debian patch's new aarch64 one; add the forwarder by hand.
printf '#include "../../src/corelib/arch/qatomic_aarch64.h"\n' \
  > include/QtCore/qatomic_aarch64.h

echo "== make -j2 =="
# -k: the src/plugins/accessible/widgets plugin fails by design with
# -no-declarative (it includes qtdeclarative private headers, a known upstream
# wart); it is not needed by qtiplot. Everything else must succeed.
make -j2 -k

echo "== make install (skip known-broken plugin) =="
make -k install

# Qt's header install uses a fixed file list, so the extra aarch64 atomic
# header never reaches the prefix; install the real (self-contained) header.
cp "$BUILD/src/corelib/arch/qatomic_aarch64.h" "$PREFIX/include/QtCore/"

# lrelease/lupdate are not part of the default ordered build after a src
# failure; build and install them explicitly for qtiplot's translations.
for t in lrelease lupdate; do
  ( cd "tools/linguist/$t" && make clean >/dev/null 2>&1
    "$PREFIX/bin/qmake" "$t.pro" >/dev/null 2>&1
    make -j2 >/dev/null 2>&1
    cp -f "../../../bin/$t" "$PREFIX/bin/" )
  echo "installed $t"
done

chown -R 1000:1000 "$WORK"
echo "== $(date -u +%FT%TZ) done =="
