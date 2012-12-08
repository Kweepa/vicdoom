.setcpu	"6502"
.autoimport	on
.importzp sp

.segment "LOWCODE"

;
; P_ApproxDistance
; Gives an estimation of distance (not exact)
;

.export _P_ApproxDistance

dy:
.byte 0
dx:
.byte 0

_P_ApproxDistance:

; TOS - dx
; AX - dy

; just bin the lower byte

txa
bpl :+
eor #$ff
clc
adc #1
:
sta dy


ldy #1
lda (sp),y
bpl :+
eor #$ff
clc
adc #1
:
sta dx

; dx > dy, dx+dy/2
; dx < dy, dx/2+dy

cmp dy
bpl shiftY
lsr
jmp :+
shiftY:
lsr dy
:
clc
adc dy
ldy #2
jmp addysp
