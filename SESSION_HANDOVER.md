# Session Handover - QtiPlot 0.9.8.9 Qt4 Win64 Py3

Last updated: 2026-10-05, end of session (host local time)

Workspace (moved from the Windows A:\ drive to an NVIDIA DGX Spark,
aarch64 Ubuntu 24.04, X11):

```text
/home/c/projects/qtiplot-0.9.8.9-qt4-win64-py3
```

## Resume Here

Two tracks are underway, both user-approved on 2026-10-05:

- Track A (native reference): **Milestone 2 is DONE.** QtiPlot 0.9.8.9 now
  builds and runs natively for aarch64 Linux. Next step is the GUI
  diagnostic matrix below, run against this native build. Start a fresh
  milestone-3 list from it; do not change GUI code before each issue has a
  reproducible sequence and expected-vs-actual behavior.
- Track B (Lazarus port): spike 0 verified; next milestone per
  `lazarus-port/PORT-PLAN.md` is M1 (table + CSV + column formulas).

Run the native build (host desktop, X11 session :1):

```text
QT=$PWD/build/linux-arm64/Qt-4.8.7-aarch64
LD_LIBRARY_PATH=$QT/lib:$PWD/build/linux-arm64/runtime-libs \
QT_PLUGIN_PATH=$QT/plugins DISPLAY=:1 \
  build/linux-arm64/qtiplot-tree/bin/qtiplot
```

`build/linux-arm64/runtime-libs/` holds libgsl.so.27/libgslcblas.so.0
extracted from the container image (no sudo on the host); re-extract with
`docker run --rm -v $PWD/build/linux-arm64/runtime-libs:/out qtiplot-deps:qt4 cp /usr/lib/aarch64-linux-gnu/libgsl.so.27 /usr/lib/aarch64-linux-gnu/libgslcblas.so.0 /out/`.

Verified 2026-10-05: process alive 20 s, window "QtiPlot - untitled"
(2560x1440) present on the X server, empty stderr.

Session close state (2026-10-05): this session ended immediately after the
milestone-2 verification above. Nothing is half-done — no QtiPlot process
is running, the working tree is clean, and the `qt4env` build container is
left up on purpose (2 GB cap; `docker rm qt4env` if you need the memory).
Read this file top to bottom, then pick the next milestone.

## Repository and Remote State

```text
Branch: main  HEAD: session-close handover commit on top of b163d11b
        (both pushed to origin/main 2026-10-05; confirm with
        git log --oneline -3)
Remote: https://github.com/jszm/qtiplot-0.9.8.9-qt4-win64-py3.git
```

History tip: d78940d3 -> 8f8c6824 (libqti restore) -> 0ce3a6a1 (ISO 8601
datetime) -> c6b62af7 (docs) -> f0ca1c81 (Lazarus spike) -> 400aab16 (Qt
aarch64 build) -> b163d11b (native qtiplot build) -> session-close
handover.

Working tree is clean. Do not commit or push unless the user explicitly
asks for it and the exact path list has been reviewed.

## Native aarch64 Build (Track A)

- Qt 4.8.7 installed at `build/linux-arm64/Qt-4.8.7-aarch64/`; reproduce
  with `build/patches/qt4-aarch64/build-qt.sh` inside a 2GB-capped
  container (Ubuntu 24.04 + Ubuntu qt4-x11 4.8.7 orig tarball + Debian
  patch series; gotchas documented in the script comments).
- QtiPlot: reproduce with `build/patches/qt4-aarch64/build-qtiplot.sh`
  (same caps). It copies the repo to `build/linux-arm64/qtiplot-tree/`
  (qmake must not run in the repo itself), purges stale Windows
  Makefiles/objects, builds vendored muParser, then qmake+make -j2.
  Container image: `qtiplot-deps:qt4` (qt4env + zlib1g-dev + libgsl-dev).
- Source changes for the native build are in commit b163d11b: unix
  sections in `build.conf`, EMF export made Windows-only, Qt4 QBool
  comparison fixes (GCC 13), GSL 2.x `gsl_multifit_fdfsolver_jac`, and
  `unix:DESTDIR = ../bin`.
- Memory discipline: docker `--memory=2g --memory-swap=2g --cpus=4`,
  `make -j2` (host only has ~5-6 GB available). If a compile is
  OOM-killed inside the container, retry at -j1.
- New tools/libraries go in `~/tools` (Lazarus 4.8 + FPC 3.2.2 live
  there; wine-hangover 11.9 is there for future Windows-exe checks).
- Python/SIP scripting is deferred on unix; muParser scripting is built.

## Windows Reference (historical)

The Windows toolchain and portable package live on the old A:\ drive and
are documented in `BUILD-WINDOWS-QT4-PY3.md`. Canonical portable build:
`artifacts\portable\qtiplot-0.9.8.9-qt4-win64-py3-portable.7z`
(SHA256 9D1C69007B039D1F4D287B489C0A992A9D42069570E0000E7F3CD45AFA966DE6).
Required source patches remain:
`build/patches/qt-4.8.7-uic-widget-attributes.patch` and
`build/patches/qtiplot-qflags-qt4.patch` (the latter is Windows-specific
and not needed on unix).

## GUI Diagnostic Matrix (platform-agnostic; run on the native build)

Record dataset size, exact actions, elapsed delay, expected behavior,
actual behavior, and a screenshot for every confirmed issue.

1. Baseline interaction
   - startup and first table creation
   - cell editing, column selection, scrolling, and table-window resizing
   - main-window and MDI child minimize/maximize/restore behavior
2. Basic plotting
   - generate deterministic X/Y data with column formulas
   - create line, symbol, and line-plus-symbol plots
   - check autoscale, axis labels, legends, curve visibility, and redraw
3. Plot interaction
   - zoom in/out, pan, rescale, data reader/picker, and layer resize
   - resize the plot MDI window repeatedly from every edge and corner
   - switch rapidly between the table and graph
4. Responsiveness by data size
   - small: 1,000 points; medium: 10,000; heavy: 100,000
   - note UI freezes, delayed repaints, high CPU, input lag, and
     cancellation behavior during plot creation and redraw
5. Output
   - copy/export a representative graph to PNG, SVG, and PDF if available
   - verify that export does not alter the graph or freeze the application

Start with 2D plotting. Exercise 3D plotting only after the 2D path has a
clear baseline. The 2026-06-27 MDI table-border resize fix
(d78940d3) belongs in this regression pass.

## Lazarus Port (Track B)

- `lazarus-port/PORT-PLAN.md` has the milestone list (M0 spike done,
  M1 next: table + CSV + formulas) and the rule: no port feature without
  a reference behavior from Track A.
- Build: `. ~/tools/lazarus-4.8/env.sh && cd lazarus-port &&
  lazbuild --build-all qtiplot-laz.lpi` (binary at lazarus-port/bin/).

## Safe Next Actions

1. Read this file, `.zcode/local-agent-checkpoint.md`, and
   `lazarus-port/PORT-PLAN.md`.
2. Recheck `git status --short --branch`.
3. Track A: run the GUI diagnostic matrix against the native build.
4. Track B: implement Lazarus M1.
5. Review the exact commit contents before any commit or push.
