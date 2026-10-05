# QtiPlot Lazarus Port

Free Pascal / LCL rewrite of the QtiPlot workflows actually used. See
[PORT-PLAN.md](PORT-PLAN.md) for scope, milestones, and library decisions.

## Build (this machine: DGX Spark, aarch64, Ubuntu 24.04, X11)

Uses the self-contained toolchain under `~/tools` (workspace convention):

```sh
. ~/tools/lazarus-4.8/env.sh          # puts ~/tools/fpc-3.2.2 on PATH
export PATH="$HOME/tools/lazarus-4.8/bin:$PATH"
cd lazarus-port
lazbuild --build-all qtiplot-laz.lpi  # gtk2 widgetset, aarch64-linux
./bin/qtiplot-laz
```

## Spike 0

`src/frmmain.pas` — a window with a small table (i, i^2, ISO 8601 dates) and
a smoke-test button. Proves: fpc + LCL gtk2 units link, .lfm resources embed,
gtk2 runtime runs on X11 (`DISPLAY=:1`).

Acceptance: binary builds with `lazbuild` and the grid shows computed rows.
