.setcpu	"6502"
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
.export _getNumSectors
.export _getEdgeIndex
.export _getEdgeTexture
.export _getEdgeLen
.export _getOtherSector
.export _getNextEdge
.export _getGlobalEdgeTexture

.export getOtherSector
.export getEdgeIndex
.export getSectorVertexXY

; object functions
.export _getNumObjects
.export _getObjectSector
.export _getObjectX
.export _getObjectY
.export _getObjectType
.export vertX
.export edgeTex
.export secVerts
.export numVerts

.export _setObjectX
.export _setObjectY
.export _setObjectSector
.export _setObjectType

.export _getPlayerSpawnX
.export _getPlayerSpawnY
.export _getPlayerSpawnAngle
.export _getPlayerSpawnSector

.export _addObjectsToSectors
.export _addObjectToSector
.export _removeObjectFromSector
.export _getFirstObjectInSector
.export _getNextObjectInSector

.segment "MAPDATA"
; summary data (7 bytes)
numVerts:
.byte 91
numEdges:
.byte 126
numSectors:
.byte 32
numObj:
.byte 30
playerSpawnX:
.byte -17
playerSpawnY:
.byte -11
playerSpawnAngle:
.byte 0
playerSpawnSector:
.byte 0
pad:
.res 88, 0
; sector info
secNumVerts:
.byte 4, 4, 7, 4, 8, 4, 8, 4, 4, 5, 4, 4, 5, 4, 3, 5
.byte 6, 4, 6, 4, 4, 7, 6, 6, 5, 8, 4, 4, 8, 4, 4, 4

; object data
objXhi:
.byte -19, -19, -11, -11, -46, -46, -14, -60, -50, 51, 40, -56, 6, 16, 23, 28
.byte 46, 40, 12, 38, 34, 47, 42, 38, 58, 59, 42, 31, 47, 36
.res 2, 0

objYhi:
.byte 0, 7, 7, 0, 0, 7, 11, 3, 7, -8, -48, 3, 19, 27, 28, -14
.byte -18, -34, 3, 11, -33, -33, -5, 0, -7, 0, -51, -29, 2, -4
.res 2, 0

objType:
.byte 13, 13, 13, 13, 13, 13, 12, 5, 0, 1, 1, 0, 0, 0, 0, 0
.byte 0, 0, 6, 0, 12, 12, 1, 0, 7, 7, 8, 1, 15, 15
.res 2, 0

objSec:
.byte 2, 2, 2, 2, 4, 4, 2, 5, 4, 16, 22, 5, 6, 7, 9, 19
.byte 17, 25, 21, 11, 26, 30, 23, 23, 15, 15, 22, 26, 12, 23
.res 2, 0

; vertex data
vertX:
.byte -20, -14, -14, -20, -28, -6, -28, -28, -28, -12, -6, -37, -37, -39, -48, -55
.byte -55, -48, -39, -63, -63, -9, 4, 4, 4, 26, 20, 14, 14, 18, 18, 14
.byte 14, 26, 4, 20, 26, 33, 37, 42, 33, 38, 42, 49, 37, 42, 38, 44
.byte 36, 44, 36, 34, 28, 28, 52, 52, 46, 37, 42, 44, 44, 35, 35, 38
.byte 38, 20, 15, 20, -3, -3, 23, 30, 30, 49, 51, 52, 57, 62, 49, 44
.byte 44, 49, 56, 56, 61, 57, 15, 34, 36, 44, 46
.res 37, 0

vertY:
.byte -10, -10, -13, -13, -6, -6, -1, 8, 13, 13, 13, 1, 6, -3, -3, 0
.byte 7, 10, 10, 0, 7, 25, 28, 23, 37, 37, 32, 32, 30, 30, 23, 23
.byte 21, 22, 16, 21, 16, 14, 8, 8, -6, -9, -9, -7, 20, -21, -21, -25
.byte -25, -38, -38, -38, -25, -44, -44, -25, -38, -44, -44, -48, -53, -53, -48, -12
.byte -17, -12, -7, -7, -7, 13, 13, 6, -7, 6, 3, -5, 13, 6, -14, -14
.byte -20, -20, -17, -12, -14, 7, -17, -36, -36, -36, -36
.res 37, 0

