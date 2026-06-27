# QtiPlot 0.9.8.9 Qt4 Win64 Python 3 Build

Date: 2026-06-27

This is an alternate route for avoiding a Qt migration while still moving the
public QtiPlot 0.9.8.9 line to a 64-bit Windows build with Python 3 scripting.
It is separate from the Qt 6 modernization route described in the main handover
docs.

## Result

The probe build is viable in this source tree after building:

```text
.\qtiplot.exe
```

Verified runtime marker:

```text
python=3.7.9
pyqt=4.12.3
qt=4.8.7
qti_app=True
```

The executable was verified as `pei-x86-64` and imports `python37.dll`.

The local build directory has also been staged so this direct launch works
without a development PATH:

```text
C:\localdata\projects\qtiplot-0.9.8.9-qt4-win64-py3\qtiplot.exe
```

## Toolchain

```text
GCC:    TDM-GCC 9.2.0 x86_64-w64-mingw32
Qt:     Qt 4.8.7 x64, built with GCC 9.2.0
Python: CPython 3.7.9 x64
SIP:    4.19.25
PyQt:   4.12.3
```

Local paths used by the probe:

```text
C:\localdata\dev\Dev-Cpp\TDM-GCC-64
C:\localdata\dev\qtiplot-tools\qt\4.8.7-x64-gcc920
C:\Users\c\AppData\Local\Python\pythoncore-3.7-64
C:\localdata\dev\qtiplot-tools\python\Python37-qt4-x64
```

The SIP and PyQt sources used were Riverbank's official SIP 4.19.25 and PyQt4
4.12.3 archives.

## Build

From PowerShell:

```powershell
$qt='C:\localdata\dev\qtiplot-tools\qt\4.8.7-x64-gcc920'
$mingw='C:\localdata\dev\Dev-Cpp\TDM-GCC-64\bin'
$src='C:\localdata\projects\qtiplot-0.9.8.9-qt4-win64-py3'
$prefix='C:\localdata\dev\qtiplot-tools\python\Python37-qt4-x64'
$py37root='C:\Users\c\AppData\Local\Python\pythoncore-3.7-64'

$env:PATH="$mingw;$qt\bin;$qt\lib;$prefix\bin;$py37root;C:\Windows\System32;C:\Windows;C:\Windows\System32\Wbem"
$env:QTDIR=$qt
$env:QMAKESPEC=(Join-Path $qt 'mkspecs\win32-g++')
$env:PYTHONPATH="$prefix\Lib\site-packages"
$env:QTIPLOT_PYQT_PREFIX=$prefix

Set-Location $src
cmd /c "mingw32-make -j4"
```

Important: after changing `SCRIPTING_LANGS` or Python-related defines, do a
clean rebuild. A partial rebuild produced stale object layout and startup
crashes during the probe.

Also do a clean rebuild after changing `MdiSubWindow` virtual functions or data
layout. Derived window classes and SIP objects can otherwise retain stale vtable
or object-layout assumptions.

## Runtime Verification

Use the same runtime environment shape as the probe:

```powershell
$qt='C:\localdata\dev\qtiplot-tools\qt\4.8.7-x64-gcc920'
$mingw='C:\localdata\dev\Dev-Cpp\TDM-GCC-64\bin'
$src='C:\localdata\projects\qtiplot-0.9.8.9-qt4-win64-py3'
$prefix='C:\localdata\dev\qtiplot-tools\python\Python37-qt4-x64'
$py37root='C:\Users\c\AppData\Local\Python\pythoncore-3.7-64'

$env:PATH="$mingw;$qt\bin;$qt\lib;$src;$prefix\bin;$prefix\Lib\site-packages\PyQt4;$py37root;$py37root\DLLs;C:\Windows\System32;C:\Windows;C:\Windows\System32\Wbem"
$env:PYTHONHOME=$py37root
$env:PYTHONPATH="$src\qtiplot;$prefix\Lib\site-packages;$py37root\Lib;$py37root\DLLs"
```

Architecture/import check:

```powershell
& "$mingw\objdump.exe" -f "$src\qtiplot.exe"
& "$mingw\objdump.exe" -p "$src\qtiplot.exe" |
  Select-String -Pattern 'DLL Name:|python|Qt(Core|Gui|Network|OpenGL|Svg|Xml|3Support)4|qwtplot3d'
```

Startup smoke:

```powershell
$p=Start-Process -FilePath (Join-Path $src 'qtiplot.exe') `
  -ArgumentList @('--default-settings') `
  -WorkingDirectory $src -WindowStyle Hidden -PassThru
Start-Sleep -Seconds 6
$p.HasExited
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force }
```

