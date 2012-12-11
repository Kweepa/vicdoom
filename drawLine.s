.setcpu "6502"
.autoimport on
.import addysp
.importzp sp
.export _automap_draw
.export _drawLine
.export _automap_sawEdge
.export _automap_resetEdges
.export _automap_setEdges
.import _getSinOf
.import _getCosOf

buffer = $BE00

.segment "LOWCODE"

minx:
.word 0
maxx:
.word 0
miny:
.word 0
maxy:
.word 0
player_x:
.word 0
player_y:
.word 0
player_a:
.byte 0
sina:
.word 0
cosa:
.word 0
zoom:
.byte 0
shift:
.byte 128, 64, 32, 16, 8, 4, 2, 1
edgesSeen:
.res 32, 0

_automap_resetEdges:
ldx #31
lda #0
:
sta edgesSeen,x
dex
bpl :-
rts

_automap_setEdges:
ldx #31
lda #$FF
:
sta edgesSeen,x
dex
bpl :-
rts

; zero page usage!
x0 = $50
y0 = $52
x1 = $54
y1 = $56
dx = $58
dy = $59
sx = $5a
sy = $5c
ndxp1 = $5e
err = $5f
addr = $60
chk = $62
drewAPixel = $63

.macro set16 q, p
   lda p
   sta q
   lda p+1
   sta q+1
.endmacro

.macro asr_a
   cmp #$80
   ror
.endmacro

.macro asr_a_xtimes
: cmp #$80
  ror
  dex
  bne :-
.endmacro

.macro neg_a
   eor #$ff
   clc
   adc #1
.endmacro

.macro add16 p, q
   lda p
   clc
   adc q
   sta p
   lda p+1
   adc q+1
   sta p+1
.endmacro

.macro sub16 p, q
   lda p
   sec
   sbc q
   sta p
   lda p+1
   sbc q+1
   sta p+1
.endmacro

.macro add16_imm8 p, q
  lda p
  clc
  adc #q
  sta p
  bcc :+
  inc p+1
:
.endmacro

.macro mul16_by2 p
  asl p
  rol p+1
.endmacro

.macro sign_extend p
  ldy #0
  lda p
  bpl :+
  dey
  :
  sty p+1
.endmacro

.macro load_char_from_stack p
  lda (sp),y
  sta p
  iny
.endmacro

.macro load_int_from_stack p
  lda (sp),y
  sta p
  iny
  lda (sp),y
  sta p+1
  iny
.endmacro

drawLine:

