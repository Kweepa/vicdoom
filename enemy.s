.setcpu	"6502"
.autoimport	on
.importzp sp

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

cmp dy
bmi :+
; dx > dy, so dx+dy - dy/2
clc
adc dy
asl dy
sec
sbc dy
ldy #2
jmp addysp

:
; dx < dy, so dx+dy - dx/2
clc
adc dy
asl dx
sec
sbc dx
ldy #2
jmp addysp