Embedded Python smoke:

```powershell
$out='C:\localdata\projects\qtiplot\build\probes\qtiplot_py3_smoke.out'
Remove-Item -LiteralPath $out -Force -ErrorAction SilentlyContinue
$p=Start-Process -FilePath (Join-Path $src 'qtiplot.exe') `
  -ArgumentList @('-d','-X','C:\localdata\projects\qtiplot\build\probes\qtiplot_py3_smoke.py') `
  -WorkingDirectory $src -WindowStyle Hidden -PassThru
$p.WaitForExit(12000)
Get-Content -LiteralPath $out
```

Expected marker:

```text
python=3.7.9
pyqt=4.12.3
qt=4.8.7
qti_app=True
```

## Portable 7z Package

The portable staging script is:

```text
tools\make_qt4_x64_py3_portable.ps1
```

To regenerate the portable folder, build the 7z, and mirror the runtime files
back into the source folder for direct `qtiplot.exe` launch:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools\make_qt4_x64_py3_portable.ps1 -InstallRuntimeToSource
```

Current local artifact:

```text
Release tag: v0.9.8.9-qt4-win64-py3-r2
dist\qtiplot-0.9.8.9-qt4-win64-py3-portable.7z
SHA256: 4121BE8266BB24211EA2467F90E05403F071A456813F3036D73C85D773DC34B2
Size:   33,083,269 bytes
```

The portable folder includes:

- `qtiplot.exe`, `qwtplot3d.dll`, and the required Qt 4 DLLs beside the EXE
- `python37.dll`, `python3.dll`, `vcruntime140.dll`
- Python 3.7 `Lib` and `DLLs`
- PyQt4/SIP runtime under `Lib\site-packages`
- QtiPlot Python config files beside the EXE
- `manual`, `translations`, and `fitPlugins`
- `qtiplot-portable.cmd` as an optional launcher for hostile host environments

Clean-environment verification passed from both the source build directory and
the portable `dist` directory with:

```text
PATH=C:\Windows\System32;C:\Windows;C:\Windows\System32\Wbem
PYTHONHOME unset
PYTHONPATH unset
```

Results:

```text
source-dir embedded Python smoke: exit 0
dist-dir embedded Python smoke:   exit 0
GUI start from source-dir:        stayed alive
GUI start from dist-dir:          stayed alive
plain EXE start from source-dir:  stayed alive
plain EXE start from dist-dir:    stayed alive
extracted 7z embedded smoke:      exit 0
```

## 2026-06-27 MDI Table Resize Fix

Symptom: after loading CSV data into a table, the table MDI window could be
moved and programmatically resized, but border dragging did not enter resize
mode. Cursor sweeps over all borders stayed at `ArrowCursor`.

Fix: `MdiSubWindow` now enables mouse tracking and intercepts MDI frame mouse
events at `event()` level. If Qt4's built-in MDI hit-test misses a frame edge,
QtiPlot applies its own edge cursor and drag-resize fallback. The resize state
is stored outside the C++ object layout to avoid breaking the existing SIP/Python
bindings.

Regression checks used:

```text
qtiplot_newtable_probe_oneline.py: exit 0
qtiplot_csv_resize_probe_oneline.py: resize_changed=True
qtiplot_mdi_resize_cursor_sweep_oneline.py: top/bottom/left/right/corners show resize cursors
qtiplot_mdi_direct_move_resize_probe_oneline.py: cursor_at_resize=8, resize_changed=True
```

## Publishing Recommendation

For an online release, publish this source repository and attach the 7z to a
GitHub Release. Keep the separate `qtiplot-modern-windows-x64` repository for
the Qt 6 modernization line unless intentionally renaming/splitting projects.

Because QtiPlot is GPL, the release should include or link the exact
corresponding source and build notes for the binary artifact. The 7z is the
right binary format for this route; avoid committing the 7z into git history.

## Probe Caveats

- This started as a proof build; broader manual GUI testing is still useful
  before treating it as a polished release.
- Qt is still Qt 4.8.7; this route deliberately avoids Qt migration.
- Python scripting works with Python 3.7 x64 and PyQt4, but this is not a
  promise that every legacy Python 2 script is source-compatible.
- Multi-line command-line scripts exposed old `ScriptEdit` behavior during
  probing. Single-line script execution works, and normal interactive script
  usage still needs broader manual testing.
- Optional TAMUANOVA/ANOVA SIP binding was removed because the dependency is
  not available in this probe.
