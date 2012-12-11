.export _updateCheatCodes

cheats:
.byte 'i', 'd', 'd', 'q', 'd', 0, 0, 0
.byte 'i', 'd', 'k', 'f', 'a', 0, 0, 0
.byte 'i', 'd', 'c', 'l', 'e', 'v', 0, 0
.byte 'i', 'd', 'd', 't', 0, 0, 0, 0
cheatp:
.byte 0, 0, 0, 0

key:
.byte 0
keyIndex:
.byte 0
usedCheat:
.byte 0

_updateCheatCodes:

lda #255
sta usedCheat
ldx #0

keyLoop:
stx keyIndex
cpx 198
bcs done

lda 631,x
sta key

ldx #3

cheatLoop:
txa
asl
asl
asl
clc
adc cheatp,x
tay

lda cheats,y
cmp key
beq partialMatch
lda #0
sta cheatp,x
beq continue

partialMatch:
inc cheatp,x
iny
lda cheats,y
bne continue

stx usedCheat
lda #0
sta cheatp,x

continue:
dex
bpl cheatLoop

ldx keyIndex
inx
bne keyLoop

done:

lda #0
sta 198

lda usedCheat
rts
