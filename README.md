# QtiPlot 0.9.8.9 Qt4 Win64 Py3

Unofficial Windows 64-bit portable build route for the public GPL
QtiPlot 0.9.8.9 source line, keeping Qt 4 and adding Python 3 scripting.

This repository is not the official QtiPlot project and is not a build of the
current commercial/latest QtiPlot release.

## Build Route

The local proof build uses:

- Qt 4.8.7, 64-bit
- TDM-GCC 9.2.0, x86_64-w64-mingw32
- CPython 3.7.9, 64-bit
- SIP 4.19.25
- PyQt4 4.12.3

Detailed build notes are in [`BUILD-WINDOWS-QT4-PY3.md`](BUILD-WINDOWS-QT4-PY3.md).
The canonical current status and restart instructions are in
[`SESSION_HANDOVER.md`](SESSION_HANDOVER.md).

## Portable Package

The intended binary release asset is:

```text
qtiplot-0.9.8.9-qt4-win64-py3-portable.7z
```

Initial release:

```text
https://github.com/jszm/qtiplot-0.9.8.9-qt4-win64-py3/releases/tag/v0.9.8.9-qt4-win64-py3
```

Current resize-fix refresh:

```text
https://github.com/jszm/qtiplot-0.9.8.9-qt4-win64-py3/releases/tag/v0.9.8.9-qt4-win64-py3-r2
```

After building `qtiplot.exe`, regenerate the project-local portable folder and
archive with:

```powershell
$src='A:\projects\qtiplot-0.9.8.9-qt4-win64-py3'
$qtRuntime='A:\toolchains\qtiplot\Qt-4.8.7-reference-runtime'
$py37root='C:\Users\c\AppData\Local\Python\pythoncore-3.7-64'
$prefix='A:\toolchains\qtiplot\Python37-qt4-x64'
powershell -NoProfile -ExecutionPolicy Bypass -File tools\make_qt4_x64_py3_portable.ps1 `
  -QtDir $qtRuntime -PythonRoot $py37root -PyQtPrefix $prefix `
  -OutRoot (Join-Path $src 'artifacts\portable')
```

The portable package stages the Qt4 DLLs, Python 3.7 runtime, PyQt4/SIP runtime,
manual, translations, and fit plugins beside the application.

Current local refresh:

```text
artifacts\portable\qtiplot-0.9.8.9-qt4-win64-py3-portable.7z
SHA256: 9D1C69007B039D1F4D287B489C0A992A9D42069570E0000E7F3CD45AFA966DE6
Size:   50,489,171 bytes
```

## Status

The project-local portable build passed these clean-environment checks:

- plain `qtiplot.exe` startup from the portable folder
- embedded Python smoke from the portable folder
- embedded Python smoke after extracting the 7z archive
- `7z t` archive integrity check
- MDI table border resize after CSV import

Verified Python marker:

```text
python=3.7.9
pyqt=4.12.3
qt=4.8.7
qti_app=True
```

The 2026-06-27 refresh fixes a Qt4 MDI edge hit-test regression where table
windows could be moved and programmatically resized but not resized by dragging
the window border.

A broader GUI diagnostic focused on responsiveness and plotting is the next
planned task. It was started but cancelled before any glitch was confirmed; the
exact restart point and test matrix are recorded in `SESSION_HANDOVER.md`.

## License

QtiPlot 0.9.8.9 is GPL. Binary releases should include or link the exact
corresponding source and build notes for the published artifact.