; edge data
edgeTex:
.byte 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 0, 2
.byte 0, 2, 2, 0, 2, 0, 2, 3, 3, 0, 2, 2, 3, 0, 0, 1
.byte 0, 1, 0, 1, 1, 0, 1, 6, 0, 0, 0, 0, 0, 0, 2, 0
.byte 0, 6, 0, 0, 0, 0, 5, 0, 1, 1, 1, 0, 1, 0, 0, 0
.byte 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0
.byte 0, 1, 1, 1, 1, 1, 0, 0, 6, 1, 1, 0, 0, 0, 0, 0
.byte 0, 0, 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0
.res 2, 0

edgeSec1:
.byte 0, -1, -1, -1, -1, 1, -1, -1, 2, -1, -1, 2, -1, -1, 3, -1
.byte -1, -1, -1, 4, -1, -1, -1, -1, -1, -1, -1, 6, -1, -1, 8, -1
.byte 6, -1, -1, -1, -1, 6, -1, -1, -1, 6, -1, -1, -1, 9, 9, -1
.byte 11, -1, 18, -1, -1, 11, -1, 18, -1, -1, 25, -1, 28, -1, 27, -1
.byte -1, 22, -1, -1, -1, -1, -1, -1, -1, -1, -1, 20, -1, -1, -1, -1
.byte -1, -1, -1, 12, -1, 12, -1, 12, -1, -1, 15, -1, -1, -1, -1, 16
.byte -1, -1, -1, -1, -1, -1, 14, 13, 18, -1, -1, 19, -1, -1, -1, -1
.byte -1, -1, -1, -1, -1, 25, 26, 25, -1, 29, -1, -1, -1, -1
.res 2, 0

edgeSec2:
.byte 1, -1, -1, -1, -1, 2, -1, -1, 3, -1, -1, 24, -1, -1, 4, -1
.byte -1, -1, -1, 5, -1, -1, -1, -1, -1, -1, -1, 24, -1, -1, 9, -1
.byte 8, -1, -1, -1, -1, 7, -1, -1, -1, 10, -1, -1, -1, 31, 10, -1
.byte 23, -1, 23, -1, -1, 31, -1, 25, -1, -1, 28, -1, 29, -1, 28, -1
.byte -1, 28, -1, -1, -1, -1, -1, -1, -1, -1, -1, 21, -1, -1, -1, -1
.byte -1, -1, -1, 23, -1, 13, -1, 16, -1, -1, 16, -1, -1, -1, -1, 17
.byte -1, -1, -1, -1, -1, -1, 15, 14, 19, -1, -1, 20, -1, -1, -1, -1
.byte -1, -1, -1, -1, -1, 26, 27, 30, -1, 30, -1, -1, -1, -1
.res 2, 0

edgeLen:
.byte 6, 3, 6, 3, 9, 22, 9, 5, 9, 5, 16, 6, 19, 10, 5, 10
.byte 5, 9, 8, 7, 8, 9, 5, 8, 7, 8, 13, 5, 9, 22, 8, 6
.byte 12, 2, 4, 7, 4, 7, 11, 15, 7, 12, 6, 2, 22, 6, 8, 8
.byte 5, 6, 4, 8, 13, 8, 12, 4, 5, 5, 8, 19, 9, 19, 9, 2
.byte 10, 5, 9, 5, 5, 9, 5, 5, 3, 4, 18, 5, 5, 18, 20, 26
.byte 10, 13, 10, 17, 8, 4, 9, 4, 11, 8, 5, 8, 5, 6, 5, 6
.byte 9, 7, 21, 6, 20, 9, 6, 6, 5, 10, 23, 8, 2, 2, 2, 15
.byte 14, 15, 12, 8, 8, 11, 13, 11, 8, 13, 2, 2, 2, 2
.res 2, 0

