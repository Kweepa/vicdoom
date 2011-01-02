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
.byte 62
numEdges:
.byte 86
numSectors:
.byte 22
numObj:
.byte 4
playerSpawnX:
.byte 0
playerSpawnY:
.byte -14
playerSpawnAngle:
.byte 0
playerSpawnSector:
.byte 5
pad:
.res 88, 0
; sector info
secNumVerts:
.byte 4, 7, 7, 4, 8, 4, 5, 4, 4, 4, 8, 4, 5, 4, 5, 4
.byte 5, 5, 5, 4, 6, 4
.res 10, 0

; object data
objXhi:
.byte -13, 14, 14, 29
.res 28, 0

objYhi:
.byte 5, 5, -5, 0
.res 28, 0

objType:
.byte 13, 13, 13, 9
.res 28, 0

objSec:
.byte 1, 3, 6, 10
.res 28, 0

; vertex data
vertX:
.byte -20, -20, -20, -18, 0, 20, 20, 20, 20, 4, 4, -4, -4, -12, -16, -16
.byte -8, -2, -2, -8, 2, 2, 8, 8, 5, 34, 35, 26, 26, 35, 34, 40
.byte 40, 44, 44, 40, 40, -26, -26, -22, -11, -11, -27, -27, -32, -32, -43, -43
.byte -24, -29, -32, -48, -48, -43, -47, -47, -43, -39, -34, -35, -21, -21
.res 66, 0

vertY:
.byte -12, -2, 4, 10, 12, 10, 6, -4, -10, -10, -16, -16, -10, -10, -6, -12
.byte 6, 6, -3, -3, 6, -3, -3, 3, 6, 12, 4, 4, -3, -3, -12, -20
.byte -3, -3, 4, 4, 20, 4, 12, 16, 16, 30, 30, 18, 12, 7, 7, 20
.byte -2, -2, 2, 2, 23, 26, 29, 34, 36, 36, 33, 18, 4, -2
.res 66, 0

; edge data
edgeTex:
.byte 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.byte 0, 0, 0, 4, 0, 0, 0, 3, 3, 0, 2, 0, 2, 2, 0, 2
.byte 3, 3, 0, 2, 2, 3, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1
.byte 1, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0
.byte 0, 0, 4, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0
.byte 0, 5, 8, 5, 0, 0
.res 42, 0

edgeSec1:
.byte -1, 1, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0
.byte -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
.byte -1, -1, -1, -1, 9, 9, 8, 7, 6, 4, 1, 1, 2, 2, 3, 4
.byte 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
.byte -1, -1, -1, -1, -1, -1, -1, 12, 12, 12, 13, 15, 16, 17, -1, -1
.byte -1, -1, 20, -1, -1, -1
.res 42, 0

edgeSec2:
.byte -1, 21, -1, -1, -1, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1
.byte -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
.byte -1, -1, -1, -1, 19, 10, 10, 8, 19, 5, 2, 4, 11, 3, 6, 6
.byte 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
.byte -1, -1, -1, -1, -1, -1, -1, 20, 15, 13, 14, 16, 17, 18, -1, -1
.byte -1, -1, 21, -1, -1, -1
.res 42, 0

edgeLen:
.byte 10, 6, 7, 19, 21, 4, 10, 16, 6, 8, 6, 8, 6, 6, 4, 6
.byte 9, 6, 6, 3, 6, 6, 9, 23, 16, 4, 7, 4, 17, 10, 9, 7
.byte 9, 9, 16, 5, 10, 5, 5, 10, 6, 8, 11, 9, 4, 16, 13, 14
.byte 4, 12, 16, 14, 11, 6, 8, 8, 5, 11, 13, 9, 16, 6, 4, 5
.byte 5, 5, 6, 21, 16, 5, 5, 7, 5, 6, 6, 8, 6, 11, 9, 17
.byte 23, 1, 6, 1, 5, 3
.res 42, 0

; sector data
secVerts:
.byte 0, 1, 14, 15, 0, 0, 0, 0, 14, 1, 2, 3, 16, 19, 13, 0
.byte 16, 3, 4, 5, 24, 20, 17, 0, 23, 24, 5, 6, 0, 0, 0, 0
.byte 13, 19, 18, 21, 22, 8, 9, 12, 11, 12, 9, 10, 0, 0, 0, 0
.byte 22, 23, 6, 7, 8, 0, 0, 0, 5, 36, 25, 6, 0, 0, 0, 0
.byte 26, 25, 36, 35, 30, 33, 34, 36, 30, 29, 32, 31, 0, 0, 0, 0
.byte 28, 27, 26, 35, 34, 33, 32, 29, 18, 17, 20, 21, 38, 0, 0, 0
.byte 50, 45, 44, 38, 37, 0, 0, 0, 44, 43, 39, 38, 40, 39, 0, 0
.byte 43, 42, 41, 40, 39, 0, 0, 0, 51, 46, 45, 50, 47, 46, 45, 0
.byte 51, 52, 53, 47, 46, 0, 0, 0, 47, 53, 57, 58, 59, 0, 0, 0
.byte 54, 55, 56, 57, 53, 0, 0, 0, 31, 8, 7, 30, 89, 0, 0, 0
.byte 49, 50, 37, 60, 61, 48, 0, 0, 61, 60, 2, 1, 0, 0, 0, 0
.res 80, 0

secEdges:
.byte 0, 15, 13, 14, 0, 0, 0, 0, 15, 1, 2, 42, 16, 43, 12, 0
.byte 42, 3, 4, 45, 19, 44, 17, 0, 35, 45, 5, 46, 0, 0, 0, 0
.byte 43, 18, 48, 21, 47, 7, 41, 11, 10, 41, 8, 9, 0, 0, 0, 0
.byte 20, 46, 6, 40, 47, 0, 0, 0, 23, 39, 34, 5, 0, 0, 0, 0
.byte 33, 39, 24, 38, 42, 48, 46, 45, 29, 37, 28, 36, 0, 0, 0, 0
.byte 31, 32, 38, 25, 26, 27, 37, 30, 78, 44, 22, 48, 51, 0, 0, 0
.byte 72, 56, 73, 54, 71, 0, 0, 0, 55, 74, 53, 73, 53, 52, 0, 0
.byte 49, 50, 51, 52, 74, 0, 0, 0, 75, 57, 72, 68, 60, 59, 58, 0
.byte 67, 66, 76, 58, 75, 0, 0, 0, 76, 77, 61, 60, 59, 0, 0, 0
.byte 64, 63, 62, 77, 65, 0, 0, 0, 80, 40, 79, 36, 108, 0, 0, 0
.byte 69, 71, 84, 82, 85, 70, 0, 0, 82, 81, 1, 83, 0, 0, 0, 0
.res 80, 0


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

