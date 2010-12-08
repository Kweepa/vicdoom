.setcpu		"6502"

.export _playSoundInitialize
.export _playSound
.import soundTable

; do not use $90-$95! must be overwritten by NMI
soundPointer = $30
soundIndex = $32
soundCount = $33
soundMax = $34
soundVIA = $9120
irqVector = $314
irqContinue = $eabf
; trigger every 7143 cycles (1000000/140)
timerValue = 7141

.segment "CODE"

.proc playSoundIrq : near

; check we're playing a sound
lda soundIndex
cmp #$ff
beq end

inc soundCount
ldy soundCount
cpy soundMax
beq stopPlaying
lda (soundPointer),y
sta $900c
sta $900d

end:
jmp irqContinue

stopPlaying:
lda #127
sta $900c
sta $900d

lda #$ff
sta soundIndex

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
; start the new sound playing
stx soundIndex
rts

.endproc

.proc _playSoundInitialize : near

lda #$ff
sta soundIndex

lda #$7f	; turn off all NMI
sta $911e

lda #<timerValue
ldx #>timerValue
sta soundVIA + $6
stx soundVIA + $5

lda #$40	; timer 1 free run
sta soundVIA + $b

; insert into chain
lda #<playSoundIrq
ldx #>playSoundIrq
sta irqVector
stx irqVector+1

lda #$C0    ; set timer 1 enable
sta soundVIA + $e

; turn up the volume
lda #0
sta $900a
sta $900b
sta $900c
sta $900d
lda $900e
and #$f0
ora #$07
sta $900e

rts

.endproc