; sector data
secVerts:
.byte 3, 0, 1, 2, 0, 0, 0, 0, 0, 4, 5, 1, 0, 0, 0, 0
.byte 4, 6, 7, 8, 9, 10, 5, 0, 11, 12, 7, 6, 0, 0, 0, 0
.byte 15, 16, 17, 18, 12, 11, 13, 14, 19, 20, 16, 15, 0, 0, 0, 0
.byte 23, 22, 24, 27, 28, 31, 32, 34, 31, 28, 29, 30, 30, 33, 34, 36
.byte 27, 24, 25, 26, 30, 33, 34, 36, 35, 26, 25, 33, 36, 0, 0, 0
.byte 34, 32, 35, 36, 38, 0, 0, 0, 37, 44, 39, 38, 37, 37, 0, 0
.byte 43, 39, 73, 74, 75, 39, 0, 0, 73, 76, 85, 74, 79, 39, 0, 0
.byte 85, 76, 77, 78, 81, 0, 0, 0, 83, 85, 77, 84, 82, 46, 45, 0
.byte 78, 43, 75, 83, 82, 81, 0, 0, 80, 79, 78, 81, 86, 85, 0, 0
.byte 64, 63, 41, 42, 45, 46, 0, 0, 86, 65, 63, 64, 47, 48, 0, 0
.byte 86, 66, 67, 65, 49, 50, 0, 0, 68, 69, 70, 71, 72, 67, 66, 0
.byte 61, 62, 57, 58, 59, 60, 58, 57, 40, 38, 39, 43, 42, 41, 62, 61
.byte 9, 21, 22, 23, 10, 41, 64, 63, 88, 48, 46, 45, 47, 89, 49, 50
.byte 87, 52, 48, 88, 10, 42, 65, 64, 53, 52, 87, 51, 10, 44, 65, 64
.byte 53, 51, 50, 49, 56, 54, 58, 57, 56, 90, 55, 54, 66, 67, 0, 0
.byte 89, 47, 55, 90, 66, 67, 0, 0, 36, 33, 44, 37, 0, 0, 0, 0

secEdges:
.byte 3, 0, 1, 2, 0, 0, 0, 0, 4, 5, 6, 0, 0, 0, 0, 0
.byte 7, 8, 9, 10, 11, 12, 5, 0, 14, 15, 8, 13, 0, 0, 0, 0
.byte 19, 20, 21, 22, 14, 16, 17, 18, 24, 25, 19, 23, 0, 0, 0, 0
.byte 27, 28, 32, 33, 37, 43, 41, 40, 37, 34, 35, 36, 42, 48, 46, 45
.byte 32, 29, 30, 31, 42, 48, 46, 45, 38, 30, 39, 45, 46, 0, 0, 0
.byte 41, 42, 46, 44, 51, 0, 0, 0, 53, 52, 48, 47, 48, 47, 0, 0
.byte 83, 84, 85, 86, 87, 52, 0, 0, 88, 103, 89, 85, 96, 52, 0, 0
.byte 103, 101, 102, 94, 101, 0, 0, 0, 100, 102, 98, 99, 90, 59, 58, 0
.byte 97, 87, 96, 90, 91, 95, 0, 0, 93, 94, 95, 92, 100, 104, 0, 0
.byte 104, 72, 50, 54, 55, 73, 0, 0, 107, 74, 104, 106, 59, 82, 0, 0
.byte 105, 75, 76, 107, 64, 87, 0, 0, 78, 79, 80, 81, 82, 75, 77, 0
.byte 70, 71, 65, 67, 68, 69, 66, 67, 111, 48, 83, 51, 50, 49, 74, 75
.byte 26, 112, 27, 113, 11, 49, 79, 80, 117, 57, 55, 56, 119, 124, 58, 110
.byte 118, 116, 117, 109, 11, 51, 81, 82, 61, 118, 108, 62, 11, 56, 82, 83
.byte 62, 63, 58, 125, 60, 64, 65, 66, 123, 121, 59, 60, 85, 86, 0, 0
.byte 119, 120, 121, 122, 86, 87, 0, 0, 45, 114, 53, 115, 0, 0, 0, 0


.segment "CODE"

objXlo:
.res 32, 0

objYlo:
.res 32, 0

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

vertexCount = $60
vertexCounter = $61
vertexCounterPP = $62
x_L = $63
x_R = $65
outsideSector = $67
; see logMathAsm (keep in sync)
xToTransform = $68
yToTransform = $6A

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
; we're outside the sector flag further down the stack

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

ldy #1
lda (sp),y
sta outsideSector

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

; the second part of the test should only be done if the camera is inside the sector
lda outsideSector
bne notThisVert

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
ldy #2
jmp addysp

notThisVert:
inc vertexCounter
ldx vertexCounter
cpx vertexCount
bne keepLooking

lda #255
ldy #2
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


getSectorVertexXY:

; A - vertexIndex
; X - sectorIndex

