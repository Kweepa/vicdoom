.setcpu "6502"
.autoimport	on
.export log2
.export exp2
.export _muladd88
.export _div88
.export _transformxy
.export _transformxy_withParams
.export _transformy
.export _leftShift4ThenDiv
.export _getObjectTexIndex
.export _setCameraAngle
.export _setCameraX
.export _setCameraY
.export _get_sin
.export _get_cos
.export _getSinOf
.export _getCosOf

.export _generateMulTab
.export _fastMulTest
.export _fastMultiplySetup8x8
.export _fastMultiply8x8
.export _fastMultiplySetup16x8
.export _fastMultiply16x8

.importzp sp

.segment "LUTS"

log2tab:

.byte 0, 1, 3, 4, 6, 7, 9, 10, 11, 13, 14, 16, 17, 18, 20, 21
.byte 22, 24, 25, 26, 28, 29, 30, 32, 33, 34, 36, 37, 38, 40, 41, 42
.byte 44, 45, 46, 47, 49, 50, 51, 52, 54, 55, 56, 57, 59, 60, 61, 62
.byte 63, 65, 66, 67, 68, 69, 71, 72, 73, 74, 75, 77, 78, 79, 80, 81
.byte 82, 84, 85, 86, 87, 88, 89, 90, 92, 93, 94, 95, 96, 97, 98, 99
.byte 100, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 116, 117
.byte 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133
.byte 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149

.byte 150, 151, 152, 153, 154, 155, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164
.byte 165, 166, 167, 168, 169, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 178
.byte 179, 180, 181, 182, 183, 184, 185, 185, 186, 187, 188, 189, 190, 191, 192, 192
.byte 193, 194, 195, 196, 197, 198, 198, 199, 200, 201, 202, 203, 203, 204, 205, 206
.byte 207, 208, 208, 209, 210, 211, 212, 212, 213, 214, 215, 216, 216, 217, 218, 219
.byte 220, 220, 221, 222, 223, 224, 224, 225, 226, 227, 228, 228, 229, 230, 231, 231
.byte 232, 233, 234, 234, 235, 236, 237, 238, 238, 239, 240, 241, 241, 242, 243, 244
.byte 244, 245, 246, 247, 247, 248, 249, 249, 250, 251, 252, 252, 253, 254, 255, 255

exp2tab:

.byte 0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 7, 8, 8, 9, 10, 10
.byte 11, 12, 13, 13, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 22, 22
.byte 23, 24, 25, 25, 26, 27, 28, 28, 29, 30, 31, 31, 32, 33, 34, 35
.byte 35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 48
.byte 48, 49, 50, 51, 52, 53, 53, 54, 55, 56, 57, 58, 58, 59, 60, 61
.byte 62, 63, 64, 64, 65, 66, 67, 68, 69, 70, 71, 71, 72, 73, 74, 75
.byte 76, 77, 78, 79, 80, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90
.byte 91, 92, 93, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105

.byte 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121
.byte 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 134, 135, 136, 137, 138
.byte 139, 140, 141, 142, 143, 144, 145, 146, 147, 149, 150, 151, 152, 153, 154, 155
.byte 156, 157, 159, 160, 161, 162, 163, 164, 165, 167, 168, 169, 170, 171, 172, 173
.byte 175, 176, 177, 178, 179, 180, 182, 183, 184, 185, 186, 188, 189, 190, 191, 192
.byte 194, 195, 196, 197, 199, 200, 201, 202, 204, 205, 206, 207, 209, 210, 211, 212
.byte 214, 215, 216, 217, 219, 220, 221, 223, 224, 225, 227, 228, 229, 230, 232, 233
.byte 234, 236, 237, 238, 240, 241, 242, 244, 245, 247, 248, 249, 251, 252, 253, 255

sintab:

.byte 0, 12, 24, 36, 48, 59, 70, 80, 89, 98, 105, 112, 117, 121, 124, 126
.byte 127, 126, 124, 121, 117, 112, 105, 98, 89, 80, 70, 59, 48, 36, 24, 12
.byte 0, -12, -24, -36, -48, -59, -70, -80, -89, -98, -105, -112, -117, -121, -124, -126
.byte -127, -126, -124, -121, -117, -112, -105, -98, -89, -80, -70, -59, -48, -36, -24, -12

