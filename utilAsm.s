.setcpu "6502"
.segment "CODE"

.import addysp
.importzp sp
.import _P_Random
.import _readInput
.import _getControlKeys
.export _eraseMessage
.export _waitForRaster
.export _meltScreen
.export _load_file
.export _setTextColor
.export _print3DigitNumToScreen
.export _print2DigitNumToScreen
.export _clearScreen
.export _setupBitmap
.export _clearMenuArea
.export _drawBorders
.export _addKeyCard
.export _keyCardColor
.export _resetKeyCard
.export _haveKeyCard
.export _colorFace
.export _drawFace
.export _updateFace

.export _setObjForMobj
.export _objForMobj
.export _mobjForObj

.autoimport on

.segment "MIDCODE"

_eraseMessage:

ldx #21
lda #32
woop:
sta $1134,x
sta $114a,x
dex
bpl woop
rts

yyy:
.byte 0
yyyx22:
.byte 0
xxx:
.byte 0
charColor:
.byte 0

_clearScreen:

ldy #0
:
  lda #32
  sta $1000,y ; clear screen
  sta $1100,y
  iny
  bne :-
  rts

_setupBitmap:

sta charColor

  jsr _clearSecondBuffer
  jsr _copyToPrimaryBuffer

; write an 8x8 block for the graphics
; into the middle of the screen

lda #0
sta xxx

outerloop:

ldy #0
sty yyy

lda #51
sta yyyx22

innerloop:

lda yyyx22
clc
adc xxx
tax

lda xxx
asl
asl
asl
clc
adc yyy
adc #64

sta $1000,x
lda charColor
sta $9400,x

lda yyyx22
clc
adc #22
sta yyyx22

inc yyy
lda yyy
cmp #8
bne innerloop

inc xxx
lda xxx
cmp #8
bne outerloop

rts

_clearMenuArea:

lda #32
ldx #109
:
sta $10f2,x
dex
bpl :-
rts

_drawBorders:

tay

; borders at the bottom
ldx #21
:
tya
sta $1176,x
sta $11a2,x
lda #6
sta $9576,x
sta $95a2,x
dex
bpl :-

; top and bottom of bitmap

ldx #9
:
tya
sta $101c,x
sta $10e2,x
lda #6
sta $941c,x
sta $94e2,x
dex
bpl :-

; left and right of bitmap
ldx #154
:
tya
sta $1032,x
sta $103b,x
lda #6
sta $9432,x
sta $943b,x
txa
sec
sbc #22
tax
bcs :-

rts

.proc _waitForRaster: near

  tay
  @loop:
    :
    lda $9004
    cmp #16
    bpl :-
    :
    lda $9004
    cmp #16
    bmi :-
    dey
    bne @loop
  rts

.endproc

x22p7:
.byte 51, 73, 95, 117, 139, 161, 183
meltCount:
.byte 0
column:
.byte 0

.proc _meltScreen: near

    sta sm+1
   
    lda #180
    sta meltCount

again:
    jsr _P_Random
    lsr
    lsr
    and #7
    sta column
   
    lda #1
    jsr _waitForRaster
   
    ; melt column
    ldy #6
:
    lda x22p7,y
    clc
    adc column
    tax
    lda $1000,x
    sta $1000+22,x
    dey
    bpl :-
    ldx column
    lda #32
    sta $1000+51,x
   
sm: lda #0 ; health
    beq :+
   
    dec meltCount
    bne again
    rts
:
    jsr _readInput
    jsr _getControlKeys
    and #$80 ; KEY_RETURN
    beq again
    rts

.endproc


intToAsc:
  ldy #$2f
  ldx #$3a
  sec
:   iny
    sbc #100
    bcs :-
:   dex
    adc #10
    bmi :-
  adc #$2f
  rts

textcolor:
.byte 1

_setTextColor:
  sta textcolor
  rts

.proc _print3DigitNumToScreen: near
  ; AX pos
  ; TOS num (char)

  sta sm1+1
  sta sm2+1
  txa
  sta sm1+2
  clc
  adc #$84
  sta sm2+2

  ldy #0
  lda (sp),y
  jsr intToAsc
  pha
  txa
  pha
  tya
  pha

  ldy #0
  :
  pla
sm1: sta $1000,y
  lda textcolor
sm2: sta $9400,y
  iny
  cpy #3
  bne :-

  jmp incsp1

.endproc

.proc _print2DigitNumToScreen: near
  ; AX pos
  ; TOS num (char)

  sta sm1+1
  sta sm2+1
  txa
  sta sm1+2
  clc
  adc #$84
  sta sm2+2

  ldy #0
  lda (sp),y
  jsr intToAsc
  pha
  txa
  pha

  ldy #0
  :
  pla
sm1: sta $1000,y
  lda textcolor
sm2: sta $9400,y
  iny
  cpy #2
  bne :-

  jmp incsp1

.endproc

.segment "CODE"

; params: filename, length of filename
; A - length of fname
; TOS - fname

_load_file:
  pha
  ldy #0
  lda (sp), y
  tax           ; x contains low byte
  iny
  lda (sp), y
  tay           ; y contains high byte
  pla

  jsr SETNAM ; $FFBD
  lda #1
  ldx #8      ; default to device 8
  ldy #1      ; 1 means: load to address stored in file
  jsr SETLFS ; $FFBA

  lda #$00      ; $00 means: load to memory (not verify)
  jsr LOAD ; $FFD5

  jmp incsp2


keyCard:
.byte 0
keyCardColors:
.byte 0,2,5,6
keyCardMasks:
.byte 1,2,4,8

_resetKeyCard:
  lda #0
  sta keyCard
  rts

_addKeyCard:
  ora keyCard
  sta keyCard

  tay

  ldx #3
keyLoop:
  lda #';'
  sta $11df,x
  tya
  and keyCardMasks,x
  beq :+
  lda keyCardColors,x
  :
  sta $95df,x
  dex
  bne keyLoop
  rts

_keyCardColor:
  tax
  lda keyCardColors,x
  rts

_haveKeyCard:
  tax
  lda keyCardMasks,x
  and keyCard
  rts

faceColor:
.byte 7,3
faceChars:
.byte 27,28,35,36,42,43
faceOff:
.byte $c2,$c3,$d8,$d9,$ee,$ef
changeLookTime:
.byte 7
lookDir:
.byte 0

_colorFace:
  ; A = godMode 0 or 1
  ; convert to yellow (7) or cyan (3)
  tay
  lda faceColor,y
  pha
  ldx #5
  :
  lda faceOff,x
  tay
  pla
  sta $9500,y
  pha
  dex
  bpl :-
  pla
  rts

_drawFace:
  ldx #5
  :
  lda faceOff,x
  tay
  lda faceChars,x
  sta $1100,y
  dex
  bpl :-
  rts

_updateFace:
  dec changeLookTime
  beq :+
  rts
  :
  lda lookDir
  eor #$ff
  sta lookDir
  beq :+
  ldx #6
  ldy #40
  bne drawFacePart
  :
  ldx #12
  ldy #35
drawFacePart:
  stx changeLookTime
  sty $11d8
  iny
  sty $11d9
  rts


objForMobj:
.res 21,0
mobjForObj:
.res 49,0

_setObjForMobj:
  ; A - mobj
  ; TOS - obj

  ; x - mobj
  tax
  ; y - obj
  ldy #0
  lda (sp),y
  tay
  sta objForMobj,x
  txa
  sta mobjForObj,y
  ldy #1
  jmp incsp1

_objForMobj:
  tax
  lda objForMobj,x
  rts

_mobjForObj:
  tax
  lda mobjForObj,x
  rts
