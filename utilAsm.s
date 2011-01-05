.export _eraseMessage

_eraseMessage:

ldx #21
lda #32
woop:
sta $1134,x
sta $114a,x
dex
bpl woop
rts
