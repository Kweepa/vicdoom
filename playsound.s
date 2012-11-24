.setcpu		"6502"

.export _playSoundInitialize
.export _playSound
.export _getEffectsVolume
.export _setEffectsVolume
.export _getMusicVolume
.export _setMusicVolume
.export _getTickCount
.export _setTickCount
.export _resetMapTime
.export _pauseMapTimer
.export _playMapTimer
.export _getMapTime
.export _startMusic
.export _stopMusic
.import soundTable
.import updateInput

irqVector = $314
irqContinue = $eabf


; format of a song is
; [statusByte [note]]+ $ff
; status byte is [on/off:1][voice:2][timeToNextEvent:5]

.segment "MUSIC"
.include "e1m1mus.s"

.segment "CODE"

mapTimeJiffies:
.byte 0
mapTimeSeconds:
.byte 0, 0
mapTimerOn:
.byte 0

timeToNextEvent:
.byte 0
noteInVoice:
.byte $ff, $ff, $ff, $ff
songIndex = $32
noteTable:
.byte 131, 140, 145, 151, 158, 161, 166, 173, 178, 181, 185, 189
.byte 192, 197, 200, 203, 206, 208, 211, 214, 216, 218, 220, 222
.byte 224, 226, 227, 229, 231, 232, 233, 234, 235, 237, 237, 239
noteTable2:
.byte 131, 140, 145, 151, 158, 162, 167, 174, 178, 182, 186, 190
.byte 195, 197, 200, 203, 207, 209, 212, 214, 216, 219, 221, 223
.byte 224, 226, 228, 229, 231, 232, 233, 235, 236, 237, 238, 239

soundPointer = $30
soundIndex:
.byte $ff
soundCount:
.byte 0
soundMax:
.byte 0
effectsVolume:
.byte $a
musicVolume:
.byte $1
tickCount:
.byte 0
musicPlaying:
.byte 0

.proc playSoundIrq : near

; check this came from timer 1
bit $912d
bmi :+
jmp end
:

jsr updateInput
inc tickCount

; update the map timer
lda mapTimerOn
beq :+
inc mapTimeJiffies
lda mapTimeJiffies
cmp #60
bne :+
lda #0
sta mapTimeJiffies
inc mapTimeSeconds
bne :+
inc mapTimeSeconds+1
:

; check we're playing a sound
lda soundIndex
cmp #$ff
beq playMusic

; play next sample
inc soundCount
ldy soundCount
cpy soundMax
beq stopPlaying
lda (soundPointer),y
; scale the sample back from 0..15
asl
asl
asl
adc #127
sta $900d

jmp end

stopPlaying:
lda #127
sta $900d

lda #$ff
sta soundIndex

; set to music volume
lda $900e
and #$f0
ora musicVolume
sta $900e

jmp end

playMusic:

lda musicPlaying
beq end

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

; make sure to set the volume
lda $900e
and #$f0
ora musicVolume
sta $900e

lda songIndex
clc
adc #2
sta songIndex
lda songIndex+1
adc #0
sta songIndex+1

jmp end

.endproc

.proc _playSound : near

tax
asl
tay
; first stop the old sound
lda #$ff
sta soundIndex
; then set up the pointer to the data
lda soundTable,y
sta soundPointer
lda soundTable+1,y
sta soundPointer+1
; then the counters
ldy #0
lda (soundPointer),y
tay
iny
sty soundMax
lda #0
sta soundCount

; turn off music
sta $900a
sta $900b
sta $900c

; turn up volume a bit
lda $900e
and #$f0
ora effectsVolume
sta $900e

; start the new sound playing
stx soundIndex

rts

.endproc

resetMusic:

lda #$ff
sta soundIndex

lda #<startOfSong
sta songIndex
lda #>startOfSong
sta songIndex+1
lda #1
sta timeToNextEvent

; turn off all channels
lda #0
sta $900a
sta $900b
sta $900c
sta $900d

; set music volume
lda $900e
and #$f0
ora musicVolume
sta $900e

; clear current notes
lda #255
ldx #3
:
sta noteInVoice,x
dex
bpl :-

rts

.proc _playSoundInitialize : near

sei
; insert into chain
lda #<playSoundIrq
ldx #>playSoundIrq
sta irqVector
stx irqVector+1

jsr resetMusic
cli
rts

.endproc

_startMusic:
sei
lda #1
sta musicPlaying
jsr resetMusic
cli
rts

_stopMusic:
sei
lda #0
sta musicPlaying
jsr resetMusic
cli
rts

_getEffectsVolume:
lda effectsVolume
rts

_setEffectsVolume:
sta effectsVolume
rts

_getMusicVolume:
lda musicVolume
rts

_setMusicVolume:
sta musicVolume
rts

_getTickCount:
lda tickCount
rts

_setTickCount:
lda #0
sta tickCount
rts

_resetMapTime:
lda #0
sta mapTimeJiffies
sta mapTimeSeconds
sta mapTimeSeconds+1
rts

_getMapTime:
lda mapTimeSeconds
ldx mapTimeSeconds+1
rts

_pauseMapTimer:
lda #0
sta mapTimerOn
rts

_playMapTimer:
lda #1
sta mapTimerOn
rts
