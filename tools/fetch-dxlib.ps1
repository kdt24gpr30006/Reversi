# DxLib (VisualC++版) を公式サイトから取得し third_party/DxLib に配置する。
# 使い方: pwsh -File tools/fetch-dxlib.ps1 [-Version 3_25a]
param(
    [string]$Version = "3_25a"
)
$ErrorActionPreference = "Stop"

$root      = Split-Path -Parent $PSScriptRoot
$dest      = Join-Path $root "third_party/DxLib"
$work      = Join-Path $root "third_party/_download"
$zipName   = "DxLib_VC$Version.zip"
$zipPath   = Join-Path $work $zipName
$url       = "https://dxlib.xsrv.jp/DxLib/$zipName"

if (Test-Path (Join-Path $dest "DxLib.h")) {
    Write-Host "DxLib は既に配置済みです: $dest"
    exit 0
}

New-Item -ItemType Directory -Force $work | Out-Null
if (-not (Test-Path $zipPath)) {
    Write-Host "ダウンロード中: $url"
    Invoke-WebRequest -Uri $url -OutFile $zipPath -UseBasicParsing
}

$extract = Join-Path $work "extract"
if (Test-Path $extract) { Remove-Item -Recurse -Force $extract }
Write-Host "展開中..."
Expand-Archive -Path $zipPath -DestinationPath $extract

# 「プロジェクトに追加すべきファイル_VC用」フォルダ（DxLib.h を含むフォルダ）を探す
$header = Get-ChildItem -Path $extract -Recurse -Filter "DxLib.h" | Select-Object -First 1
if (-not $header) { throw "展開物に DxLib.h が見つかりません" }

New-Item -ItemType Directory -Force $dest | Out-Null
Copy-Item -Path (Join-Path $header.DirectoryName "*") -Destination $dest -Recurse -Force
Remove-Item -Recurse -Force $extract
& (Join-Path $PSScriptRoot "prune-dxlib.ps1")
Write-Host "配置完了: $dest"
