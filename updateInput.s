.setcpu		"6502"

.export updateInput
.export _readInput

.segment "CODE"

; bits reversed from normal understanding
; so lowest at the left
; -AD-JL--
;   -W--I---  <<2 (shift left because of reversal)
;-S------     >>1
;    ----K--- <<3
;=
; SADWJLIK

; sometime soon I'm going to have to make a zero page map!
;  put this on the zero page to speed up the interrupt
; although, really I should only read the keys every third or so interrupt for speed
keys = $34
framesToNextUpdate:
.byte 3

.proc updateInput : near

dec framesToNextUpdate
bne read
rts

read:

lda #3
sta framesToNextUpdate

; query the keyboard line containing <Ctrl>ADGJL;<Right>
lda #$fb
sta $9120
lda $9121
eor #$ff
and #$36 ; get ADJL
ora keys
sta keys

; query the keyboard line containing <Left>WRYIP*<Ret>
lda #$fd
sta $9120
lda $9121
eor #$ff
asl
asl
and #$48 ; get WI
ora keys
sta keys

; query the keyboard line containing <CBM>SFHK:=<F3>
lda #$df
sta $9120
lda $9121
eor #$ff
tax
lsr
and #$01
ora keys
sta keys
txa
asl
asl
asl
and #$80
ora keys
sta keys

rts

.endproc

.proc _readInput : near

ldx #0
lda keys
stx keys

rts

.endproc
