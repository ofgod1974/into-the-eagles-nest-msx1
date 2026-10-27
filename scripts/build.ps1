$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$fusion = Join-Path $root 'vendor/fusion-c/Working Folder/fusion-c'
$cc = Join-Path $root 'tools/sdcc/bin/sdcc.exe'
if(!(Test-Path $cc)){throw 'Compilatore mancante: eseguire node scripts/setup-toolchain.cjs'}
New-Item -ItemType Directory -Force (Join-Path $root 'build') | Out-Null
$env:PATH = (Join-Path $root 'tools/sdcc/bin') + ';' + $env:PATH
Push-Location (Join-Path $root 'build')
try {
    # Rebuild the C routines we use with the same ABI as the game.
    $objects = @()
    foreach($module in @('spriteSmall','spritereset','setspritepattern','joystickread','triggerread','sprite16','setcolors')) {
        if(!(Test-Path "$module.rel")) {
            & $cc -mz80 --sdcccall 0 --disable-warning 296 -c "$fusion/source/lib/$module.c" -o "$module.rel"
            if($LASTEXITCODE){throw "Errore Fusion-C: $module"}
        }
        $objects += "$module.rel"
    }
    & $cc -mz80 --sdcccall 0 --disable-warning 296 -I "$fusion/header" -c "$root/src/msx1_vram.c" -o msx1_vram.rel
    if($LASTEXITCODE){throw 'Errore accesso VRAM MSX1'}
    $objects += 'msx1_vram.rel'
    & $cc -mz80 --sdcccall 0 --disable-warning 296 --opt-code-size -I "$fusion/header" -c "$root/src/audio.c" -o audio.rel
    if($LASTEXITCODE){throw 'Errore driver audio'}
    & (Join-Path $root 'tools/sdcc/bin/sdasz80.exe') -o audio_irq.rel "$root/src/audio_irq.s"
    if($LASTEXITCODE){throw 'Errore interrupt audio'}
    $objects += @('audio.rel','audio_irq.rel')
    & $cc -mz80 --sdcccall 0 --disable-warning 296 --disable-warning 357 --no-std-crt0 --code-loc 0x4050 --data-loc 0xC000 --opt-code-size -I "$fusion/header" -L "$fusion/lib" "$fusion/include/crt0_MSX32k_ROM4000.rel" @objects "$root/src/main.c" fusion.lib -o eagle.ihx
    if ($LASTEXITCODE) { throw 'Compilazione fallita' }
    $rom = New-Object byte[] 32768
    for ($i=0;$i -lt $rom.Length;$i++) { $rom[$i]=255 }
    foreach ($line in Get-Content eagle.ihx) {
        if (!$line.StartsWith(':')) { continue }
        $n=[Convert]::ToInt32($line.Substring(1,2),16)
        $a=[Convert]::ToInt32($line.Substring(3,4),16)
        $type=[Convert]::ToInt32($line.Substring(7,2),16)
        $sum=0
        for($j=1;$j -lt $line.Length;$j+=2){$sum += [Convert]::ToInt32($line.Substring($j,2),16)}
        if(($sum -band 255) -ne 0){throw 'Checksum Intel HEX non valido'}
        if($type -eq 0){for($j=0;$j -lt $n;$j++){
            if($a+$j -lt 0x4000 -or $a+$j -ge 0xC000){throw 'Codice fuori dalla ROM'}
            $rom[$a+$j-0x4000]=[Convert]::ToByte($line.Substring(9+$j*2,2),16)
        }}
    }
    if($rom[0] -ne 65 -or $rom[1] -ne 66){throw 'Header ROM AB mancante'}
    [IO.File]::WriteAllBytes((Join-Path $root 'build/eagle.rom'),$rom)
    Write-Host 'ROM MSX1 generata: build/eagle.rom (32768 byte)'
} finally { Pop-Location }
