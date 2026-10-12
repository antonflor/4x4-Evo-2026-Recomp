<#
Restores the original 4x4 Evolution Direct3D 7 renderer.

    powershell -ExecutionPolicy Bypass -File .\uninstall.ps1 -GamePath "C:\Games\4x4 Evolution"

Leaves your textures folder and the [evo2026] settings in place (they're ignored by the
original renderer). Delete them by hand if you want a completely clean folder.
#>
param([Parameter(Mandatory = $true)][string]$GamePath)

$ErrorActionPreference = 'Stop'
$GamePath = $GamePath.Trim('"')
$backup = Join-Path $GamePath 'TRIDX7.original.dll'
$target = Join-Path $GamePath 'TRIDX7.DLL'

if (-not (Test-Path $backup)) { throw "No TRIDX7.original.dll in '$GamePath' - nothing to restore." }

Copy-Item $backup $target -Force
Write-Host 'Original renderer restored (TRIDX7.DLL).'

$layers = 'HKCU:\Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers'
$exe = Join-Path $GamePath '4x4.exe'
if (Get-ItemProperty -Path $layers -Name $exe -ErrorAction SilentlyContinue) {
    Remove-ItemProperty -Path $layers -Name $exe
    Write-Host 'Removed the DPI compatibility flag.'
}
