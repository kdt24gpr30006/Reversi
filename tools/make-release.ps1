# Release ビルド → テスト → release/ に配布用 exe と著作権表記を配置する。
# 使い方: pwsh -File tools/make-release.ps1
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -requires Microsoft.Component.MSBuild -property installationPath
$msbuild = Join-Path $vs "MSBuild\Current\Bin\MSBuild.exe"

& $msbuild ShiftReversi.sln /m /t:Rebuild /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { throw "ビルド失敗" }

& "build\x64\Release\ShiftTests.exe" | Select-Object -Last 1
if ($LASTEXITCODE -ne 0) { throw "テスト失敗（release は更新しません）" }

New-Item -ItemType Directory -Force release | Out-Null
Copy-Item build\x64\Release\ShiftReversi.exe release\ShiftReversi.exe -Force
Copy-Item THIRD_PARTY_NOTICES.txt release\THIRD_PARTY_NOTICES.txt -Force
Write-Host "release/ を更新しました"
