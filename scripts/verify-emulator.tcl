# Tokens are filled from the current linker symbol file by verify.ps1.
puts stderr VERIFY_START
set pause_on_lost_focus false
set pause false
set throttle false
set px @PX@
set py @PY@
set camx @CAMX@
set keys @KEYS@
set ammo @AMMO@
set floor_index @FLOOR_INDEX@
set map_mode @MAP_MODE@
set world @WORLD@
set failed 0
proc word {a} {expr {[debug read memory $a]+256*[debug read memory [expr {$a+1}]]}}
proc setword {a value} {debug write memory $a [expr {$value&255}]; debug write memory [expr {$a+1}] [expr {$value>>8}]}
proc ensure {condition message} {global failed;if {![uplevel 1 [list expr $condition]]} {puts stderr "FAIL: $message";set failed 1} else {puts stderr "PASS: $message"}}
proc capture {name} {after frame [list after frame [list screenshot -raw -size 640 $name]]}
after time 5 {
 ensure {[debug read memory $floor_index]==1 && [debug read memory $ammo]==99} {initial ground floor and inventory}
 capture castle-ground.png; keymatrixdown 8 16
}
after time 6.5 {keymatrixup 8 16;keymatrixdown 8 32}
after time 7.2 {keymatrixup 8 32;ensure {[debug read memory $keys]==1} {walk to and collect original starting-room key}}
# Position fixture tests the same door from its southern side.
after time 8 {setword $px 272;setword $py 456;keymatrixdown 8 32}
after time 10 {
 keymatrixup 8 32
 ensure {[debug read memory $keys]==0 && [word $py]<430} {consume key and cross metal door}
 ensure {[debug read memory [expr {$world+56*64+34}]]==6} {door removed from world map}
 capture castle-door.png
}
after time 11 {type_via_keybuf m}
after time 13 {ensure {[debug read memory $map_mode]==1} {open complete floor plan};capture castle-map.png}
after time 14 {type_via_keybuf m}
after time 16 {ensure {[debug read memory $map_mode]==0} {return from floor plan}}
after time 17 {type_via_keybuf 1}
after time 19 {ensure {[debug read memory $floor_index]==0} {load basement};capture castle-basement.png}
after time 20 {type_via_keybuf 3}
after time 22 {
 ensure {[debug read memory $floor_index]==2} {load first floor}
 setword $px 272;setword $py 192;keymatrixdown 8 128
}
after time 22.2 {keymatrixup 8 128}
after time 24 {ensure {[debug read memory $camx]>0} {camera follows movement to another room};capture castle-first.png}
after time 25 {type_via_keybuf 4}
after time 27 {
 ensure {[debug read memory $floor_index]==3} {load second floor}
 setword $px 240;setword $py 192;keymatrixdown 8 128
}
after time 27.2 {keymatrixup 8 128}
after time 29 {capture castle-second.png;keymatrixdown 8 1}
after time 29.2 {keymatrixup 8 1}
after time 30 {ensure {[debug read memory $ammo]==98} {shoot and consume ammunition}}
after time 31 {type_via_keybuf 2}
after time 33 {
 ensure {[debug read memory $floor_index]==1} {return to ground floor}
 ensure {[debug read memory [expr {$world+56*64+34}]]==6} {opened door persists after changing floors}
 type_via_keybuf e
}
after time 36 {ensure {[debug read memory $floor_index]==2} {elevator changes floor}}
after time 37 {if {!$failed} {puts stderr CHECKS_ALL_PASSED};set throttle true;exit}
