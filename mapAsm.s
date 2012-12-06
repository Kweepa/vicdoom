.setcpu	"6502"
.autoimport	on
.importzp sp
.export _preTransformSectors
.export _transformSectorToScreenSpace
.export _getTransformedX
.export _getTransformedY
.export _getScreenX

; sector/vertex functions
.export _getNumVerts
.export _getVertexX
.export _getVertexY

; sector/edge functions
.export _getNumSectors
.export _getVertexIndex
.export _getEdgeIndex
.export _getEdgeLen
.export _getOtherSector
.export getOtherSector
.export _getNextEdge
.export _getEdgeTexture
.export _setEdgeTexture

.export _resetDoorClosedAmounts
.export _isEdgeDoor
.export _isDoorClosed
.export _basicOpenDoor
.export _basicCloseDoor

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
.res 48, 0

objYlo:
.res 48, 0

.segment "LOWCODE"

; r/w parts (48 bytes)
xfvertXhi:
.res 8, 0
xfvertXlo:
.res 8, 0
xfvertYhi:
.res 8, 0
xfvertYlo:
.res 8, 0
xfvertScreenX:
.res 8, 0

NUMSEC = 64

vertexCount = $60
vertexCounter = $61
vertexCounterPP = $62
x_L = $63
x_R = $65
outsideSector = $67
; see logMathAsm (keep in sync)
xToTransform = $68
yToTransform = $6A
cosa = $51
sina = $52
cameraX = $57
cameraY = $59
PRODUCT = $5e

.proc _getScreenX: near
tay
lda xfvertScreenX,y
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

.export clampIntToChar
clampIntToChar:

; x high, a low
tay
txa
bpl sxpos
cmp #$ff
beq :+
lda #$c0
rts
:
tya
cmp #$c0
bcs clipdone
lda #$c0
rts

sxpos:
cmp #0
beq :+
lda #$3f
rts
:
tya
cmp #$3f
bcc clipdone
lda #$3f

clipdone:
rts


edgeIndex = $80
sectorIndex = $81
numberOfVerts = $82

.proc _getNumVerts : near

; A - sectorIndex

tay
lda secNumVerts,y
rts

.endproc


.proc getSectorVertexXY : near

; A - vertexIndex
; X - sectorIndex

sta edgeIndex

txa
asl
asl
asl
sta modify+1 ; point to the correct sector verts - requires page alignment!
lda #0
adc #>secVerts
sta modify+2

ldy edgeIndex
modify:
lda secVerts, y
tay
lda vertX,y
tax
lda vertY,y
tay

rts

.endproc

.proc _getVertexIndex : near

; params:
; A - vertexIndex
; TOS - sectorIndex

sta edgeIndex
ldy #0
lda (sp),y

asl
asl
asl
sta modify+1
lda #>secVerts
adc #0
sta modify+2

ldy edgeIndex
modify:
lda secVerts,y

ldy #1
jmp addysp

.endproc

.proc _getVertexX : near

; params:
; A - global vertexIndex

tay
lda vertX,y
rts

.endproc

.proc _getVertexY : near

; params:
; A - global vertexIndex

tay
lda vertY,y
rts

.endproc



.proc getEdgeIndex : near

; A - edge index
; X = sector index

sta edgeIndex
txa

asl
asl
asl
sta modify+1
lda #0
adc #>secEdges
sta modify+2

ldy edgeIndex
modify:
lda secEdges,y
rts

.endproc

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
sta modify+1
lda #0
adc #>secEdges
sta modify+2

ldy edgeIndex
modify:
lda secEdges,y

ldy #1
jmp addysp

.endproc

.proc _getEdgeTexture : near

; params:
; A - global edgeIndex

tay
lda edgeTex,y
rts

.endproc

.proc _setEdgeTexture : near

; params:
; TOS - global edgeIndex
; A - texture

tax

ldy #0
lda (sp),y
tay

txa
sta edgeTex,y

ldy #1
jmp addysp

.endproc

.proc _getEdgeLen : near

; params:
; A - global edgeIndex

tay
lda edgeLen,y
rts

.endproc

.proc _getOtherSector : near

; params:
; A - sector index
; TOS - global edgeIndex

sta sectorIndex
ldy #0
lda (sp),y
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

.proc getOtherSector : near

; params:
; A - global edgeIndex
; X - sectorIndex

stx sectorIndex
tax

lda edgeSec1,x
cmp #$ff
beq end
cmp sectorIndex
bne end
lda edgeSec2,x

end:
rts

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


.segment "CODE"

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
.res NUMSEC, $ff

sectorNextObj:
; one for each object
.res NUMSEC, $ff

sectorPrevObj:
; one for each object
.res NUMSEC, $ff

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
ldx #63
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
.res NUMSEC, 0

_resetSectorsVisited:
ldx numSectors
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

_resetDoorClosedAmounts:

ldy #0
resetDoorsLoop:
ldx #1
lda edgeTex,y
and #$C0 ; EDGE_TYPE_MASK
cmp #$40 ; EDGE_TYPE_DOOR
beq :+
dex
:
txa
sta $9600,y
iny
cpy #200
bne resetDoorsLoop
rts

_isEdgeDoor:
tay
ldx #0
lda edgeTex,y
and #$C0
cmp #$40
bne :+
inx
:
txa
rts

