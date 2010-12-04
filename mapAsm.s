.setcpu		"6502"
.autoimport	on
.importzp sp
.export _transformSectorToScreenSpace
.export _findFirstEdgeInSpan
.export _getTransformedX
.export _getTransformedY
.export _getScreenX

; sector/vertex functions
.export _getNumVerts
.export _getSectorVertexX
.export _getSectorVertexY

; sector/edge functions
.export _getEdgeTexture
.export _getEdgeLen
.export _getOtherSector
.export _getNextEdge

; object functions
.export _getNumObjects
.export _getObjectSector
.export _getObjectX
.export _getObjectY
.export _getObjectType
.export secNumVerts
.export vertX
.export edgeTex
.export secVerts
.export numVerts


.if 0
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

.endif

.segment "MAPDATA"
; sector info
secNumVerts:
.byte 4, 4, 7, 4, 8, 4, 5, 4, 8, 4, 4, 5, 4, 6, 4, 7
.byte 5, 4, 3, 5, 6, 4, 6, 4, 4, 7, 6, 8, 4, 4, 6
.res 1, 0

; object data
objXhi:
.byte -34, 35, -38, -38, -22, -22, -92, -92, -28, -120, -100, 103, 80, -113, 12, 32
.byte 46, 56, 47, 93, 80, 25, 77, 62, 98, 83, 77, 117, 118, 84, 60
.res 1, 0

objXlo:
.res 32, 0

objYhi:
.byte -24, 50, 0, 14, 14, 0, 0, 14, 22, 6, 14, -17, -85, 7, 39, 55
.byte 56, -29, -29, -36, -68, 6, 22, -60, -60, -10, 0, -15, -1, -92, -68
.res 1, 0

objYlo:
.res 32, 0

objAng:
.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.byte 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.res 1, 0

objType:
.byte 0, 11, 12, 12, 12, 12, 12, 12, 3, 1, 11, 7, 7, 11, 11, 11
.byte 11, 11, 11, 11, 11, 1, 11, 3, 3, 11, 11, 10, 10, 6, 7
.res 1, 0

objSec:
.byte 0, 9, 2, 2, 2, 2, 4, 4, 2, 5, 4, 20, 30, 5, 8, 9
.byte 11, 23, 23, 21, 27, 25, 14, 28, 29, 15, 15, 19, 19, 30, 28
.res 1, 0

; vertex data
vertX:
.byte -40, -28, -28, -40, -56, -12, -56, -56, -56, -24, -12, -74, -74, -78, -96, -110
.byte -110, -96, -78, -126, -126, -18, -2, 8, 8, -2, 8, 52, 40, 28, 28, 36
.byte 36, 28, 28, 52, 8, 40, 52, 60, 68, 74, 84, 66, 66, 76, 84, 98
.byte 66, 76, 84, 76, 88, 72, 88, 72, 66, 66, 56, 56, 104, 104, 94, 94
.byte 74, 84, 88, 88, 70, 70, 76, 76, 40, 30, 40, -6, -6, 46, 60, 60
.byte 98, 102, 104, 114, 124, 98, 88, 88, 98, 112, 112, 122, 114, 30
.res 34, 0

vertY:
.byte -20, -20, -26, -26, -12, -12, -2, 16, 26, 26, 26, 2, 12, -6, -6, 0
.byte 14, 20, 20, 0, 14, 50, 56, 56, 46, 46, 74, 74, 64, 64, 60, 60
.byte 46, 46, 42, 44, 32, 42, 32, 28, 28, 16, 16, 16, -12, -18, -18, -14
.byte 44, 36, -42, -42, -50, -50, -64, -64, -64, -50, -50, -78, -78, -50, -50, -64
.byte -78, -78, -84, -96, -96, -84, -24, -34, -24, -14, -14, -14, 26, 26, 12, -14
.byte 12, 6, -10, 26, 12, -28, -28, -40, -40, -34, -24, -28, 14, -34
.res 34, 0

; edge data
edgeTex:
.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.byte 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0
.byte 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0
.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0
.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.byte 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
.res 2, 0

edgeSec1:
.byte 0, -1, -1, -1, -1, 1, -1, -1, 2, -1, -1, 2, -1, -1, 3, -1
.byte -1, -1, -1, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7, -1, -1
.byte 6, -1, -1, 10, -1, 8, -1, -1, -1, -1, 8, -1, -1, -1, 8, -1
.byte -1, -1, 11, 11, -1, -1, -1, 14, -1, -1, -1, 15, -1, -1, -1, -1
.byte 13, -1, 22, -1, -1, -1, 26, -1, -1, -1, 27, -1, -1, -1, 27, -1
.byte -1, -1, -1, 27, -1, -1, -1, -1, -1, -1, -1, -1, -1, 24, -1, -1
.byte -1, -1, -1, -1, -1, 15, -1, 16, -1, 16, -1, -1, 19, -1, -1, -1
.byte -1, 20, -1, -1, -1, -1, -1, -1, 18, 17, 22, -1, -1, 23
.res 2, 0

