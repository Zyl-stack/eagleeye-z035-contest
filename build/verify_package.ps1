$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$manifest = Join-Path $root 'SHA256SUMS.txt'
if (-not (Test-Path -LiteralPath $manifest)) { throw 'Missing SHA256SUMS.txt' }
$checked = 0
foreach ($line in Get-Content -LiteralPath $manifest) {
    if ($line -notmatch '^([0-9a-f]{64})  (.+)$') { throw "Invalid manifest line: $line" }
    $expected = $Matches[1]
    $relative = $Matches[2]
    if ($relative -match '(^|/)\.\.(/|$)') { throw "Unsafe manifest path: $relative" }
    $file = Join-Path $root ($relative -replace '/', [IO.Path]::DirectorySeparatorChar)
    if (-not (Test-Path -LiteralPath $file -PathType Leaf)) { throw "Missing: $relative" }
    $actual = (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash
    if ($actual -ine $expected) { throw "SHA256 mismatch: $relative" }
    $checked++
}
Write-Output "PACKAGE_SHA256_OK files=$checked"
