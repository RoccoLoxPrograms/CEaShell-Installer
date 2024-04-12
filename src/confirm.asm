assume adl = 1
section .text

include 'ti84pceg.inc'

_confirm_delete_vars:
    ld iy, ti.flags
    ld a, 4
    ld (ti.curRow), a
    res ti.textInverse, (iy + ti.textFlags)

.update_display:
    ld hl, 72
    ld de, 253
    ld b, 117
    ld c, 134
    call ti.ClearRect
    ld hl, 6
    ld.sis (ti.curCol and $FFFF), hl
    ld.sis hl, (ti.localLanguage)
    ld de, $010C
    or a, a
    sbc hl, de
    ld hl, option_yes
    jr nz, $ + 6
    ld hl, option_yes_fr
    call ti.PutS
    ld hl, 16
    ld.sis (ti.curCol and $FFFF), hl
    ld a, (iy + ti.textFlags)
    xor a, 8
    ld (iy + ti.textFlags), a
    ld.sis hl, (ti.localLanguage)
    ld de, $010C
    or a, a
    sbc hl, de
    ld hl, option_no
    jr nz, $ + 6
    ld hl, option_no_fr
    call ti.PutS

.get_key:
    call ti.GetCSC
    cp a, ti.skRight
    jr z, .update_display
    cp a, ti.skLeft
    jr z, .update_display
    cp a, ti.skClear
    jr z, .exit
    cp a, ti.skEnter
    jr nz, .get_key
    bit ti.textInverse, (iy + ti.textFlags)
    res ti.textInverse, (iy + ti.textFlags)
    ld a, 1
    ret z

.exit:
    res ti.textInverse, (iy + ti.textFlags)
    xor a, a
    ret

option_yes:
    db " Yes ", 0

option_no:
    db " No ", 0

option_yes_fr:
    db " Oui ", 0

option_no_fr:
    db " Non ", 0

public _confirm_delete_vars
