#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

#define SCREENHEIGHT 64
#define HALFSCREENHEIGHT (SCREENHEIGHT/2)
#define TEXHEIGHT 32

extern const char *texture;

char mask[4] = { 0xC0, 0x30, 0x0C, 0x03 };

void __fastcall__ drawColumnInternal(char textureIndex, char texI, char curX, short texY, short yStep, char h)
{
  unsigned short screenAddr;
  char const *texAddr;
  register char yy;

     screenAddr = 0x1800 + SCREENHEIGHT*(curX/4) + (HALFSCREENHEIGHT - h);
     texAddr = texture + 128*textureIndex + TEXHEIGHT*(texI/4);
	 for (yy = 2*h; yy > 0; --yy)
	 {
		register char ty = texY/256;
		register char sv = PEEK(screenAddr);
		register char tv = texAddr[ty];
		register char tb = texI&3;
		register char sb = curX&3;
		if (tb > sb)
		{
			sv = sv | (mask[sb] & (tv << (2*(tb-sb))));
		}
		else
		{
			sv = sv | (mask[sb] & (tv >> (2*(sb-tb))));
		}				
		POKE(screenAddr, sv);
		screenAddr++;
		texY += yStep;
	 }
}

void transformSectorToScreenSpace(unsigned char sectorIndex)
{
  sector *sec = &sectors[sectorIndex];
  signed char edgeIndex = sec->numverts - 1;
  vertex *v;
  xfvertex *xfv;
  int xoff, yoff;
  char vertIndex;
  int tx, ty;
  int screenx;
  do
  {
     vertIndex = sec->verts[edgeIndex];
     v = &verts[vertIndex];
     xoff = 256*v->x - camera->x;
     yoff = 256*v->y - camera->y;
     // call the machine code
     tx = transformxy_withParams(xoff, yoff);
     ty = transformy();
     screenx = 0;
     if (ty > 0)
     {
        // screenx = HALFSCREENWIDTH*tx/ty;
        screenx = leftShift4ThenDiv(tx, ty);
     }
     else if (tx < 0)
     {
        screenx = -1000;
     }
     else
     {
        screenx = 1000;
     }
     xfv = &xfverts[edgeIndex];
     xfv->x = tx;
     xfv->y = ty;
     xfv->screenx = screenx;

     --edgeIndex;
  }
  while (edgeIndex >= 0);
}

char findFirstEdgeInSpan(char numVerts, signed char leftx, signed char rightx)
{
  signed char firstEdge = -1;
  char edgeIndex, edgeIndex2;
  xfvertex *xfv1;
  xfvertex *xfv2;
  short sx1, sx2;
  for (edgeIndex = 0; edgeIndex < numVerts; ++edgeIndex)
  {
     xfv1 = &xfverts[edgeIndex];
     sx1 = xfv1->screenx;
     edgeIndex2 = edgeIndex + 1;
     if (edgeIndex2 == numVerts) edgeIndex = 0;
     xfv2 = &xfverts[edgeIndex2];
     sx2 = xfv2->screenx;
     if ((sx1 <= leftx || (sx1 == 1000 && sx2 < rightx)) // left is off screen
        && sx2 > leftx
        && ((xfv1->y > 0) || (xfv2->y > 0))) // one in front
     {
        firstEdge = edgeIndex;
        break;
     }
  }
  return firstEdge;
}

.export _findFirstEdgeInSpan
.proc _findFirstEdgeInSpan: near

; x_R in A
; x_L on stack
; we're outside the sector flag further down the stack

; save and sign extend L & R
ldx #0
sta x_R
cmp #0
bpl signExtendR
dex
signExtendR:
stx x_R+1

ldx #0
ldy #0
lda (sp), y
sta x_L
bpl signExtendL
dex
signExtendL:
stx x_L+1

ldy #1
lda (sp),y
sta outsideSector

ldx #0
stx vertexCounter

keepLooking:

inx
cpx vertexCount
bne dontReset
ldx #0
dontReset:
stx vertexCounterPP

; if (sx2 > x_L)
; interesting that having x_L sign extended makes this quicker and clearer!
sec
lda xfvertScreenXlo, x
sbc x_L
lda xfvertScreenXhi, x
sbc x_L+1
bmi notThisVert

; if (vy2 >= 1 || vy1 >= 1)
sec
lda #1
sbc xfvertYlo, x
lda #0
sbc xfvertYhi, x

bmi keepConsideringThisVert1

ldx vertexCounter
sec
lda #1
sbc xfvertYlo, x
lda #0
sbc xfvertYhi, x
bpl notThisVert

keepConsideringThisVert1:

; if ((sx1 <= leftx || (sx1 == 1000 && sx2 < rightx)) // left is off screen

ldx vertexCounter
sec
lda x_L
sbc xfvertScreenXlo, x
lda x_L+1
sbc xfvertScreenXhi, x
bpl thisVert

; the second part of the test should only be done if the camera is inside the sector
lda outsideSector
bne notThisVert

lda xfvertScreenXlo, x
bne notThisVert
lda xfvertScreenXhi, x
cmp #$04
bne notThisVert

ldx vertexCounterPP
sec
lda xfvertScreenXlo, x
sbc x_R
lda xfvertScreenXhi, x
sbc x_R+1
bpl notThisVert

thisVert:

lda vertexCounter
ldy #2
jmp addysp

notThisVert:
inc vertexCounter
ldx vertexCounter
cpx vertexCount
bne keepLooking

lda #255
ldy #2
jmp addysp

.endproc

