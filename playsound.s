.segment "CODE"

.export _playSoundInitialize
.export _playSound
.import soundTable

soundPointer = $90
soundIndex = $92
soundCount = $93
soundMax = $94

.proc playSoundIrq

lda soundIndex
cmp #$ff
beq end
inc soundCount
ldy soundCount
cpy soundMax
beq stopPlaying
lda (soundPointer),y
sta $900d
sta $900c

end:
jmp $eabf

stopPlaying:
lda #127
sta $900c
sta $900d

lda #$ff
sta soundIndex

jmp $eabf

.endproc

.proc _playSound

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
; start the new sound playing
stx soundIndex
rts

.endproc

.proc _playSoundInitialize

sei

lda #$ff
sta soundIndex

lda #$40	; loop
sta $911b

; trigger every 7143=$1BE6 cycles (1000000/140)
lda #$e6
sta $9116
lda #$1b
sta $9115

lda #<playSoundIrq
sta $314
lda #>playSoundIrq
sta $315

cli

; turn up the volume
lda #0
sta $900a
sta $900b
sta $900c
sta $900d
lda $900e
ora #$0f
sta $900e

rts

.endproc