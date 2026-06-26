param(
  [string]$SourceDir = (Join-Path $PSScriptRoot '..'),
  [string]$QtDir = 'C:\localdata\dev\qtiplot-tools\qt\4.8.7-x64-gcc920',
  [string]$PythonRoot = 'C:\Users\c\AppData\Local\Python\pythoncore-3.7-64',
  [string]$PyQtPrefix = 'C:\localdata\dev\qtiplot-tools\python\Python37-qt4-x64',
  [string]$OutRoot = (Join-Path $PSScriptRoot '..\dist'),
  [string]$PackageName = 'qtiplot-0.9.8.9-qt4-win64-py3-portable',
  [string]$SevenZip = 'C:\Program Files\7-Zip\7z.exe',
  [switch]$InstallRuntimeToSource
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolve-FullPath([string]$Path) {
  return [System.IO.Path]::GetFullPath($Path)
}

function Assert-File([string]$Path) {
  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    throw "Required file not found: $Path"
  }
}

function Assert-Directory([string]$Path) {
  if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
    throw "Required directory not found: $Path"
  }
}

function Copy-RequiredFile([string]$From, [string]$ToDirectory) {
  Assert-File $From
  New-Item -ItemType Directory -Force -Path $ToDirectory | Out-Null
  Copy-Item -LiteralPath $From -Destination $ToDirectory -Force
}

function Invoke-Robocopy([string]$From, [string]$To, [string[]]$ExtraArgs = @()) {
  Assert-Directory $From
  New-Item -ItemType Directory -Force -Path $To | Out-Null
  $args = @($From, $To, '/E', '/NFL', '/NDL', '/NJH', '/NJS', '/NP') + $ExtraArgs
  & robocopy @args | Out-Null
  if ($LASTEXITCODE -gt 7) {
    throw "robocopy failed with exit code $LASTEXITCODE while copying $From to $To"
  }
}

function Write-TextFile([string]$Path, [string]$Content) {
  New-Item -ItemType Directory -Force -Path (Split-Path -Parent $Path) | Out-Null
  [System.IO.File]::WriteAllText($Path, $Content, [System.Text.Encoding]::ASCII)
}

$sourceFull = Resolve-FullPath $SourceDir
$outRootFull = Resolve-FullPath $OutRoot
$portableFull = Resolve-FullPath (Join-Path $outRootFull $PackageName)

Assert-Directory $sourceFull
Assert-Directory $QtDir
Assert-Directory $PythonRoot
Assert-Directory $PyQtPrefix