edgeSec2:
.byte 1, -1, -1, -1, -1, 2, -1, -1, 3, -1, -1, 6, -1, -1, 4, -1
.byte -1, -1, -1, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, -1, -1
.byte 7, -1, -1, 11, -1, 10, -1, -1, -1, -1, 9, -1, -1, -1, 12, -1
.byte -1, -1, 13, 12, -1, -1, -1, 15, -1, -1, -1, 22, -1, -1, -1, -1
.byte 14, -1, 26, -1, -1, -1, 27, -1, -1, -1, 29, -1, -1, -1, 28, -1
.byte -1, -1, -1, 30, -1, -1, -1, -1, -1, -1, -1, -1, -1, 25, -1, -1
.byte -1, -1, -1, -1, -1, 16, -1, 17, -1, 20, -1, -1, 20, -1, -1, -1
.byte -1, 21, -1, -1, -1, -1, -1, -1, 19, 18, 23, -1, -1, 24
.res 2, 0

edgeLen:
.byte 12, 6, 12, 6, 18, 44, 18, 10, 18, 10, 32, 12, 38, 18, 10, 18
.byte 9, 18, 15, 14, 15, 18, 9, 16, 14, 16, 25, 17, 10, 10, 10, 22
.byte 10, 18, 44, 16, 12, 22, 4, 8, 14, 8, 14, 22, 30, 14, 22, 12
.byte 4, 44, 12, 16, 9, 8, 13, 10, 8, 28, 12, 8, 15, 14, 13, 22
.byte 11, 24, 8, 9, 9, 14, 16, 14, 10, 28, 17, 14, 10, 28, 17, 14
.byte 6, 6, 20, 10, 18, 7, 12, 18, 12, 7, 6, 8, 36, 10, 10, 36
.byte 40, 52, 20, 26, 20, 33, 15, 7, 16, 7, 21, 14, 10, 15, 10, 12
.byte 10, 12, 16, 14, 40, 12, 38, 17, 10, 12, 10, 20, 46, 14
.res 2, 0

; sector data
secVerts:
.byte 3, 0, 1, 2, 0, 0, 0, 0, 0, 4, 5, 1, 0, 0, 0, 0
.byte 4, 6, 7, 8, 9, 10, 5, 0, 11, 12, 7, 6, 0, 0, 0, 0
.byte 15, 16, 17, 18, 12, 11, 13, 14, 19, 20, 16, 15, 0, 0, 0, 0
.byte 9, 21, 22, 25, 10, 0, 0, 0, 25, 22, 23, 24, 0, 0, 0, 0
.byte 24, 23, 26, 29, 30, 33, 34, 36, 33, 30, 31, 32, 0, 0, 0, 0
.byte 29, 26, 27, 28, 0, 0, 0, 0, 37, 28, 27, 35, 38, 0, 0, 0
.byte 36, 34, 37, 38, 0, 0, 0, 0, 38, 35, 48, 49, 40, 39, 0, 0
.byte 40, 49, 42, 41, 0, 0, 0, 0, 44, 43, 41, 42, 47, 46, 45, 0
.byte 47, 42, 80, 81, 82, 0, 0, 0, 80, 83, 92, 81, 0, 0, 0, 0
.byte 92, 83, 84, 0, 0, 0, 0, 0, 90, 92, 84, 91, 89, 0, 0, 0
.byte 85, 47, 82, 90, 89, 88, 0, 0, 87, 86, 85, 88, 0, 0, 0, 0
.byte 71, 70, 45, 46, 50, 51, 0, 0, 93, 72, 70, 71, 0, 0, 0, 0
.byte 93, 73, 74, 72, 0, 0, 0, 0, 75, 76, 77, 78, 79, 74, 73, 0
.byte 55, 53, 51, 50, 52, 54, 0, 0, 59, 56, 55, 54, 63, 60, 65, 64
.byte 59, 58, 57, 56, 0, 0, 0, 0, 60, 63, 62, 61, 0, 0, 0, 0
.byte 68, 69, 64, 65, 66, 67, 0, 0
.res 8, 0