sta edgeIndex
txa
asl
asl
asl
clc
adc edgeIndex
tay
lda secVerts,y
tay
lda vertX,y
tax
lda vertY,y
tay

rts

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



getEdgeIndex:

; A - edge index
; X = sector index

sta edgeIndex
txa
asl
asl
asl
clc
adc edgeIndex
tay
lda secEdges,y
rts

.proc _getEdgeIndex : near

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


getOtherSector:

; params:
; A - edgeIndex
; X - sectorIndex

sta edgeIndex
txa
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
beq @end
cmp sectorIndex
bne @end
lda edgeSec2,y

@end:
rts

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

objectIndex:
.byte 0

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

_setObjectX:
pha
ldy #0
lda (sp),y
tay
pla
sta objXlo,y
txa
sta objXhi,y
ldy #1
jmp addysp

_setObjectY:
pha
ldy #0
lda (sp),y
tay
pla
sta objYlo,y
txa
sta objYhi,y
ldy #1
jmp addysp

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

_setObjectType:

; A - type
; TOS - object

tax
ldy #0
lda (sp),y
tay
txa
sta objType,y

ldy #1
jmp addysp

_getGlobalEdgeTexture:
tay
lda edgeTex,y
rts

_getNumSectors:
lda numSectors
ldx #0
rts

_getPlayerSpawnX:
ldx playerSpawnX
lda #0
rts

_getPlayerSpawnY:
ldx playerSpawnY
lda #0
rts

_getPlayerSpawnAngle:
lda playerSpawnAngle
rts

_getPlayerSpawnSector:
lda playerSpawnSector
rts

sectorFirstObj:
; one for each sector
.res 32, $ff

sectorNextObj:
; one for each object
.res 32, $ff

sectorPrevObj:
; one for each object
.res 32, $ff

addObjectToSector:
; x contains object index
; a contains sector index

; o->next = first->next
; o->prev = 0xff
; first->next->prev = o
; first = o

tay ; AY = sec, X = o
lda sectorFirstObj,y ; A = next, Y = sec, X = o

sta sectorNextObj,x ; o->next = next
lda #$ff ; A = ff, Y = sec, X = o
sta sectorPrevObj,x ; o->prev = ff
stx objectIndex ; A = ff, Y = sec, X = o, tmp = o
lda sectorFirstObj,y ; A = next, Y = sec, X = tmp = o
tax ; A = next, Y = sec, X = next, tmp = o
bmi @skip
lda objectIndex ; A = o, Y = sec, X = next, tmp = o
sta sectorPrevObj,x ; next->prev = o
@skip:
lda objectIndex ; A = o, Y = sec, X = next, tmp = o
sta sectorFirstObj,y ; first = o
tax ; A = o, X = o
rts

_addObjectsToSectors:

; clear first objects
ldx #31
lda #$ff
@clearLoop:
sta sectorFirstObj,x
dex
bpl @clearLoop

; add obj
ldx numObj
dex
@loop:
lda objSec,x
jsr addObjectToSector
dex
bpl @loop
rts

_getFirstObjectInSector:
tay
lda sectorFirstObj,y
rts

_getNextObjectInSector:
tay
lda sectorNextObj,y
rts

_removeObjectFromSector:

; A contains object index
;
; if (first == o) first = o->next
; o->next->prev = o->prev
; o->prev->next = o->next
; note that after the operation, the prev and next pointers are still valid for the sector

tay
lda objSec,y
tax
tya
cmp sectorFirstObj,x
bne @notfirst
lda sectorNextObj,y
sta sectorFirstObj,x

@notfirst:
lda sectorPrevObj,y
bmi @skip
tax
lda sectorNextObj,y
sta sectorNextObj,x
@skip:
lda sectorNextObj,y
bmi @skip2
tax
lda sectorPrevObj,y
sta sectorPrevObj,x
@skip2:
rts

_addObjectToSector:

; A contains object index
; TOS contains sector index

tax
ldy #0
lda (sp),y
jsr addObjectToSector
ldy #1
jmp addysp

_setObjectSector:

; TOS - object
; A - sector

sta sectorIndex
ldy #0
lda (sp),y
sta objectIndex
jsr _removeObjectFromSector

lda sectorIndex
ldx objectIndex
jsr addObjectToSector
; x is unchanged

lda sectorIndex
sta objSec,x

ldy #1
jmp addysp