.segment "CODE"

; unused ZP
tmple = $40
tmps = $41
tmp = $42
savey = $4C
angle = $50
cosa = $51
sina = $52
logcosa = $53
logsina = $55
cameraX = $57
cameraY = $59

.macro loadaxfromstack offs
	ldy #offs
	lda (sp),y
	tax
	dey
	lda (sp),y
.endmacro

.macro negate_ax
	clc
 	eor	#$FF
 	adc	#1
 	tay
 	txa
 	eor	#$FF
 	adc	#0
   	tax
 	tya
.endmacro

.macro add_to_stack val
.local skip
    ldy #val
    pha
 	clc
 	tya
 	adc	sp
 	sta	sp
 	bcc	skip
 	inc	sp+1
skip:	pla
.endmacro


; ---------------------------------------------------------------
; unsigned int __near__ __fastcall__ log2 (unsigned int x)
; ---------------------------------------------------------------

;      int e = 8;
;      while (x < 256)
;      {
;         x <<= 1;
;         --e;
;      }
;      while (x > 511)
;      {
;         x >>= 1;
;         ++e;
;      }
;      v = e*256 + log2[x&255];

.proc log2:near

sta tmple
txa
bne startshiftdown
ldx #0
shiftup:
dex
asl tmple
rol
beq shiftup
bne finishedshifting
startshiftdown:
ldx #0
shiftdown:
lsr
beq finishedshifting
ror tmple
inx
jmp shiftdown
finishedshifting:
ldy tmple
lda log2tab,y
rts

.endproc

; ---------------------------------------------------------------
; unsigned int __near__ __fastcall__ exp2 (unsigned int x)
; ---------------------------------------------------------------

;      int e = v/256;
;      int m = v&255;
;      int i = 256 + exp2[m];
;      while (e > 8)
;      {
;         i *= 2;
;         e--;
;      }
;      while (e < 8)
;      {
;         i /= 2;
;         e++;
;      }

.proc exp2:near

tay
lda #1
sta tmple
lda exp2tab,y
cpx #0
beq done
bmi shiftdown
bpl shiftup

done:
ldx #1
rts

shiftup:
asl
rol tmple
dex
bne shiftup
ldx tmple
rts

shiftdown:
lsr tmple
ror
inx
bne shiftdown
ldx tmple
rts

.endproc

; ---------------------------------------------------------------
; int __near__ __fastcall__ _muladd88(int p, int q, int r)
; ---------------------------------------------------------------

; multiply two 8.8 numbers and add a third
; p*q + r
; first pass, try using logs - see if it's accurate enough

.proc _muladd88:near

; save off r
sta tmp
stx tmp+1

; convert p and q to log
; first p

loadaxfromstack 1
cmp #0
bne pnotzero
cpx #0
bne pnotzero
jmp returnr

pnotzero:
stx tmps
cpx #0
bpl ppos

negate_ax

ppos:
jsr log2
sta tmp+2
stx tmp+3

; load x and check for zero
loadaxfromstack 3
cmp #0
bne xnotzero
cpx #0
bne xnotzero
jmp returnr

xnotzero:
tay
txa
eor tmps
sta tmps
tya
cpx #0
bpl qpos

negate_ax

qpos:
jsr log2
stx tmp+5

clc
adc tmp+2
sta tmp+4
lda tmp+5
adc tmp+3
tax
lda tmp+4
jsr exp2

ldy tmps
bpl pqpos

negate_ax

pqpos:
stx tmp+3
clc
adc tmp
tay
lda tmp+3
adc tmp+1
tax
tya

ldy #4
jmp addysp

returnr:
lda tmp
ldx tmp+1
add_to_stack 4
rts

.endproc

; ---------------------------------------------------------------
; unsigned int __near__ __fastcall__ _div88(unsigned int x, unsigned int y)
; ---------------------------------------------------------------

.proc _div88:near

