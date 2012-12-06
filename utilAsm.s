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

_clearScreen:

ldy #0
:
  lda #32
  sta $1000,y ; clear screen
  sta $1100,y
  lda #0
  sta $1600,y ; clear bitmap
  sta $1700,y
  iny
  bne :-

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
lda #10
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

  ldy #1
  jmp addysp

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

  ldy #1
  jmp addysp

.endproc

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

        JSR $FFBD     ; call SETNAM
        LDA #$01
        LDX #$08      ; default to device 8
        LDY #$01      ; $01 means: load to address stored in file
        JSR $FFBA     ; call SETLFS

        LDA #$00      ; $00 means: load to memory (not verify)
        JSR $FFD5     ; call LOAD

        ldy #2
        jmp addysp    ; clean up stack
