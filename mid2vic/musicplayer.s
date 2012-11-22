.setCPU "6502"
.export _playMusicInitialize
.import startOfSong

irqVector = $314
irqContinue = $eabf

.segment "CODE"

; format of a song is
; statusByte [note] [statusByte [note]]* $ff
; status byte is [on/off:1][voice:2][timeToNextEvent:5]

timeToNextEvent:
.byte 0
noteInVoice:
.byte $ff, $ff, $ff, $ff
songIndex = $60
noteTable:
.byte 131, 140, 145, 151, 158, 161, 166, 173, 178, 181, 185, 189
.byte 192, 197, 200, 203, 206, 208, 211, 214, 216, 218, 220, 222
.byte 224, 226, 227, 229, 231, 232, 233, 234, 235, 236, 237, 239
noteTable2:
.byte 132, 141, 146, 152, 159, 162, 167, 174, 179, 182, 186, 190
.byte 195, 198, 201, 204, 207, 209, 212, 215, 217, 219, 221, 223
.byte 225, 227, 228, 230, 232, 233, 234, 235, 236, 237, 238, 240

.proc _playMusicIrq

; check this came from timer 1
bit $912d
bpl end

dec timeToNextEvent
beq nextEvent

; switch between notes

lda timeToNextEvent
and #1
bne odd

; even
ldx #3
loop:
lda noteInVoice,x
bmi @next
tay
lda noteTable,y
sta $900a,x
@next:
dex
bpl loop

bmi end

odd:
ldx #3
loop2:
lda noteInVoice,x
bmi @next
tay
lda noteTable2,y
sta $900a,x
@next:
dex
bpl loop2

end:

jmp irqContinue

nextEvent:

ldy #0
lda (songIndex),y
cmp #$ff
beq restart

; status byte [on/off:1][voice:2][timeToNextEvent:5]
pha
and #$1f
clc
adc #1
sta timeToNextEvent
pla
pha
rol
rol
rol
rol
and #$03
tax
pla
cmp #0
bpl turnOn

; turn off
lda #0
sta $900a,x
lda #$ff
sta noteInVoice,x

; increment songIndex
inc songIndex
bne end
inc songIndex+1
jmp end

restart:

lda #<startOfSong
sta songIndex
lda #>startOfSong
sta songIndex+1
lda #1
sta timeToNextEvent

jmp end

turnOn:

ldy #1
lda (songIndex),y
sta noteInVoice,x
tay
lda noteTable,y
sta $900a,x

lda songIndex
clc
adc #2
sta songIndex
lda songIndex+1
adc #0
sta songIndex+1

jmp end

.endproc


.proc _playMusic : near

lda #<startOfSong
sta songIndex
lda #>startOfSong
sta songIndex+1
lda #1
sta timeToNextEvent

lda #0
sta $900a
sta $900b
sta $900c
sta $900d
lda $900e
and $f0
ora $07
sta $900e

rts

.endproc

.proc _playMusicInitialize : near

jsr _playMusic

; insert into chain
lda #<_playMusicIrq
ldx #>_playMusicIrq
sta irqVector
stx irqVector+1

rts
.endproc
