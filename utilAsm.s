.setcpu "6502"
.segment "CODE"

.import addysp
.importzp sp
.export _eraseMessage
.export _load_file

_eraseMessage:

ldx #21
lda #32
woop:
sta $1134,x
sta $114a,x
dex
bpl woop
rts


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
