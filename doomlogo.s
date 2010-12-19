.setcpu "6502"
.export _drawLogo

.segment "CODE"

bitmap = $1600

logo:
.include "doomlogo.inc"

_drawLogo:

lda #(8+2)
sta $900f

ldy #0
lda #(8+7)
color:
sta $942c,y
iny
bne color

ldx #0
ldy #0

copy1:
lda logo,x
inx
sta bitmap,y
iny
sta bitmap,y
iny
sta bitmap,y
iny
sta bitmap,y
iny
bne copy1

copy2:
lda logo,x
inx
sta bitmap+$100,y
iny
sta bitmap+$100,y
iny
sta bitmap+$100,y
iny
sta bitmap+$100,y
iny
bne copy2

rts
