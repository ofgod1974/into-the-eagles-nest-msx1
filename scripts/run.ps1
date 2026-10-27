$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
& "$PSScriptRoot/build.ps1"
$env:OPENMSX_HOME='work/emulator'
$env:OPENMSX_SYSTEM_DATA='tools/openmsx/share'
$env:OPENMSX_USER_DATA='work/emulator/share'
Push-Location $root
try {& "$root/tools/openmsx/openmsx.exe" -machine C-BIOS_MSX1_EU -cart build/eagle.rom -romtype Normal -ext scc -script scripts/play.tcl}
finally {Pop-Location}