;   signed char err, e2;
;   char drewAPixel = 0;
;   int maxx = x1;
;   int maxy = y1;
;   int minx = x0;
;   int miny = y0;
;   signed char sx = 1;
;   signed char sy = 1;
;   signed char dx = x1-x0;
;   signed char dy = y1-y0;
;   if (dx < 0)
;   {
;     sx = -1;
;     dx = -dx;
;     maxx = x0;
;     minx = x1;
;   }
;   if (dy < 0)
;   {
;     sy = -1;
;     dy = -dy;
;     maxy = y0;
;     miny = y1;
;   }
;   if (maxx >= 0 && minx < 64 && maxy >= 0 && miny < 64)
;   {

lda #0
sta drewAPixel
sta sx+1
sta sy+1
lda x1
sec
sbc x0
sta dx
bpl @dxpositive
; dxnegative
neg_a
sta dx
lda #-1
sta sx
sta sx+1
set16 maxx, x0
set16 minx, x1
jmp @checkxbbox
@dxpositive:
lda #1
sta sx
set16 maxx, x1
set16 minx, x0

@checkxbbox:
lda maxx+1
bmi @end1
lda minx+1
bmi @checky
cmp #1
bpl @end1
lda minx
and #$c0
beq @checky
@end1:
rts

@checky:

lda y1
sec
sbc y0
sta dy
bpl @dypositive
; dynegative
neg_a
sta dy
lda #-1
sta sy
sta sy+1
set16 maxy, y0
set16 miny, y1
jmp @checkybbox
@dypositive:
lda #1
sta sy
set16 maxy, y1
set16 miny, y0

@checkybbox:
lda maxy+1
bmi @end2
lda miny+1
bmi @checkspassed
cmp #1
bpl @end2
lda miny
and #$c0
beq @checkspassed
@end2:
rts

@checkspassed:

lda dx
neg_a
clc
adc #1
sta ndxp1

; err = (dx > dy) ? dx>>1 : -(dy>>1);

lda dx
cmp dy
bmi @dygreater
asr_a
sta err
jmp loop
@dygreater:
lda dy
neg_a
asr_a
sta err

loop:

; signed char chk = (x0 & 0xc0) | (y0 & 0xc0);
; if (!chk)

lda x0
and #$c0
sta chk
lda y0
and #$c0
ora chk
beq @drawAPixel

lda drewAPixel
beq @checkForEnd
rts

; int a = buffer + ((x0&0xf8)<<3) + y0;
; POKE(a, PEEK(a) | shift[x0&7]);
; drewAPixel = 1;

@drawAPixel:
lda x0
tax
and #$38
asl
asl
asl
sta addr
lda #>buffer
adc #0
sta addr+1
txa
and #$7
tax
ldy y0
lda (addr),y
ora shift,x
sta (addr),y
lda #1
sta drewAPixel

@checkForEnd:

; if (x0 == x1 && y0 == y1) return;

lda x0
cmp x1
bne @updateXandY
lda y0
cmp y1
bne @updateXandY
lda x0+1
cmp x1+1
bne @updateXandY
lda y0+1
cmp y1+1
bne @updateXandY
rts

@updateXandY:

; e2 = err;
; if (e2 > -dx)
; {
;   err -= dy;
;   x0 += sx;
; }
; if (e2 < dy)
; {
;   err += dx;
;   y0 += sy;
; }

lda err
tax
cmp ndxp1
bmi noxinc
sec
sbc dy
sta err
add16 x0, sx

noxinc:

txa
cmp dy
bpl loop
lda err
clc
adc dx
sta err
add16 y0, sy
jmp loop


numSectors:
.byte 0
sectorIndex:
.byte 0
numVerts:
.byte 0
edgeIndex:
.byte 0
globalEdgeIndex:
.byte 0
secondVertexIndex:
.byte 0
offsetX:
.word 0
offsetY:
.word 0

_automap_sawEdge:
tay
and #7
tax
tya
lsr
lsr
lsr
tay
lda edgesSeen,y
ora shift,x
sta edgesSeen,y
rts

offsetAndScaleVert0:

sign_extend x0
sign_extend y0
add16 x0, offsetX
lda offsetY
sec
sbc y0
sta y0
lda offsetY+1
sbc y0+1
sta y0+1

lda zoom
cmp #2
bne :+
mul16_by2 x0
mul16_by2 y0
:
add16_imm8 x0,32
add16_imm8 y0,32

rts

offsetAndScaleVert1:

sign_extend x1
sign_extend y1
add16 x1, offsetX
lda offsetY
sec
sbc y1
sta y1
lda offsetY+1
sbc y1+1
sta y1+1

lda zoom
cmp #2
bne :+
mul16_by2 x1
mul16_by2 y1
:
add16_imm8 x1,32
add16_imm8 y1,32

rts

offsetAndScaleVerts:

jsr offsetAndScaleVert0
jmp offsetAndScaleVert1

automap_drawSector:

sta sectorIndex
jsr _getNumVerts
sta numVerts
lda #0
sta edgeIndex

@loop:
lda edgeIndex
ldx sectorIndex
jsr getEdgeIndex
ldx sectorIndex
jsr getOtherSector
cmp #0
bmi :+
jmp @next
:
lda edgeIndex
ldx sectorIndex
jsr getEdgeIndex
tay
and #7
tax
tya
lsr
lsr
lsr
tay
lda edgesSeen,y
and shift,x
bne :+
jmp @next
:
ldx edgeIndex
inx
cpx numVerts
bne @skip
ldx #0
@skip:
stx secondVertexIndex

lda edgeIndex
ldx sectorIndex
jsr getSectorVertexXY
stx x0
sty y0

lda secondVertexIndex
ldx sectorIndex
jsr getSectorVertexXY
stx x1
sty y1

jsr offsetAndScaleVerts

jsr drawLine

@next:
inc edgeIndex
ldx edgeIndex
cpx numVerts
beq :+
jmp @loop
:
rts

drawPlayer:

lda player_a
jsr _getSinOf
ldx #5
asr_a_xtimes
sta sina
sign_extend sina

lda player_a
jsr _getCosOf
ldx #5
asr_a_xtimes
sta cosa
sign_extend cosa

lda player_x
sta x0
sta x1
lda player_y
sta y0
sta y1
jsr offsetAndScaleVerts

add16 x1, sina
sub16 y1, cosa

sub16 x0, sina
add16 y0, cosa

jsr drawLine

;lda sina
;asr_a
;sta sina
sign_extend sina
;lda cosa
;asr_a
;sta cosa
sign_extend cosa

lda player_x
sta x0
lda player_y
sta y0
jsr offsetAndScaleVert0

sub16 x0, cosa
sub16 y0, sina

jsr drawLine

lda player_x
sta x0
lda player_y
sta y0
jsr offsetAndScaleVert0

add16 x0, cosa
add16 y0, sina

jsr drawLine

rts

_automap_draw:

sta player_a

ldy #0
load_char_from_stack player_y
load_char_from_stack player_x
load_char_from_stack zoom
load_int_from_stack offsetY
load_int_from_stack offsetX

sign_extend player_x
sign_extend player_y

sub16 offsetX, player_x
add16 offsetY, player_y

; draw player
; a=0 is pointing right up

jsr drawPlayer

jsr _getNumSectors
sta numSectors
lda #0
sta sectorIndex

@loop:
jsr automap_drawSector
inc sectorIndex
lda sectorIndex
cmp numSectors
bne @loop

@end:
ldy #7
jmp addysp


_drawLine:

sta y1
stx y1+1

ldy #0
lda (sp),y
sta x1
iny
lda (sp),y
sta x1+1

iny
lda (sp),y
sta y0
iny
lda (sp),y
sta y0+1

iny
lda (sp),y
sta x0
iny
lda (sp),y
sta x0+1

jsr drawLine

ldy #6
jmp addysp


