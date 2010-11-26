.setcpu		"6502"
.autoimport	on
.importzp sp
.export _transformSectorToScreenSpace
.export _findFirstEdgeInSpan
.export _getTransformedX
.export _getTransformedY
.export _getScreenX
.export secNumVerts
.export vertX
.export edgeTex
.export secVerts
.export numVerts


.segment "MAPDATA"

; sector info and objects (one page)

; sector info (32 bytes)
secNumVerts:
.byte 6, 4
.res 30, 0

; total of 32 objects (224 bytes)
objXhi:
.byte 0, 10, -5, -10, -10
.res 27, 0
objXlo:
.res 32, 0
objYhi:
.byte -2, 2, 15, 0, 1
.res 27, 0
objYlo:
.res 32, 0
objAng:
.res 32, 0
objType:
.byte 0, 1, 2, 3, 4
.res 27, 0
objSec:
.byte 0, 0, 1, 0, 0
.res 27, 0

; total of 128 vertices (one page)
vertX:
.byte -20, -20, 20, 20, -10, -10, 0, 0
.res 120, 0
vertY:
.byte -10, 10, 10, -10, 10, 20, 20, 10
.res 120, 0

; total of 128 edges (two pages)
edgeTex:
.byte 0, 1, -1, 1, 2, 0, 1, 2, 0, -1
.res 118, 0
edgeSec:
.byte -1, -1, 1, -1, -1, -1, -1, -1, -1, 0
.res 118, 0
edgeIndex:
.byte -1, -1, 1, -1, -1, -1, -1, -1, -1, 1
.res 118, 0
edgeLen:
.byte 20, 10, 10, 20, 20, 40, 10, 10, 10, 10
.res 118, 0

; total of 32 sectors (two pages)
secVerts:
.byte 0, 1, 4, 7, 2, 3, 0, 0, 4, 5, 6, 7, 0, 0, 0, 0
.res 240, 0
secEdges:
.byte 0, 1, 2, 3, 4, 5, 0, 0, 6, 7, 8, 9, 0, 0, 0, 0
.res 240, 0

; summary data (4 bytes)
numVerts:
.byte 8
numEdges:
.byte 10
numSectors:
.byte 2
numObj:
.byte 3

; r/w parts (48 bytes)
xfvertXhi:
.res 8, 0
xfvertXlo:
.res 8, 0
xfvertYhi:
.res 8, 0
xfvertYlo:
.res 8, 0
xfvertScreenXhi:
.res 8, 0
xfvertScreenXlo:
.res 8, 0

padFromHere:
; final two pages for future expansion
.res 512 - 52, 0

vertexCount = $60
vertexCounter = $61
vertexCounterPP = $62
x_L = $63
x_R = $65
; see logMathAsm (keep in sync)
xToTransform = $68
yToTransform = $6A

.segment "CODE"

.proc _getScreenX: near
tay
lda xfvertScreenXhi,y
tax
lda xfvertScreenXlo,y
rts
.endproc

.proc _getTransformedX: near
tay
lda xfvertXhi,y
tax
lda xfvertXlo,y
rts
.endproc

.proc _getTransformedY: near
tay
lda xfvertYhi,y
tax
lda xfvertYlo,y
rts
.endproc

.proc _transformSectorToScreenSpace: near

; sectorIndex in A
tay
asl
asl
asl
sta modify+1 ; point to the correct sector verts - this only works because secVerts is page-aligned
lda secNumVerts,y
sta vertexCount
tax
dex ; vertex counter (7..0) in x

anotherVertToTransform:

; set the lo bytes
lda #0
sta xToTransform
sta yToTransform

stx vertexCounter
modify:
lda secVerts, x
tay

lda vertX, y
sta xToTransform+1
lda vertY, y
sta yToTransform+1
jsr _transformxy
ldy vertexCounter
sta xfvertXlo, y
txa
sta xfvertXhi, y
jsr _transformy
sta xfvertYlo, y
txa
sta xfvertYhi, y
bmi Yneg
bne Ypos
lda xfvertYlo, y
bne Ypos
Yneg:
lda xfvertXhi, y
bpl Xpos
lda #$FC
bmi over
Xpos:
lda #$04
over:
sta xfvertScreenXhi, y
lda #0
sta xfvertScreenXlo, y
beq continue
Ypos:
lda xfvertXlo, y
ldx xfvertXhi, y
jsr pushax
ldy vertexCounter
lda xfvertYlo, y
ldx xfvertYhi, y
jsr _leftShift4ThenDiv
ldy vertexCounter
sta xfvertScreenXlo, y
txa
sta xfvertScreenXhi, y

continue:
ldx vertexCounter
dex
bpl anotherVertToTransform
rts

.endproc


.proc _findFirstEdgeInSpan: near

; x_R in A
; x_L on stack

; save and sign extend L & R
ldx #0
sta x_R
cmp #0
bpl signExtendR
dex
signExtendR:
stx x_R+1

ldx #0
ldy #0
lda (sp), y
sta x_L
bpl signExtendL
dex
signExtendL:
stx x_L+1

ldx #0
stx vertexCounter

keepLooking:

inx
cpx vertexCount
bne dontReset
ldx #0
dontReset:
stx vertexCounterPP

; if (sx2 > x_L)
; interesting that having x_L sign extended makes this quicker and clearer!
sec
lda xfvertScreenXlo, x
sbc x_L
lda xfvertScreenXhi, x
sbc x_L+1
bmi notThisVert

; if (vy2 >= 1 || vy1 >= 1)
sec
lda #1
sbc xfvertYlo, x
lda #0
sbc xfvertYhi, x

bmi keepConsideringThisVert1

ldx vertexCounter
sec
lda #1
sbc xfvertYlo, x
lda #0
sbc xfvertYhi, x
bpl notThisVert

keepConsideringThisVert1:

; if ((sx1 <= leftx || (sx1 == 1000 && sx2 < rightx)) // left is off screen

ldx vertexCounter
sec
lda x_L
sbc xfvertScreenXlo, x
lda x_L+1
sbc xfvertScreenXhi, x
bpl thisVert

lda xfvertScreenXlo, x
bne notThisVert
lda xfvertScreenXhi, x
cmp #$04
bne notThisVert

ldx vertexCounterPP
sec
lda xfvertScreenXlo, x
sbc x_R
lda xfvertScreenXhi, x
sbc x_R+1
bpl notThisVert

thisVert:

lda vertexCounter
ldy #1
jmp addysp

notThisVert:
inc vertexCounter
ldx vertexCounter
cpx vertexCount
bne keepLooking

lda #255
ldy #1
jmp addysp

.endproc