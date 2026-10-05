# Session Handover - QtiPlot 0.9.8.9 Qt4 Win64 Py3

Last updated: 2026-08-02 (Asia/Tokyo)

Workspace:

```text
A:\projects\qtiplot-0.9.8.9-qt4-win64-py3
```

## Resume Here

The build and portable-package reconstruction are complete. The next task is a
diagnostic GUI pass focused on responsiveness and plotting, followed by a
prioritized improvement plan. Do not start changing GUI code until each issue
has a reproducible sequence and expected-versus-actual behavior.

Run the canonical portable build from:

```text
A:\projects\qtiplot-0.9.8.9-qt4-win64-py3\artifacts\portable\qtiplot-0.9.8.9-qt4-win64-py3-portable\qtiplot.exe
```

No QtiPlot process was running when this handover was written.

## Exact Stop Point of the GUI Pass

The GUI diagnostic was started and then cancelled at the user's request before
substantive plotting tests:

1. The portable QtiPlot executable opened successfully with the default
   two-column table.
2. Column 1 was selected.
3. `Table > Set Column Values...` was opened.
4. Formula `i` was typed, but **Apply was not clicked**.
5. The test was cancelled. No QtiPlot project or table was saved, and the
   application is no longer running.

No new GUI glitch should be considered confirmed from that partial pass.

## Recommended GUI Diagnostic Matrix

Record the dataset size, exact actions, elapsed delay, expected behavior,
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
   - small: 1,000 points
   - medium: 10,000 points
   - heavy: 100,000 points
   - note UI freezes, delayed repaints, high CPU, input lag, and cancellation
     behavior during plot creation and redraw
5. Output
   - copy/export a representative graph to PNG, SVG, and PDF if available
   - verify that export does not alter the graph or freeze the application

Start with 2D plotting. Exercise 3D plotting only after the 2D path has a clear
baseline.

## Repository and Remote State

The checkout is detached at the release-fix tag:

```text
Tag:    v0.9.8.9-qt4-win64-py3-r2
Commit: d78940d34273501be9280640057bb903fe5052a5
Remote: https://github.com/jszm/qtiplot-0.9.8.9-qt4-win64-py3.git
```

On 2026-08-02, a live `git ls-remote` check showed that `origin/main` and the
remote default `HEAD` both pointed to the same commit. There are no committed
local changes ahead of the online repository. No files are staged.

The working tree intentionally contains pending reconstruction work:

- modified: `.gitignore`, `BUILD-WINDOWS-QT4-PY3.md`, `build.conf`, and
  `README.md`
- untracked: `SESSION_HANDOVER.md`
- untracked: the restored Boost preprocessor header
- untracked: two build patches and the embedded-Python smoke probe under
  `build\`
- untracked: 42 restored files under `qtiplot\src\lib\`

The checkout has no current branch. Create or switch to an intentional branch
before any future commit. Do not commit or push unless the user explicitly asks
for it and the exact path list has been reviewed again.

## Build and Artifact Status

The reproduced executable is 64-bit and uses Qt 4.8.7 plus Python 3.7.9.
Detailed commands and deviations are in
[`BUILD-WINDOWS-QT4-PY3.md`](BUILD-WINDOWS-QT4-PY3.md).

Canonical local package:

```text
artifacts\portable\qtiplot-0.9.8.9-qt4-win64-py3-portable.7z
SHA256: 9D1C69007B039D1F4D287B489C0A992A9D42069570E0000E7F3CD45AFA966DE6
Size:   50,489,171 bytes
```

The portable package was refreshed locally on 2026-08-02 after the Date
format/parser changes. No commit or push was performed.

Artifact layout:

```text
artifacts\portable       staged portable folder, 7z, and checksum
artifacts\source-build   archived generated Makefiles, objects, EXE, and runtime
artifacts\verification   retained smoke-test marker
```

`artifacts\` is ignored by Git. The source root was cleaned of generated DLLs,
Python runtime directories, object files, and Makefiles. Run qmake before
`mingw32-make` when rebuilding from the cleaned checkout.

The external toolchains remain under:

```text
A:\toolchains\qtiplot\TDM-GCC-9.2.0
A:\toolchains\qtiplot\Qt-4.8.7-x64-gcc920
A:\toolchains\qtiplot\Qt-4.8.7-reference-runtime
A:\toolchains\qtiplot\Python37-qt4-x64
C:\Users\c\AppData\Local\Python\pythoncore-3.7-64
```

## Verified Runtime Evidence

The project-local portable folder passed:

- embedded Python smoke: exit 0
- GUI startup: stayed alive for six seconds
- direct executable startup: stayed alive
- extracted-archive embedded Python smoke: exit 0
- `7z t` archive integrity check: exit 0

Verified marker:

```text
python=3.7.9
pyqt=4.12.3
qt=4.8.7
qti_app=True
```

The marker is retained at:

```text
artifacts\verification\portable-smoke-20260802-date-refresh\qtiplot-0.9.8.9-qt4-win64-py3-portable\qtiplot_py3_smoke.out
```

## Reconstruction Details That Must Be Preserved

- The tagged repository omitted `qtiplot\src\lib`; it was restored from the
  upstream QtiPlot 0.9.8.9 source archive.
- Missing Boost 1.47 header
  `3rdparty\boost\boost\preprocessor\debug\error.hpp` was restored.
- Required source patches are:
  - `build\patches\qt-4.8.7-uic-widget-attributes.patch`
  - `build\patches\qtiplot-qflags-qt4.patch`
- The final runtime uses the reference QtCore/QtGui PyQt4 wrappers with the
  locally built SIP bridge. The fully local QtCore/QtGui wrappers did not
  initialize reliably with this legacy stack.
- Multi-line command-line Python scripts showed legacy `ScriptEdit` behavior;
  use the one-line probe in `build\probes\qtiplot_py3_smoke.py` for automated
  runtime verification.
- The current tagged code already contains the 2026-06-27 MDI table-border
  resize fix. Include that behavior in the GUI regression pass.

## Safe Next Actions

1. Read this file and `BUILD-WINDOWS-QT4-PY3.md`.
2. Recheck `git status --short --branch` and verify the portable artifact hash.
3. Relaunch the portable executable and complete the GUI diagnostic matrix.
4. Produce a prioritized list of reproducible glitches before editing code.
5. If fixes are authorized, implement and test them one issue at a time.
6. Review the exact commit contents before any commit or push.