.import _fastMulTest

_isDoorClosed:
tay
lda $9600,y
and #$0f
rts

_basicOpenDoor:
tay
lda #0
sta $9600,y
rts

_basicCloseDoor:
tay
lda #1
sta $9600,y
rts


.if 1
; ----------------------------
; transform sector
; ----------------------------

; x = vx - px
; y = vy - py

; x = x*cosa - y*sina
; y = y*cosa + x*sina
; (4 16.8 multiplies)

; x = (vx-px)*cosa - (vy-py)*sina
;   = vx*cosa - vy*sina - px*cosa + py*sina
; y = (vy-py)*cosa + (vx-px)*sina
;   = vy*cosa + vx*sina - py*cosa - px*sina
; (4 8.8 multiplies per vert + 4 16.8 multiplies per screen!)
; requires a little more summing
; ah, but we know vx and vy are always positive
; balls, we do not know that at all

; pre-transform

pxcosa:
.word 0
pysina:
.word 0
pycosa:
.word 0
pxsina:
.word 0
pysina_minus_pxcosa:
.word 0
pycosa_plus_pxsina:
.word 0

.proc _preTransformSectors: near

lda cosa
jsr _fastMultiplySetup16x8
lda cameraX
ldx cameraX+1
jsr _fastMultiply16x8
sta pxcosa
stx pxcosa+1
lda cameraY
ldx cameraY+1
jsr _fastMultiply16x8
sta pycosa
stx pycosa+1

lda sina
jsr _fastMultiplySetup16x8
lda cameraX
ldx cameraX+1
jsr _fastMultiply16x8
sta pxsina
stx pxsina+1
lda cameraY
ldx cameraY+1
jsr _fastMultiply16x8
sta pysina
stx pysina+1

sec
lda pysina
sbc pxcosa
sta pysina_minus_pxcosa
lda pysina+1
sbc pxcosa+1
sta pysina_minus_pxcosa+1

clc
lda pycosa
adc pxsina
sta pycosa_plus_pxsina
lda pycosa+1
adc pxsina+1
sta pycosa_plus_pxsina+1

rts

.endproc ; _preTransformSectors

.proc _transformSectorToScreenSpace: near

; A is the sector index

tax
lda secNumVerts,x
tay
dey
sty vertexCount

; loop and transform

; see getVertexIndex
txa
asl
asl
asl
sta modify1+1
sta modify2+1
lda #>secVerts
adc #0
sta modify1+2
sta modify2+2

lda cosa
jsr _fastMultiplySetup8x8

ldy vertexCount

loop1:

modify1:
  lda secVerts,y
  tax
  stx modify1a+1
  lda vertX,x
  jsr _fastMultiply8x8
  sta xfvertXlo,y
  txa
  sta xfvertXhi,y

modify1a:
  ldx #0 ; global vertex index
  lda vertY,x
  jsr _fastMultiply8x8
  sta xfvertYlo,y
  txa
  sta xfvertYhi,y

  dey
  bpl loop1

lda sina
jsr _fastMultiplySetup8x8

ldy vertexCount

loop2:

modify2:
  ldx secVerts,y
  stx modify2a+1
  lda vertX,x
  jsr _fastMultiply8x8
  ; y = vy*cosa + vx*sina - py*cosa - px*sina
  clc
  lda xfvertYlo,y
  adc PRODUCT
  sta PRODUCT
  lda xfvertYhi,y
  adc PRODUCT+1
  sta PRODUCT+1
  sec
  lda PRODUCT
  sbc pycosa_plus_pxsina
  sta PRODUCT
  lda PRODUCT+1
  sbc pycosa_plus_pxsina+1

  asl PRODUCT
  rol
  sta xfvertYhi,y
  lda PRODUCT
  sta xfvertYlo,y

modify2a:
  ldx #0 ; global vertex index
  lda vertY,x
  jsr _fastMultiply8x8
  ; x = vx*cosa - vy*sina - px*cosa + py*sina
  sec
  lda xfvertXlo,y
  sbc PRODUCT
  sta PRODUCT
  lda xfvertXhi,y
  sbc PRODUCT+1
  sta PRODUCT+1
  clc
  lda PRODUCT
  adc pysina_minus_pxcosa
  sta PRODUCT
  lda PRODUCT+1
  adc pysina_minus_pxcosa+1

  asl PRODUCT
  rol
  sta xfvertXhi,y
  lda PRODUCT
  sta xfvertXlo,y

  ; to finish, need to do the division

lda xfvertYhi, y
bmi Yneg
bne Ypos
lda xfvertYlo, y
bne Ypos
Yneg:
lda xfvertXhi, y
bpl Xpos
lda #$c0
bmi over
Xpos:
lda #$3f
over:
sta xfvertScreenX, y
bne continue
Ypos:
sty vertexCounter
lda xfvertXlo, y
ldx xfvertXhi, y
jsr pushax
ldy vertexCounter
lda xfvertYlo, y
ldx xfvertYhi, y
jsr _leftShift4ThenDiv
jsr clampIntToChar

ldy vertexCounter
sta xfvertScreenX, y

continue:

  dey
  bmi :+
  jmp loop2
  :

rts

.endproc

.endif