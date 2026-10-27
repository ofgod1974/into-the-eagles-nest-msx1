.module eagle_audio_irq
.globl _AudioInstallHook
.globl _AudioReadSlot
.globl _AudioWriteSlot
.globl _AudioPSG
.globl _AudioTick
.globl _audio_old_hook
.area _DATA
_audio_old_hook:: .ds 5
.area _CODE
; Called only while interrupts are disabled. Preserve and chain the BIOS hook.
_AudioInstallHook::
 ld hl,#0xfd9f
 ld de,#_audio_old_hook
 ld bc,#5
 ldir
 ld a,#0xc3
 ld (0xfd9f),a
 ld hl,#audio_irq
 ld (0xfda0),hl
 ld a,#0xc9
 ld (0xfda2),a
 ld (0xfda3),a
 ret
audio_irq:
 push af
 push bc
 push de
 push hl
 push ix
 push iy
 exx
 ex af,af'
 push af
 push bc
 push de
 push hl
 call _AudioTick
 pop hl
 pop de
 pop bc
 pop af
 ex af,af'
 exx
 pop iy
 pop ix
 pop hl
 pop de
 pop bc
 pop af
 jp _audio_old_hook
; All public calls use the Fusion-C stack ABI (--sdcccall 0).
_AudioReadSlot::
 push ix
 ld ix,#0
 add ix,sp
 ld a,4(ix)
 ld l,5(ix)
 ld h,6(ix)
 push iy
 call 0x000c
 di
 pop iy
 ld l,a
 pop ix
 ret
_AudioWriteSlot::
 push ix
 ld ix,#0
 add ix,sp
 ld a,4(ix)
 ld l,5(ix)
 ld h,6(ix)
 ld e,7(ix)
 push iy
 call 0x0014
 di
 pop iy
 pop ix
 ret
_AudioPSG::
 ld hl,#2
 add hl,sp
 ld a,(hl)
 out (0xa0),a
 inc hl
 ld a,(hl)
 out (0xa1),a
 ret
