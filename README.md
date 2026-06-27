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

Detailed build notes are in `BUILD-WINDOWS-QT4-PY3.md`.

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

After building `qtiplot.exe`, regenerate the portable folder and archive with:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools\make_qt4_x64_py3_portable.ps1 -InstallRuntimeToSource
```

The portable package stages the Qt4 DLLs, Python 3.7 runtime, PyQt4/SIP runtime,
manual, translations, and fit plugins beside the application.

Current local refresh:

```text
SHA256: 4121BE8266BB24211EA2467F90E05403F071A456813F3036D73C85D773DC34B2
Size:   33,083,269 bytes
```

## Status

The local proof build passed these clean-environment checks:

- plain `qtiplot.exe` startup from the source build folder
- plain `qtiplot.exe` startup from the portable folder
- embedded Python smoke from the source build folder
- embedded Python smoke from the portable folder
- embedded Python smoke after extracting the 7z archive
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

## License

QtiPlot 0.9.8.9 is GPL. Binary releases should include or link the exact
corresponding source and build notes for the published artifact.
