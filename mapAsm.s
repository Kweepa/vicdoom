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

.export _getNumEnemies
.export _getNumItems
.export _getNumSecrets
.export _getParTime

.export _resetSectorsVisited
.export _setSectorVisited
.export _getNumVisitedSecrets

.export _getMapName

.include "e1m1.s"

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

cmp #$ff
bne :+
rts
:

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

_getMapName:
lda #<mapName
ldx #>mapName
rts

_getNumEnemies:
lda numEnemies
ldx #0
rts

_getNumItems:
lda numItems
ldx #0
rts

_getNumSecrets:
lda numSecrets
ldx #0
rts

_getParTime:
lda parTime
ldx #0
rts

visitedSectors:
.res 32, 0

_resetSectorsVisited:
ldx #31
lda #0
:
sta visitedSectors,x
dex
bpl :-
rts

_setSectorVisited:
tax
lda #1
sta visitedSectors,x
rts

numVisitedSecrets:
.byte 0

_getNumVisitedSecrets:
lda #0
sta numVisitedSecrets
ldx numSecrets
dex
secretLoop:
lda secretSectors,x
tay
lda visitedSectors,y
beq :+
inc numVisitedSecrets
:
dex
bpl secretLoop
lda numVisitedSecrets
ldx #0
rts