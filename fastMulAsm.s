.setcpu "6502"
.autoimport	on

.export _generateMultiplicationTables
.export multiply_8bit_unsigned
.export multiply_8bit_signed
.export multiply_16bit_unsigned
.export multiply_16bit_signed

.segment "LUTS"

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

.proc _generateMultiplicationTables: near

; generate f(x)=int(x*x/4)
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

; generate f(x)=int((x-255)*(x-255)/4)
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

.endproc

T1 = $70
T2 = $72
PRODUCT = $74

; Description: Unsigned 8-bit multiplication with unsigned 16-bit result.
;                                                                        
; Input: 8-bit unsigned value in T1                                      
;        8-bit unsigned value in T2                                      
;        Carry=0: Re-use T1 from previous multiplication (faster)        
;        Carry=1: Set T1 (slower)                                        
;                                                                        
; Output: 16-bit unsigned value in PRODUCT                               
;                                                                        
; Clobbered: PRODUCT, X, A, C                                            
;                                                                        
; Allocation setup: T1,T2 and PRODUCT preferably on Zero-page.           
;                   square1_lo, square1_hi, square2_lo, square2_hi must be
;                   page aligned. Each table are 512 bytes. Total 2kb.    
;                                                                         
; Table generation: I:0..511                                              
;                   square1_lo = <((I*I)/4)                               
;                   square1_hi = >((I*I)/4)                               
;                   square2_lo = <(((I-255)*(I-255))/4)                   
;                   square2_hi = >(((I-255)*(I-255))/4)                   
.proc multiply_8bit_unsigned                                              
                bcc :+                                                    
                    lda T1                                                
                    sta sm1+1                                             
                    sta sm3+1                                             
                    eor #$ff                                              
                    sta sm2+1                                             
                    sta sm4+1                                             
                :                                                         

                ldx T2
                sec   
sm1:            lda square1_lo,x
sm2:            sbc square2_lo,x
                sta PRODUCT+0   
sm3:            lda square1_hi,x
sm4:            sbc square2_hi,x
                sta PRODUCT+1   

                rts
.endproc           



; Description: Signed 8-bit multiplication with signed 16-bit result.
;                                                                    
; Input: 8-bit signed value in T1                                    
;        8-bit signed value in T2                                    
;        Carry=0: Re-use T1 from previous multiplication (faster)    
;        Carry=1: Set T1 (slower)                                    
;                                                                    
; Output: 16-bit signed value in PRODUCT                             
;                                                                    
; Clobbered: PRODUCT, X, A, C                                        
.proc multiply_8bit_signed                                           
                jsr multiply_8bit_unsigned                           

                ; Apply sign (See C=Hacking16 for details).
                lda T1                                     
                bpl :+                                     
                    sec                                    
                    lda PRODUCT+1                          
                    sbc T2                                 
                    sta PRODUCT+1                          
                :                                          
                lda T2                                     
                bpl :+                                     
                    sec                                    
                    lda PRODUCT+1                          
                    sbc T1                                 
                    sta PRODUCT+1                          
                :                                          

                rts
.endproc           



; Description: Unsigned 16-bit multiplication with unsigned 32-bit result.
;                                                                         
; Input: 16-bit unsigned value in T1                                      
;        16-bit unsigned value in T2                                      
;        Carry=0: Re-use T1 from previous multiplication (faster)         
;        Carry=1: Set T1 (slower)                                         
;                                                                         
; Output: 32-bit unsigned value in PRODUCT                                
;                                                                         
; Clobbered: PRODUCT, X, A, C                                             
;                                                                         
; Allocation setup: T1,T2 and PRODUCT preferably on Zero-page.            
;                   square1_lo, square1_hi, square2_lo, square2_hi must be
;                   page aligned. Each table are 512 bytes. Total 2kb.    
;                                                                         
; Table generation: I:0..511                                              
;                   square1_lo = <((I*I)/4)                               
;                   square1_hi = >((I*I)/4)                               
;                   square2_lo = <(((I-255)*(I-255))/4)                   
;                   square2_hi = >(((I-255)*(I-255))/4)                   
.proc multiply_16bit_unsigned                                             
                ; <T1 * <T2 = AAaa                                        
                ; <T1 * >T2 = BBbb                                        
                ; >T1 * <T2 = CCcc                                        
                ; >T1 * >T2 = DDdd                                        
                ;                                                         
                ;       AAaa                                              
                ;     BBbb                                                
                ;     CCcc                                                
                ; + DDdd                                                  
                ; ----------                                              
                ;   PRODUCT!                                              

                ; Setup T1 if changed
                bcc :+               
                    lda T1+0         
                    sta sm1a+1       
                    sta sm3a+1       
                    sta sm5a+1       
                    sta sm7a+1       
                    eor #$ff         
                    sta sm2a+1       
                    sta sm4a+1       
                    sta sm6a+1       
                    sta sm8a+1       
                    lda T1+1         
                    sta sm1b+1       
                    sta sm3b+1       
                    sta sm5b+1       
                    sta sm7b+1       
                    eor #$ff         
                    sta sm2b+1       
                    sta sm4b+1       
                    sta sm6b+1       
                    sta sm8b+1       
                :                    

                ; Perform <T1 * <T2 = AAaa
                ldx T2+0                  
                sec                       
