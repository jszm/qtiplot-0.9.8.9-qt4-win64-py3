#!/usr/bin/env bash
# Build QtiPlot 0.9.8.9 natively for aarch64 Linux against the Qt 4.8.7
# prefix produced by build-qt.sh.
#
# Run inside the build container (OOM caps per project policy):
#   docker run --rm --memory=2g --memory-swap=2g --cpus=4 \
#     -v "$PWD":/home/c/projects/qtiplot-0.9.8.9-qt4-win64-py3 \
#     -v "$PWD/build/linux-arm64":/work \
#     qtiplot-deps:qt4 \
#     bash /home/c/projects/qtiplot-0.9.8.9-qt4-win64-py3/build/patches/qt4-aarch64/build-qtiplot.sh
#
# Image requirements (qtiplot-deps:qt4 = the qt4env image + zlib1g-dev
# libgsl-dev):
#   - the repo is bind-mounted at its absolute host path, so the working
#     copy below is durable on the host;
#   - build/linux-arm64 is ALSO bind-mounted at /work: the Qt prefix baked
#     into qmake is /work/Qt-4.8.7-aarch64 and build.conf points LUPDATE/
#     LRELEASE there.

set -o pipefail

REPO=/home/c/projects/qtiplot-0.9.8.9-qt4-win64-py3
TREE=$REPO/build/linux-arm64/qtiplot-tree
QTPREFIX=/work/Qt-4.8.7-aarch64

export PATH="$QTPREFIX/bin:$PATH"
export LD_LIBRARY_PATH="$QTPREFIX/lib"

# 1. Working copy of the source tree. qmake must run in-tree (the .pro
#    files use relative QTI_ROOT/3rdparty paths) but NOT in the repo
#    itself: qmake would overwrite the committed Windows Makefiles. build/
#    is excluded because the Qt install lives there and the copy sits
#    inside it.
if [ ! -d "$TREE/qtiplot/src" ]; then
  echo "== copying source tree =="
  mkdir -p "$TREE"
  tar -C "$REPO" \
    --exclude='./.git' --exclude='./build' --exclude='./artifacts' \
    --exclude='./lazarus-port' --exclude='./tmp' --exclude='./__pycache__' \
    --exclude='./qtiplot/release' --exclude='./qtiplot/debug' \
    --exclude='./*.exe' --exclude='./*.dll' --exclude='./*.a' \
    --exclude='./*.pyc' --exclude='./*.res' \
    -cf - . | tar -C "$TREE" -xf -
fi

# 1b. Windows qmake-generated Makefiles must not survive the copy: qmake
#     only regenerates subdir Makefiles at make time, and make would
#     otherwise happily use the stale ones (A:/ paths, backslash
#     separators). No Makefile in this repo is a source file.
find "$TREE" \( -name 'Makefile' -o -name 'Makefile.Debug' -o -name 'Makefile.Release' \
   -o -name 'Makefile.exportEMF*' -o -name 'Makefile.importOPJ*' \) -type f -delete

# 1c. Stale Windows build products must go too: tar preserves mtimes, so
#     make would treat the copied COFF objects (qwt/src/obj, qwtplot3d/tmp)
#     as up to date and archive them -> "archive has no index" at link.
find "$TREE" \( -name '*.o' -o -name 'moc_*.cpp' -o -name '*.a' \) -type f -delete

# 2. muParser: no prebuilt aarch64 lib is vendored (3rdparty has sources
#    only), so compile them. muParserDLL.cpp is the Windows DLL API,
#    unused here.
echo "== muParser static lib =="
cd "$TREE/3rdparty/muparser"
mkdir -p lib obj
for f in src/*.cpp; do
  base=$(basename "$f" .cpp)
  [ "$base" = muParserDLL.cpp ] && continue
  g++ -O2 -fPIC -Iinclude -c "$f" -o "obj/$base.o"
done
ar rcs lib/libmuparser.a obj/*.o

# 3. qmake + make. -k so one failing target doesn't hide the others; if
#    the container OOM-kills a compile, re-run at -j1.
echo "== qmake =="
cd "$TREE"
qmake qtiplot.pro
mkdir -p bin

echo "== make -j2 =="
make -j2 -k > "$TREE/build-qtiplot.log" 2>&1
status=$?
tail -n 30 "$TREE/build-qtiplot.log"
if [ $status -ne 0 ]; then
  echo "BUILD FAILED (status $status) - full log: $TREE/build-qtiplot.log"
else
  echo "BUILD OK - binary: $TREE/qtiplot"
  ls -la "$TREE/qtiplot"
fi

# 4. The container runs as root; hand the tree back to the host user.
chown -R 1000:1000 "$REPO/build/linux-arm64/qtiplot-tree"
exit $status