jsr log2
sta tmp+2
stx tmp+3
loadaxfromstack 1
jsr log2
sec
sbc tmp+2
tay
txa
sbc tmp+3
tax
tya
jsr exp2

add_to_stack 2
rts

.endproc

; ---------------------------------------------------------------
; int __near__ __fastcall__ _transformxy_withParams(int x, int y)
; int __near__ __fastcall__ _transformxy(void)
; int __near__ __fastcall__ _transformy(void)
; ---------------------------------------------------------------
;
; x = x*cosa - y*sina
; y = x*sina + y*cosa

.proc _transformy:near

lda savey
ldx savey+1
rts

.endproc

; int __fastcall__ _mul88(int p, int q)
; p in ax
; logq in tmp/tmp+1, sign of q in tmps

.proc _mul88:near

; convert p to log
; first check it's not zero
cmp #0
bne pnotzero
cpx #0
bne pnotzero
rts

pnotzero:
tay
txa
eor tmps
sta tmps
tya
cpx #0
bpl ppos

negate_ax

ppos:
jsr log2
sta tmp+2
stx tmp+3

lda tmp
clc
adc tmp+2
tay
lda tmp+1
adc tmp+3
tax
tya

jsr exp2

ldy tmps
bpl pqpos

negate_ax

pqpos:
rts

.endproc

; ---------------------------------------------------------------

; zero page vars (see mapAsm - keep in sync)
xToTransform = $68
yToTransform = $6A

.proc _transformxy_withParams: near

sta yToTransform
stx yToTransform+1
loadaxfromstack 1
sta xToTransform
stx xToTransform+1
ldy #2
jsr addysp

; let it fall into the next routine

.endproc

.proc _transformxy:near

sec
lda xToTransform
sbc cameraX
sta xToTransform
lda xToTransform+1
sbc cameraX+1
sta xToTransform+1

sec
lda yToTransform
sbc cameraY
sta yToTransform
lda yToTransform+1
sbc cameraY+1
sta yToTransform+1

lda angle
and #15
bne nonaxis

lda angle
cmp #0
bne notrot0

; norot
lda yToTransform
ldx yToTransform+1
sta savey
stx savey+1
lda xToTransform
ldx xToTransform+1
rts

notrot0:
cmp #16
bne notrot90

; rot90
; x <- -y
; y <- x
lda xToTransform
ldx xToTransform+1
sta savey
stx savey+1
lda yToTransform
ldx yToTransform+1
jsr negax
rts

notrot90:
cmp #32
bne notrot180

; rot180
; x <- -x
; y <- -y
lda yToTransform
ldx yToTransform+1
jsr negax
sta savey
stx savey+1
lda xToTransform
ldx xToTransform+1
jsr negax
rts

notrot180:

; rot270
; x <- y
; y <- -x
lda xToTransform
ldx xToTransform+1
jsr negax
sta savey
stx savey+1
lda yToTransform
ldx yToTransform+1
rts

nonaxis:

; y = x*sina + y*cosa

; load up cosa
lda logcosa
sta tmp
lda logcosa+1
sta tmp+1
lda cosa
sta tmps
; y into AX
lda yToTransform
ldx yToTransform+1
jsr _mul88
sta savey
stx savey+1

; load up sina
lda logsina
sta tmp
lda logsina+1
sta tmp+1
lda sina
sta tmps
; get x into AX
lda xToTransform
ldx xToTransform+1
jsr _mul88

clc
adc savey
sta savey
txa
adc savey+1
sta savey+1

; got y, now x
; x = x*cosa - y*sina

; load sina
lda logsina
sta tmp
lda logsina+1
sta tmp+1
lda sina
sta tmps
; multiply by y
lda yToTransform
ldx yToTransform+1
jsr _mul88
sta tmp+4
stx tmp+5

lda logcosa
sta tmp
lda logcosa+1
sta tmp+1
lda cosa
sta tmps
; multiply by x
lda xToTransform
ldx xToTransform+1
jsr _mul88

sec
sbc tmp+4
tay
txa
sbc tmp+5
tax
tya

rts

.endproc

