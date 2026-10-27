$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
& "$PSScriptRoot/build.ps1"
$template=Get-Content "$PSScriptRoot/verify-emulator.tcl" -Raw
$symbols=Get-Content "$root/build/eagle.noi" -Raw
foreach($name in @('px','py','camx','keys','ammo','floor_index','map_mode','world')){
    $match=[regex]::Match($symbols,('DEF _'+$name+' (0x[0-9A-Fa-f]+)'))
    if(!$match.Success){throw "Simbolo mancante: $name"}
    $template=$template.Replace('@'+$name.ToUpper()+'@',$match.Groups[1].Value)
}
New-Item -ItemType Directory -Force "$root/work" | Out-Null
Set-Content "$root/work/verify-current.tcl" $template
Write-Host 'Verifica preparata. Dalla cartella del progetto eseguire:'
Write-Host '$env:OPENMSX_HOME="work/emulator"; $env:OPENMSX_SYSTEM_DATA="tools/openmsx/share"; $env:OPENMSX_USER_DATA="work/emulator/share"'
Write-Host './tools/openmsx/openmsx.exe -machine C-BIOS_MSX1_EU -cart build/eagle.rom -romtype Normal -script work/verify-current.tcl'
Write-Host 'Il test stampa CHECKS_ALL_PASSED se tutti i controlli sono superati.'
