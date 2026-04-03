param(
    [string]$QtRoot = "",
    [string]$ConfigurePreset = "vs2022-x64",
    [string]$BuildPreset = "build-release-x64",
    [switch]$DisableDevDeploy
)

$ErrorActionPreference = "Stop"

function Resolve-QtRoot {
    param([string]$RequestedQtRoot)

    if ($RequestedQtRoot) {
        $configPath = Join-Path $RequestedQtRoot "lib/cmake/Qt6/Qt6Config.cmake"
        if (Test-Path $configPath) {
            return (Resolve-Path $RequestedQtRoot).Path
        }
        throw "RO_QT_ROOT '$RequestedQtRoot' does not contain lib/cmake/Qt6/Qt6Config.cmake."
    }

    $compatibleConfigs = Get-ChildItem -Path "C:\Qt" -Filter Qt6Config.cmake -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match "\\msvc\d+_64\\lib\\cmake\\Qt6\\Qt6Config\.cmake$" } |
        Sort-Object FullName -Descending

    if (-not $compatibleConfigs) {
        $detectedConfigs = Get-ChildItem -Path "C:\Qt" -Filter Qt6Config.cmake -Recurse -ErrorAction SilentlyContinue |
            Select-Object -ExpandProperty FullName

        $details = if ($detectedConfigs) {
            "Detected Qt configs:`n - " + ($detectedConfigs -join "`n - ")
        } else {
            "No Qt6Config.cmake files were found under C:\Qt."
        }

        throw "No compiler-compatible MSVC Qt 6 kit was found under C:\Qt. Install a kit such as C:\Qt\6.11.0\msvc2022_64 or pass -QtRoot explicitly.`n$details"
    }

    return $compatibleConfigs[0].Directory.Parent.Parent.Parent.FullName
}

$resolvedQtRoot = Resolve-QtRoot -RequestedQtRoot $QtRoot
$devDeployValue = if ($DisableDevDeploy) { "OFF" } else { "ON" }

Write-Host "Using Qt root: $resolvedQtRoot"

cmake --preset $ConfigurePreset -DRO_ENABLE_QT6_UI_SPIKE=ON -DRO_QT_ROOT="$resolvedQtRoot" -DRO_ENABLE_DEV_DEPLOY=$devDeployValue
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

cmake --build --preset $BuildPreset
exit $LASTEXITCODE