; ---------------------------------------------------------------
; int __near__ __fastcall__ _leftShift4ThenDiv(int p, unsigned int q)
; ---------------------------------------------------------------

.proc _leftShift4ThenDiv:near

jsr log2
sta tmp
txa
clc
adc #(8-4) ; compensate for 8.8 assumption in log and exp, and right shift denom
sta tmp+1

loadaxfromstack 1
cmp #0
bne xnotzero
cpx #0
bne xnotzero
jmp pwaspos

xnotzero:
stx tmps
cpx #0
bpl pispos

negate_ax

pispos:
jsr log2
stx tmp+3

sec
sbc tmp
tay
lda tmp+3
sbc tmp+1
tax
tya
jsr exp2

ldy tmps
cpy #0
bpl pwaspos

negate_ax

pwaspos:
add_to_stack 2
rts

.endproc

; ---------------------------------------------------------------
; unsigned char __fastcall__ _getObjectTexIndex(unsigned int halfWidth, unsigned int x);
; ---------------------------------------------------------------

.proc _getObjectTexIndex:near

cmp #0
bne txnotzero
cpx #0
bne txnotzero
ldy #2
jmp addysp

txnotzero:

; 2*x + 1
stx tmp+1
asl
rol tmp+1
clc
adc #1
tay
lda tmp+1
adc #0
tax
tya

; log, then multiply by 16/4 (tex width/4) and divide by 256
jsr log2
sta tmp
txa
sec
sbc #6
sta tmp+1

; take log of w
loadaxfromstack 1
jsr log2
sta tmp+2
stx tmp+3

; divide, then exp
lda tmp
sec
sbc tmp+2
tay
lda tmp+1
sbc tmp+3
tax
tya
jsr exp2

and #15

add_to_stack 2
rts

.endproc


; ---------------------------------------------------------------
; void __fastcall__ _setCameraAngle(unsigned char a);
; ---------------------------------------------------------------

.proc _setCameraAngle:near

sta angle
tay
lda sintab,y
sta sina
tya
clc
adc #16
and #63
tay
lda sintab,y
sta cosa

lda angle
and #15
beq sca_done

lda #0
ldx sina
cpx #0
bpl sca_sinPositive
jsr negax
sca_sinPositive:
jsr log2

; divide by 256*127 (subtract 06FD)
; this routine is purely for the transform

sec
sbc #$FD
sta logsina
txa
sbc #$06
sta logsina+1

lda #0
ldx cosa
cpx #0
bpl sca_cosPositive
jsr negax
sca_cosPositive:
jsr log2

; divide by 256*127 (subtract 06FD)
; this routine is purely for the transform

sec
sbc #$FD
sta logcosa
txa
sbc #$06
sta logcosa+1

sca_done:
rts

.endproc

; ---------------------------------------------------------------
; void __fastcall__ _setCameraX/Y(int val);
; ---------------------------------------------------------------

.proc _setCameraX: near
sta cameraX
stx cameraX+1
rts
.endproc

.proc _setCameraY: near
sta cameraY
stx cameraY+1
rts
.endproc


_get_sin:
lda sina
rts

_get_cos:
lda cosa
rts


_getSinOf:
tay
lda sintab,y
rts

_getCosOf:
clc
adc #16
and #63
tay
lda sintab,y
rts


;-----------------------------------------------------------
; fast multiply with tables
;-----------------------------------------------------------
; in this case want 16bit signed * 8bit signed = 24bit signed
; but we can drop the least significant byte so the result is
; just 16bit
; ABxC = AxC<<8 + BxC
; need full AxC result but just high byte of BxC
; also, need to convert to unsigned before multiply
; and back again after

.segment "HILUTS"

multablo:
square1_lo:
.res $200, 0
multabhi:
square1_hi:
.res $200, 0
multab2lo:
square2_lo:
.res $200, 0
multab2hi:
square2_hi:
.res $200, 0

.segment "CODE"

; misc numeric work area and accum #1 (according to the PRG)
; at $57-$66 on the ZP
; safe to use $5b up (see usage above)