if (-not $portableFull.StartsWith($outRootFull.TrimEnd('\') + '\', [System.StringComparison]::OrdinalIgnoreCase)) {
  throw "Refusing to stage outside output root: $portableFull"
}

New-Item -ItemType Directory -Force -Path $outRootFull | Out-Null
if (Test-Path -LiteralPath $portableFull) {
  Remove-Item -LiteralPath $portableFull -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $portableFull | Out-Null

$qtDlls = @(
  'Qt3Support4.dll',
  'QtCore4.dll',
  'QtGui4.dll',
  'QtNetwork4.dll',
  'QtOpenGL4.dll',
  'QtSql4.dll',
  'QtSvg4.dll',
  'QtXml4.dll'
)

$pythonRootFiles = @(
  'python3.dll',
  'python37.dll',
  'vcruntime140.dll',
  'LICENSE.txt'
)

$qtiConfigFiles = @(
  'qtiplotrc.py',
  'qtiplot_remote_ctl.py',
  'qtiUtil.py',
  'qti_wordlist.txt',
  'remote_ctl.py'
)

Copy-RequiredFile (Join-Path $sourceFull 'qtiplot.exe') $portableFull
Copy-RequiredFile (Join-Path $sourceFull 'qwtplot3d.dll') $portableFull

foreach ($dll in $qtDlls) {
  Copy-RequiredFile (Join-Path $QtDir "bin\$dll") $portableFull
}

foreach ($file in $pythonRootFiles) {
  Copy-RequiredFile (Join-Path $PythonRoot $file) $portableFull
}

foreach ($file in $qtiConfigFiles) {
  Copy-RequiredFile (Join-Path $sourceFull "qtiplot\$file") $portableFull
}

foreach ($file in @('gpl_licence.txt', 'www.gnu.org licenses gpl.txt', 'README.html', 'qtiplot.css', 'qtiplot_logo.png')) {
  Copy-RequiredFile (Join-Path $sourceFull $file) $portableFull
}

Invoke-Robocopy (Join-Path $PythonRoot 'Lib') (Join-Path $portableFull 'Lib') @('/XD', '__pycache__', '/XF', '*.pyc', '*.pyo')
Invoke-Robocopy (Join-Path $PythonRoot 'DLLs') (Join-Path $portableFull 'DLLs')
Invoke-Robocopy (Join-Path $PyQtPrefix 'Lib\site-packages') (Join-Path $portableFull 'Lib\site-packages') @('/XD', '__pycache__', '/XF', '*.pyc', '*.pyo')

Invoke-Robocopy (Join-Path $sourceFull 'manual') (Join-Path $portableFull 'manual')
Invoke-Robocopy (Join-Path $sourceFull 'qtiplot\translations') (Join-Path $portableFull 'translations')
Invoke-Robocopy (Join-Path $sourceFull 'fitPlugins') (Join-Path $portableFull 'fitPlugins')

Write-TextFile (Join-Path $portableFull 'qt.conf') @'
[Paths]
Plugins=plugins
Translations=translations
'@

Write-TextFile (Join-Path $portableFull 'qtiplot-portable.cmd') @'
@echo off
set "QTIPLOT_HOME=%~dp0"
set "PYTHONHOME=%QTIPLOT_HOME%"
set "PYTHONPATH=%QTIPLOT_HOME%Lib\site-packages;%QTIPLOT_HOME%Lib;%QTIPLOT_HOME%DLLs"
start "" "%QTIPLOT_HOME%qtiplot.exe" %*
'@

Write-TextFile (Join-Path $portableFull 'README-PORTABLE.txt') @"
QtiPlot 0.9.8.9 Qt4 x64 Python 3 portable build

Run qtiplot.exe directly from this folder. If the host has conflicting Qt or
Python environment variables, run qtiplot-portable.cmd instead.

Runtime contents:
- QtiPlot 0.9.8.9 built with Qt 4.8.7, TDM-GCC 9.2 x64, Python 3.7 x64.
- Qt4 DLLs are staged beside qtiplot.exe.
- Python 3.7 runtime and PyQt4 runtime are staged under this folder.

Licenses:
- QtiPlot is GPL. See gpl_licence.txt and source distribution terms.
- Python license text is included as LICENSE.txt.

For redistribution, publish this archive together with the matching source tree
and build notes, or publish a clear source offer for the exact build.
"@

$archivePath = Join-Path $outRootFull "$PackageName.7z"
if (Test-Path -LiteralPath $archivePath) {
  Remove-Item -LiteralPath $archivePath -Force
}
if (Test-Path -LiteralPath $SevenZip -PathType Leaf) {
  Push-Location $outRootFull
  try {
    & $SevenZip a -t7z -mx=9 "$PackageName.7z" $PackageName | Out-Null
    if ($LASTEXITCODE -ne 0) {
      throw "7z failed with exit code $LASTEXITCODE"
    }
  } finally {
    Pop-Location
  }
  Get-FileHash -Algorithm SHA256 -LiteralPath $archivePath |
    ForEach-Object { "$($_.Hash)  $(Split-Path -Leaf $archivePath)" } |
    Set-Content -LiteralPath "$archivePath.sha256" -Encoding ASCII
}

if ($InstallRuntimeToSource) {
  foreach ($file in ($qtDlls + $pythonRootFiles + $qtiConfigFiles + @('qt.conf', 'qtiplot-portable.cmd', 'README-PORTABLE.txt'))) {
    Copy-RequiredFile (Join-Path $portableFull $file) $sourceFull
  }
  Invoke-Robocopy (Join-Path $portableFull 'Lib') (Join-Path $sourceFull 'Lib') @('/XD', '__pycache__', '/XF', '*.pyc', '*.pyo')
  Invoke-Robocopy (Join-Path $portableFull 'DLLs') (Join-Path $sourceFull 'DLLs')
  Invoke-Robocopy (Join-Path $portableFull 'translations') (Join-Path $sourceFull 'translations')
}

[pscustomobject]@{
  PortableRoot = $portableFull
  Archive = if (Test-Path -LiteralPath $archivePath) { $archivePath } else { $null }
  ArchiveSha256 = if (Test-Path -LiteralPath "$archivePath.sha256") { "$archivePath.sha256" } else { $null }
  InstalledRuntimeToSource = [bool]$InstallRuntimeToSource
}