sm1a:           lda square1_lo,x          
sm2a:           sbc square2_lo,x          
                sta PRODUCT+0             
sm3a:           lda square1_hi,x          
sm4a:           sbc square2_hi,x          
                sta _AA+1                 

                ; Perform >T1_hi * <T2 = CCcc
                sec                          
sm1b:           lda square1_lo,x             
sm2b:           sbc square2_lo,x             
                sta _cc+1                    
sm3b:           lda square1_hi,x             
sm4b:           sbc square2_hi,x             
                sta _CC+1                    

                ; Perform <T1 * >T2 = BBbb
                ldx T2+1                  
                sec                       
sm5a:           lda square1_lo,x          
sm6a:           sbc square2_lo,x          
                sta _bb+1                 
sm7a:           lda square1_hi,x          
sm8a:           sbc square2_hi,x          
                sta _BB+1                 

                ; Perform >T1 * >T2 = DDdd
                sec                       
sm5b:           lda square1_lo,x          
sm6b:           sbc square2_lo,x          
                sta _dd+1                 
sm7b:           lda square1_hi,x          
sm8b:           sbc square2_hi,x          
                sta PRODUCT+3             

                ; Add the separate multiplications together
                clc                                        
_AA:            lda #0                                     
_bb:            adc #0                                     
                sta PRODUCT+1                              
_BB:            lda #0                                     
_CC:            adc #0                                     
                sta PRODUCT+2                              
                bcc :+                                     
                    inc PRODUCT+3                          
                    clc                                    
                :                                          
_cc:            lda #0                                     
                adc PRODUCT+1                              
                sta PRODUCT+1                              
_dd:            lda #0                                     
                adc PRODUCT+2                              
                sta PRODUCT+2                              
                bcc :+                                     
                    inc PRODUCT+3                          
                :                                          

                rts
.endproc           



; Description: Signed 16-bit multiplication with signed 32-bit result.
;                                                                     
; Input: 16-bit signed value in T1                                    
;        16-bit signed value in T2                                    
;        Carry=0: Re-use T1 from previous multiplication (faster)     
;        Carry=1: Set T1 (slower)                                     
;                                                                     
; Output: 32-bit signed value in PRODUCT                              
;
; Clobbered: PRODUCT, X, A, C
.proc multiply_16bit_signed
                jsr multiply_16bit_unsigned

                ; Apply sign (See C=Hacking16 for details).
                lda T1+1
                bpl :+
                    sec
                    lda PRODUCT+2
                    sbc T2+0
                    sta PRODUCT+2
                    lda PRODUCT+3
                    sbc T2+1
                    sta PRODUCT+3
                :
                lda T2+1
                bpl :+
                    sec
                    lda PRODUCT+2
                    sbc T1+0
                    sta PRODUCT+2
                    lda PRODUCT+3
                    sbc T1+1
                    sta PRODUCT+3
                :

                rts
.endproc



.if 0
.proc _transformxyNew: near

; y = x*sina + y*cosa
; x = x*cosa - y*sina

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

; x*sina
ldx #0
lda sina
sta T1
bpl sinIsPos
dex
sinIsPos:
stx T1+1
lda xToTransform
sta T2
lda xToTransform+1
sta T2+1
clc
jsr multiply_16bit_signed

lda PRODUCT+1
sta savey
lda PRODUCT+2
sta savey+1

; y*cosa
ldx #0
lda cosa
sta T1
bpl cosIsPos
dex
cosIsPos:
stx T1+1
lda yToTransform
sta T2
lda yToTransform+1
sta T2+1
clc
jsr multiply_16bit_signed

clc
lda PRODUCT+1
adc savey
sta savey
lda PRODUCT+2
adc savey+1
sta savey+1

; x*cosa
lda xToTransform
sta T2
lda xToTransform+1
sta T2+1
sec
jsr multiply_16bit_signed

lda PRODUCT+1
sta tmp+4
lda PRODUCT+2
sta tmp+5

; y*sina
ldx #0
lda sina
sta T1
bpl sinIsPos2
dex
sinIsPos2:
stx T1+1
lda yToTransform
sta T2
lda yToTransform+1
sta T2+1
clc
jsr multiply_16bit_signed

sec
lda tmp+4
sbc PRODUCT+1
tay
lda tmp+5
sbc PRODUCT+2
tax
tya

rts

.endproc
.endif