T1 = $5b
T2 = $5c
PRODUCT = $5e

; only touches A
_fastMultiplySetup8x8:

  sta T1
  sta sm1d+1
  sta sm3d+1
  sta sm5d+1
  sta sm6d+1
  eor #$ff
  sta sm2d+1
  sta sm4d+1
  rts

; only touches A and X
_fastMultiply8x8:
                sta T2
                tax
                sec                       
sm1d:           lda square1_lo,x          
sm2d:           sbc square2_lo,x          
                sta PRODUCT
sm3d:           lda square1_hi,x          
sm4d:           sbc square2_hi,x

        ; fix for sign (CHacking16)
        ; since this is fixed, the setup could obliterate the code
        ; best to do something like BIT $1000 or CMP $1000 (3 bytes, 4 cycles) as the NOP
        ; way too expensive setup (20 cycles extra vs saving 5 per multiply)
sm5d:   ldx #0 ; T1
        bpl :+
        ; sub 8bit number
        sec
        sbc T2
        :

        ldx T2
        bpl :+
        sec
sm6d:   sbc #0 ; T1
        :

        sta PRODUCT+1
        lda PRODUCT
        ; make sure N is set based on MSB on return
        ldx PRODUCT+1
        ; return value in AX, but also in PRODUCT/PRODUCT+1 for max flexibility
        rts


_fastMultiplySetup16x8:

  sta T1
  sta sm1a+1                                             
  sta sm3a+1                                             
  sta sm3b+1
  eor #$ff                                              
  sta sm2a+1                                             
  sta sm4a+1                                             
  sta sm4b+1
  rts

_fastMultiply16x8:
; AX 16bit value
; y 8bit value

; A * y = AAaa                                        
; X * y = BBbb                                        

;       AAaa                                              
;  +  BBbb                                                
; ----------                                              
;   PRODUCT!                                              
; since we are just using the upper 16 bits, we can ignore aa

                sta T2
                stx T2+1
                ; Perform X * y = BBbb
                sec                       
sm1a:           lda square1_lo,x          
sm2a:           sbc square2_lo,x          
                sta _bb+1
sm3a:           lda square1_hi,x          
sm4a:           sbc square2_hi,x
                sta PRODUCT+1

                ; Perform A * y = AAaa
                ldx T2
                sec                          
.if 0
sm1b:           lda square1_lo,x  ; only need this for one bit of extra accuracy           
sm2b:           sbc square2_lo,x             
                sta _aa+1        ; don't need this                  
.endif
sm3b:           lda square1_hi,x             
sm4b:           sbc square2_hi,x             
                sta _AA+1                    

        clc
_AA:    lda #0
_bb:    adc #0
        sta PRODUCT
        bcc :+
        inc PRODUCT+1
:

; fix for sign - see CHacking16

        lda T1
        bpl :+
        ; sub 16bit number
        sec
        lda PRODUCT
        sbc T2
        sta PRODUCT
        lda PRODUCT+1
        sbc T2+1
        sta PRODUCT+1
        :
        lda T2+1
        bpl :+
        ; sub 8bit number
        sec
        lda PRODUCT+1
        sbc T1
        sta PRODUCT+1
        :

        lda PRODUCT
        ldx PRODUCT+1

rts

_generateMulTab:

      ldx #$00
      txa
      .byte $c9
lb1:  tya
      adc #$00
ml1:  sta multabhi,x
      tay
      cmp #$40
      txa
      ror
ml9:  adc #$00
      sta ml9+1
      inx
ml0:  sta multablo,x
      bne lb1
      inc ml0+2
      inc ml1+2
      clc
      iny
      bne lb1

ldx #$00
ldy #$ff
:
   lda multabhi+1,x
   sta multab2hi+$100,x
   lda multabhi,x
   sta multab2hi,y
   lda multablo+1,x
   sta multab2lo+$100,x
   lda multablo,x
   sta multab2lo,y
   dey
   inx
bne :-

   rts

_fastMulTest:

  lda #$41
  jsr _fastMultiplySetup16x8
  lda #$77
  ldx #$FE
  jmp _fastMultiply16x8
