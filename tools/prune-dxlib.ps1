# third_party/DxLib を、このプロジェクトのビルド（x64 / マルチバイト / 静的ランタイム MT・MTd）に
# 必要なファイルだけに絞る。リポジトリに含める DxLib はこのスクリプトの結果。
$ErrorActionPreference = "Stop"
$dest = Join-Path (Split-Path -Parent $PSScriptRoot) "third_party/DxLib"

$keep = {
    param($name)
    if ($name -like "*.h" -or $name -eq "DxLib.txt") { return $true }
    if ($name -like "DxLibW_*" -or $name -like "DxUseCLibW_*") { return $false }  # Unicode 版は不要
    return $name -like "*_vs2015_x64_MT.lib" -or $name -like "*_vs2015_x64_MTd.lib" -or
           $name -like "*_vs2015_x64_ItrDbgLv0_MTd.lib"
}

$removed = 0
Get-ChildItem -Path $dest -File | ForEach-Object {
    if (-not (& $keep $_.Name)) { Remove-Item $_.FullName; $removed++ }
}
Get-ChildItem -Path $dest -Directory | ForEach-Object { Remove-Item -Recurse -Force $_.FullName; $removed++ }
$left = Get-ChildItem -Path $dest -File
Write-Host ("DxLib を整理: {0} 件削除, {1} 件 ({2:N1} MB) を保持" -f $removed, $left.Count, (($left | Measure-Object Length -Sum).Sum / 1MB))
