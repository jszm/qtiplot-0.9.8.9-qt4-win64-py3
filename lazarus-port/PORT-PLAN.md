# QtiPlot → Lazarus / Free Pascal Port — Plan (spike stage)

Last updated: 2026-10-05. Status: **spike 0** — toolchain proof on DGX Spark
(aarch64, Ubuntu 24.04, X11 session).

## Why a port, and why both tracks

- The native Qt4 arm64 build (see `../BUILD-WINDOWS-QT4-PY3.md` successors and
  the checkpoint) keeps the existing app usable and is the **behavioral
  reference**: the GUI diagnostic matrix runs against it, and every confirmed
  issue becomes a requirement for the port.
- The Lazarus port is new code: LCL gives a modern, maintained widget toolkit
  on Linux arm64 (gtk2 today; qt5/win later) without dragging Qt4 along
  forever.
- Parity is **not** a big-bang rewrite. QtiPlot is ~180k lines of C++; the
  port targets the workflows actually used, in vertical slices.

## Toolchain (this machine)

- `~/tools/fpc-3.2.2` (per env.sh: `export PATH=~/tools/fpc-3.2.2/bin:$PATH`,
  `PPC_CONFIG_PATH=~/tools/fpc-3.2.2/etc`)
- `~/tools/lazarus-4.8` — lazbuild at `~/tools/lazarus-4.8/bin/lazbuild`,
  LCL gtk2 units precompiled for aarch64-linux.
- System gtk2 runtime present (`libgtk2.0-0t64`); session is X11
  (`DISPLAY=:1`), so gtk2 apps run natively (XWayland also fine).
- New tools/libraries go under `~/tools` per workspace convention; project
  code lives in the repo under `lazarus-port/`.

## Library strategy (decide per milestone, not up front)

| Need | Candidate | Note |
|------|-----------|------|
| Table widget | LCL `TCustomDrawGrid` subclass | virtualized data model; QtiPlot tables can be large |
| 2D plots | custom `TCustomControl` canvas widget first | full control over axes/legend/zoom; TAChart is the fallback |
| CSV/Excel import | fpspreadsheet | also gives .xlsx export |
| .qti project import | FPC `XMLRead` | QtiPlot project files are XML → cheap migration path |
| Fitting | hand-rolled Levenberg–Marquardt (or FPC `numlib`) | start with linear/poly/gauss/user-formula |
| Scripting | defer; candidate PascalScript | Python embedding possible later via ctypes-style dyn-load |

## Milestones (acceptance criteria measurable against the Qt4 reference build)

- **M0 — spike 0 (this):** LCL app compiles with `~/tools` fpc+lazbuild,
  links gtk2, runs on X11: window + button + grid. *Done when the binary
  runs and shows the grid.*
- **M1 — table:** virtualized table (100k rows × 26 cols smooth), CSV
  import, column types (numeric/date/text), "Set Column Values" formulas
  incl. `i`, matching the datetime behavior of the 2026-08 Table rework.
- **M2 — 2D plot:** single-layer line/symbol plot from table columns;
  autoscale, ticks/labels, legend, zoom/pan; PNG + SVG export; 100k points
  redraw < 1 s.
- **M3 — fitting:** LM on linear/poly/gauss/user formula; parameters,
  residuals, goodness-of-fit; results written back to a table/log like
  QtiPlot.
- **M4 — .qti import (read-only):** tables, column formats, curve settings
  from a QtiPlot 0.9.8.9 project file.
- **M5 — workspace UX:** multi-window (MDI or tabbed), print/PDF export.

Out of scope for MVP: 3D plots, Python scripting, remote control, assistant
help, ODS/OPJ import.

## Repo layout

```
lazarus-port/
  PORT-PLAN.md        this file
  README.md           build/run instructions
  qtiplot-laz.lpi     Lazarus project
  src/                program + units
    qtiplot-laz.lpr
    frmmain.pas/.lfm  spike 0 main form
```

## Process rules (inherited from the diagnostic-pass rule)

- No port feature without a reference behavior: screenshot/expected-actual
  pair from the Qt4 arm64 build (or the recorded Windows release).
- Port regressions get the same reproducible-case treatment as Qt4 bugs.