secEdges:
.byte 3, 0, 1, 2, 0, 0, 0, 0, 4, 5, 6, 0, 0, 0, 0, 0
.byte 7, 8, 9, 10, 11, 12, 5, 0, 14, 15, 8, 13, 0, 0, 0, 0
.byte 19, 20, 21, 22, 14, 16, 17, 18, 24, 25, 19, 23, 0, 0, 0, 0
.byte 26, 27, 32, 31, 11, 0, 0, 0, 32, 28, 29, 30, 0, 0, 0, 0
.byte 29, 33, 37, 38, 42, 48, 46, 45, 42, 39, 40, 41, 0, 0, 0, 0
.byte 37, 34, 35, 36, 0, 0, 0, 0, 43, 35, 44, 50, 51, 0, 0, 0
.byte 46, 47, 51, 49, 0, 0, 0, 0, 50, 61, 62, 64, 53, 52, 0, 0
.byte 64, 63, 55, 54, 0, 0, 0, 0, 57, 56, 55, 101, 60, 59, 58, 0
.byte 101, 102, 103, 104, 105, 0, 0, 0, 106, 121, 107, 103, 0, 0, 0, 0
.byte 121, 119, 120, 0, 0, 0, 0, 0, 118, 120, 116, 117, 108, 0, 0, 0
.byte 115, 105, 114, 108, 109, 113, 0, 0, 111, 112, 113, 110, 0, 0, 0, 0
.byte 122, 90, 59, 65, 66, 91, 0, 0, 125, 92, 122, 124, 0, 0, 0, 0
.byte 123, 93, 94, 125, 0, 0, 0, 0, 96, 97, 98, 99, 100, 93, 95, 0
.byte 71, 68, 66, 67, 69, 70, 0, 0, 78, 80, 70, 81, 74, 82, 83, 84
.byte 77, 76, 79, 78, 0, 0, 0, 0, 74, 75, 72, 73, 0, 0, 0, 0
.byte 88, 89, 83, 85, 86, 87, 0, 0
.res 8, 0

; summary data (4 bytes)
numVerts:
.byte 94
numEdges:
.byte 126
numSectors:
.byte 31
numObj:
.byte 31

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

edgeIndex = $80
sectorIndex = $81
numberOfVerts = $82

.proc _getNumVerts : near

; A - sectorIndex

tay
lda secNumVerts,y
rts

.endproc

.proc _getSectorVertexX : near

; params:
; A - vertexIndex
; TOS - sectorIndex

sta edgeIndex
ldy #0
lda (sp),y
asl
asl
asl
clc
adc edgeIndex
tay
lda secVerts,y
tay
lda vertX,y

ldy #1
jmp addysp

.endproc

.proc _getSectorVertexY : near

; params:
; A - vertexIndex
; TOS - sectorIndex

sta edgeIndex
ldy #0
lda (sp),y
asl
asl
asl
clc
adc edgeIndex
tay
lda secVerts,y
tay
lda vertY,y

ldy #1
jmp addysp

.endproc


.proc _getEdgeTexture : near

; params:
; A - edgeIndex
; TOS - sectorIndex

sta edgeIndex
ldy #0
lda (sp),y
asl
asl
asl
clc
adc edgeIndex
tay
lda secEdges,y
tay
lda edgeTex,y

ldy #1
jmp addysp

.endproc

.proc _getEdgeLen : near

; params:
; A - edgeIndex
; TOS - sectorIndex

sta edgeIndex
ldy #0
lda (sp),y
asl
asl
asl
clc
adc edgeIndex
tay
lda secEdges,y
tay
lda edgeLen,y

ldy #1
jmp addysp

.endproc

.proc _getOtherSector : near

; params:
; A - edgeIndex
; TOS - sectorIndex

sta edgeIndex
ldy #0
lda (sp),y
sta sectorIndex
asl
asl
asl
clc
adc edgeIndex
tay
lda secEdges,y
tay
lda edgeSec1,y
cmp #$ff
beq end
cmp sectorIndex
bne end
lda edgeSec2,y

end:
ldy #1
jmp addysp

.endproc

.proc _getNextEdge : near

; params:
; A - edgeIndex
; TOS - sectorIndex

tax
inx
ldy #0
lda (sp),y
tay
lda secNumVerts,y
sta numberOfVerts
txa
cmp numberOfVerts
bne done
lda #0

done:
ldy #1
jmp addysp

.endproc


.proc _getNumObjects : near

lda numObj
rts

.endproc

.proc _getObjectSector : near

tay
lda objSec,y
rts

.endproc

.proc _getObjectX : near

tay
lda objXhi,y
tax
lda objXlo,y
rts

.endproc

.proc _getObjectY : near

tay
lda objYhi,y
tax
lda objYlo,y
rts

.endproc

.proc _getObjectType : near

tay
lda objType,y
rts

.endproc