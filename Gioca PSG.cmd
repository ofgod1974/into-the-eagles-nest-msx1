@echo off
cd /d "%~dp0"
set "OPENMSX_HOME=work/emulator"
set "OPENMSX_SYSTEM_DATA=tools/openmsx/share"
set "OPENMSX_USER_DATA=work/emulator/share"
"%~dp0tools\openmsx\openmsx.exe" -machine C-BIOS_MSX1_EU -cart build/eagle.rom -romtype Normal -script scripts/play.tcl
