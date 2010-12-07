// doom for the vic-20
// written for cc65
// to compile, cl65 -t vic20 -C vic20-32k-udg.cfg -O vicdoom.c padding.s drawColumnInternalAsm.s -o vicdoom.prg

// todo
// X 1. move angle to sin/cos to logsin/logcos to a separate function and just use those values
// X 2. fix push_out code
// X 3. add transparent objects
// X 4. finish map and use that instead of the test map
// 5. enemy AI (update only visible enemies, plus enemies in the current sector)
// 5.5. per sector objects (link list)
// 6. add keys and doors
// 7. add health and weapon
// 8. advance levels
// 9. menus
// 10. more optimization?
// 11. use a double buffer scheme that draws to two different sets of characters and just copies the characters over
// 12. optimize push_out code

// memory map:
// see the .cfg file for how to do this
// startup code is $82 bytes long
// to make this work, I needed a "padding.s" that included ".segment "UDG" .res $77F,0"
// 1000-11FF screen
// 1200-13FF startup + random data
// 1400-15FF character font, copied from rom
// 1600-17FF 8x8 bitmapped display character ram
// 1800-19FF back buffer
// 1A00-1DFF texture data
// 1E00-2DFF level data, loaded from disk (not done yet)
// 2E00-7FFF code/data

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dbg.h>

#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

// 64 * sin( 64 values [0...2pi) )
static const signed char sinTab[64] =
{
   0, 6, 12, 19, 24, 30, 36, 41, 45, 49, 53, 56, 59, 61, 63, 64,
   64, 64, 63, 61, 59, 56, 53, 49, 45, 41, 36, 30, 24, 19, 12, 6,
   0, -6, -12, -19, -24, -30, -36, -41, -45, -49, -53, -56, -59, -61, -63, -63,
   -64, -64, -63, -61, -59, -56, -53, -49, -45, -41, -36, -30, -24, -19, -12, -6
};

int __fastcall__ muladd88(int x, int y, int z);
unsigned int __fastcall__ div88(unsigned int x, unsigned int y);
void __fastcall__ setCameraAngle(unsigned char a);
void __fastcall__ setCameraX(int x);
void __fastcall__ setCameraY(int y);
void __fastcall__ transformSectorToScreenSpace(char sectorIndex);
signed char __fastcall__ findFirstEdgeInSpan(char cameraOutsideSector, signed char x_L, signed char x_R);
int __fastcall__ transformxy_withParams(int x, int y);
int __fastcall__ transformy(void);
int __fastcall__ leftShift4ThenDiv(int p, unsigned int q);
char __fastcall__ getObjectTexIndex(unsigned int halfWidth, unsigned int x);
void __fastcall__ clearFilled(void);
char __fastcall__ testFilled(signed char col);
signed char __fastcall__ testFilledWithY(signed char col, unsigned int y);
void __fastcall__ setFilled(signed char col, unsigned int y);

unsigned char *secNumVerts;
int __fastcall__ getScreenX(unsigned char i);
int __fastcall__ getTransformedX(unsigned char i);
int __fastcall__ getTransformedY(unsigned char i);

char __fastcall__ getNumObjects(void);
char __fastcall__ getObjectSector(char o);
int __fastcall__ getObjectX(char o);
int __fastcall__ getObjectY(char o);
char __fastcall__ getObjectType(char o);

#define SOUND_CLAW 0
#define SOUND_DMPAIN 1
#define SOUND_DOROPN 2
#define SOUND_ITEMUP 3
#define SOUND_OOF 4
#define SOUND_PISTOL 5
#define SOUND_PLPAIN 6
#define SOUND_POPAIN 7
#define SOUND_SGCOCK 8
#define SOUND_SGTDTH 9
#define SOUND_SHOTGN 10
void __fastcall__ playSoundInitialize(void);
void __fastcall__ playSound(char soundIndex);

char dppistol[] = { 27, 29, 26, 48, 28, 25, 25, 30, 25, 38, 24, 27, 22, 30, 27, 18, 20, 13, 20, 11, 15, 11, 7, 13, 7, 9, 8, 6 };
char dpclaw[] = { 59, 36, 53, 36, 52, 37, 52, 38, 52, 39, 52, 39, 52, 39, 52, 38, 53, 35, 54, 35, 54, 34, 54, 33, 36, 33, 29, 32, 22, 31, 14, 30, 7, 29, 0, 26, 0, 25, 0, 21, 0, 18, 0, 16, 0, 15, 0, 10, 0, 8, 0, 7, 0, 6, 0, 5, 0, 5, 0, 5 };

signed char __fastcall__ get_sin(unsigned char angle)
{
  return sinTab[angle & 63];
}

signed char __fastcall__ get_cos(unsigned char angle)
{
  return sinTab[(angle + 16) & 63];
}

void waitforraster(void)
{
    while (PEEK(0x9004) > 16) ;
    while (PEEK(0x9004) < 16) ;
}

#if 0
typedef struct vertexT
{
  signed char x;
  signed char y;
} vertex;

typedef struct edgeT
{
  signed char tex;
  signed char sector;
  signed char index;
  char len;
} edge;

typedef struct sectorT
{
  char numverts;
  char dummy;
  char verts[7];
  char edges[7];
} sector;

vertex verts[8] =
{
  { -20, -10 },
  { -20, 10 },
  { 20, 10 },
  { 20, -10 },
  { -10, 10 },
  { -10, 20 },
  { 0, 20 },
  { 0, 10 }
};

edge edges[10] =
{
  { 0, -1, -1, 20 },
  { 1, -1, -1, 10 },
  { -1, 1, 1, 10 },
  { 1, -1, -1, 20 },
  { 2, -1, -1, 20 },
  { 0, -1, -1, 40 },
  { 1, -1, -1, 10 },
  { 2, -1, -1, 10 },
  { 0, -1, -1, 10 },
  { -1, 0, 1, 10 }
};

sector sectors[2] =
{
  { 6, -1, { 0, 1, 4, 7, 2, 3 }, { 0, 1, 2, 3, 4, 5 } },
  { 4, -1, { 4, 5, 6, 7 }, { 6, 7, 8, 9 } }
};

#endif

char spanStackSec[10];
signed char spanStackL[10];
signed char spanStackR[10];

typedef struct xfvertexT
{
  short x;
  short y;
  short screenx;
  short dummy;
} xfvertex;

// transformed sector verts
xfvertex xfverts[8];

typedef struct objtypeT
{
  char texture;
  char solid;
  char widthScale;
  char startY; // from the bottom of the texture
  char height;
  char startX;
  char width; // either 8 or 16
  char dummy;
} objtype;

objtype objtypes[16] =
{
  { 6, 0, 3 }, // player spawn
  { 9, 0, 3, 16, 16, 0, 16 }, // green armor
  { 6, 0, 3 }, // backpack
  { 11, 0, 8, 16, 16, 0, 16 }, // barrel
  { 10, 0, 8, 16, 8, 8, 8 }, // blue keycard
  { 6, 1, 3 }, // caco
  { 10, 0, 8, 24, 8, 0, 8 }, // medikit
  { 5, 1, 5 }, // imp
  { 7, 1, 3 }, // demon
  { 10, 0, 8, 24, 8, 8, 8 }, // red keycard
  { 10, 0, 5, 8, 8, 0, 16 }, // bullets
  { 5, 1, 5 }, // sargeant
  { 8, 1, 5 }, // pillar
  { 10, 0, 8, 16, 24, 0, 8 }, // green keycard
};

typedef struct objectT
{
  short x;
  short y;
  char angle;
  char type;
  char sector;
  char dummy;
} object;

object player = { -17*256, -11*256, 8, 0, 0 };
object *camera = &player;

#define TYPE_DOOR 1
#define TYPE_OBJECT 2
char typeAtCenterOfView;
char itemAtCenterOfView;
char doorOpenAmount[128];

#define SCREENWIDTH 32
#define HALFSCREENWIDTH (SCREENWIDTH/2)
#define SCREENHEIGHT 64
#define HALFSCREENHEIGHT (SCREENHEIGHT/2)
#define PIXELSPERMETER 4
#define TEXWIDTH 16
#define TEXHEIGHT 32
#define INNERCOLLISIONRADIUS 512
#define OUTERCOLLISIONRADIUS 515
#define COLLISIONDELTA (OUTERCOLLISIONRADIUS - INNERCOLLISIONRADIUS)

unsigned char frame = 0;

void __fastcall__ drawColumn(char textureIndex, char texI, signed char curX, short curY, unsigned short h);
void __fastcall__ drawColumnTransparent(char textureIndex, char texYStart, char texYEnd, char texI, signed char curX, short curY, unsigned short h);

char __fastcall__ getEdgeIndex(char sectorIndex, char edgeIndex);
char __fastcall__ getEdgeTexture(char sectorIndex, char edgeIndex);
char __fastcall__ getEdgeLen(char sectorIndex, char edgeIndex);

void drawWall(char sectorIndex, char curEdgeIndex, char nextEdgeIndex, signed char x_L, signed char x_R)
{
  //sector *sec = &sectors[sectorIndex];
  //edge *curEdge = &edges[sec->edges[curEdgeIndex]];
  char textureIndex = getEdgeTexture(sectorIndex, curEdgeIndex); // curEdge->tex;
  char edgeLen = getEdgeLen(sectorIndex, curEdgeIndex); // curEdge->len;

  // intersect the view direction and the edge
  // http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
  int x1 = getTransformedX(curEdgeIndex);
  int y1 = getTransformedY(curEdgeIndex);
  int dx = getTransformedX(nextEdgeIndex) - x1;
  int dy = getTransformedY(nextEdgeIndex) - y1;

  //int x3 = 0;
  //int y3 = 0;
  //int y4 = 256*1;

  int x4;
  int denom;
  signed char curX;
  int numer;
  unsigned int t;
  unsigned int texI;
  unsigned int curY;
  unsigned int h;

  // add 128 to correct for sampling in the center of the column
  x4 = (256*x_L + 128)/HALFSCREENWIDTH;
  for (curX = x_L; curX < x_R; ++curX)
  {
     //x4 = (256*curX + 128)/HALFSCREENWIDTH;
     x4 += 16;
     if (testFilled(curX) == 0)
     {
        // denom = dx - x4 * dy / 256;
        denom = muladd88(-x4, dy, dx);
        if (denom > 0)
        {
           // numer = x4 * ((long)y1) / 256 - x1;
           numer = muladd88(x4, y1, -x1);
           // t = 256 * numer / denom;
           t = div88(numer, denom);
           if (t > 256) t = 256;
           // curY = y1 + t * dy / 256;
           curY = muladd88(t, dy, y1);
           // perspective transform
           // Ys = Yw * (Ds/Dw) ; Ys = screenY, Yw = worldY, Ds = dist to screen, Dw = dist to point
           // h = (SCREENHEIGHT/16)*512/(curY/16);

           h = div88(128, curY);
           
           if (textureIndex == 2 || textureIndex == 4)
           {
	           // door or techwall, so fit to wall
				texI = t >> 4;
		   }
		   else
		   {
	           texI = t * edgeLen / 64; // 256/PIXELSPERMETER
	       }
           texI &= 15; // 16 texel wide texture
           
           setFilled(curX, curY);

           if (curX == 0 && textureIndex == 4)
           {
			 char edgeGlobalIndex = getEdgeIndex(sectorIndex, curEdgeIndex);
             typeAtCenterOfView = TYPE_DOOR;
             itemAtCenterOfView = edgeGlobalIndex;
           }

           // can look up the yStep (and starting texY) too
           // each is a 512 byte table - hooray for wasting memory
           // on the other hand, since I've already decided to waste 2k on a multiply table, I might as well use another 2k for lookups where appropriate
           drawColumn(textureIndex, texI, curX, curY, h);
        }
     }
  }
}

signed char drawDoor(char sectorIndex, char curEdgeIndex, char nextEdgeIndex, signed char x_L, signed char x_R)
{
  char edgeGlobalIndex = getEdgeIndex(sectorIndex, curEdgeIndex);
  char doorOpenAmount = doorOpenAmount[edgeGlobalIndex];
  char textureIndex, edgeLen;
  int x1, y1, dx, dy, x4, numer, denom;
  signed char curX;
  unsigned int t, texI, curY, h;
  
  gotoxy(0,1);
  cprintf("Door l %d r %d oa %d. ", x_L, x_R, doorOpenAmount);

  if (doorOpenAmount == 0)
  {
    drawWall(sectorIndex, curEdgeIndex, nextEdgeIndex, x_L, x_R);
    return x_R;
  }
  else if (doorOpenAmount == 255)
  {
    return x_L;
  }
  textureIndex = getEdgeTexture(sectorIndex, curEdgeIndex);
  edgeLen = getEdgeLen(sectorIndex, curEdgeIndex);

  // intersect the view direction and the edge
  // http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
  x1 = getTransformedX(curEdgeIndex);
  y1 = getTransformedY(curEdgeIndex);
  dx = getTransformedX(nextEdgeIndex) - x1;
  dy = getTransformedY(nextEdgeIndex) - y1;

  // add 128 to correct for sampling in the center of the column
  x4 = (256*x_L + 128)/HALFSCREENWIDTH;
  for (curX = x_L; curX < x_R; ++curX)
  {
     x4 += 16;
     if (testFilled(curX) == 0)
     {
        denom = muladd88(-x4, dy, dx);
        if (denom > 0)
        {
           numer = muladd88(x4, y1, -x1);
           t = div88(numer, denom);
           if (t > doorOpenAmount)
           {
              return curX;
           }
           t = 255 - (doorOpenAmount - t);
           curY = muladd88(t, dy, y1);
           h = div88(128, curY);
           // door or techwall, so fit to wall
  		   texI = t >> 4;
           
           setFilled(curX, curY);

           if (curX == 0)
           {
             typeAtCenterOfView = TYPE_DOOR;
             itemAtCenterOfView = edgeGlobalIndex;
           }

           // can look up the yStep (and starting texY) too
           // each is a 512 byte table - hooray for wasting memory
           // on the other hand, since I've already decided to waste 2k on a multiply table, I might as well use another 2k for lookups where appropriate
           drawColumn(textureIndex, texI, curX, curY, h);
        }
     }
  }
  return x_R;
}

void drawObjectInSector(char o, int vx, int vy, signed char x_L, signed char x_R)
{
  // perspective transform (see elsewhere for optimization)
  //int h = (SCREENHEIGHT/16) * 512 / (vy/16);
  unsigned int h = div88(128, vy);
  char objectType = getObjectType(o);
  unsigned int w = h/objtypes[objectType].widthScale;
  char textureIndex = objtypes[objectType].texture;
  int sx;
  int leftX;
  int rightX;

  int startX;
  int endX;
  signed char curX;
  char texI;
  if (w > 0)
  {
     //sx = vx / (vy / HALFSCREENWIDTH);
     sx = leftShift4ThenDiv(vx, vy);
     leftX = sx - w;
     rightX = sx + w;
     startX = leftX;
     endX = rightX;
     if (startX < x_L) startX = x_L;
     if (endX > x_R) endX = x_R;
     if (startX < x_R && endX > x_L)
     {
        for (curX = startX; curX < endX; ++curX)
        {
           if (testFilled(curX) == 0)
           {
              setFilled(curX, vy);
              if (curX == 0)
              {
                typeAtCenterOfView = TYPE_OBJECT;
                itemAtCenterOfView = o;
              }
              // compensate for pixel samples being mid column
              //texI = TEXWIDTH * (2*(curX - leftX) + 1) / (4 * w);
              texI = getObjectTexIndex(w, curX - leftX);
              if ((frame & 4) != 0) texI = (TEXWIDTH - 1) - texI;
              drawColumn(textureIndex, texI, curX, vy, h);
           }
        }
     }
  }
}

void drawTransparentObject(char o, int vx, int vy, signed char x_L, signed char x_R)
{
  // perspective transform (see elsewhere for optimization)
  //int h = (SCREENHEIGHT/16) * 512 / (vy/16);
  unsigned int h = div88(128, vy);
  char objectType = getObjectType(o);
  unsigned int w = h/objtypes[objectType].widthScale;
  char textureIndex = objtypes[objectType].texture;
  int sx;
  int leftX;
  int rightX;
  int startX;
  int endX;
  signed char curX;
  char texI;
  if (w > 0)
  {
     //sx = vx / (vy / HALFSCREENWIDTH);
     sx = leftShift4ThenDiv(vx, vy);
     leftX = sx - w;
     rightX = sx + w;
     startX = leftX;
     endX = rightX;
     if (startX < x_L) startX = x_L;
     if (endX > x_R) endX = x_R;
     if (startX < x_R && endX > x_L)
     {
        for (curX = startX; curX < endX; ++curX)
        {
           char startY = objtypes[objectType].startY;
           char height = objtypes[objectType].height;
           if (testFilledWithY(curX, vy) > 0)
           {
              if (curX == 0)
              {
                typeAtCenterOfView = TYPE_OBJECT;
                itemAtCenterOfView = o;
              }
              // compensate for pixel samples being mid column
              //texI = TEXWIDTH * (2*(curX - leftX) + 1) / (4 * w);
              texI = getObjectTexIndex(w, curX - leftX);
              if (objtypes[objectType].width != 16)
              {
                 texI = objtypes[objectType].startX + (texI>>1);
              }
              drawColumnTransparent(textureIndex, startY, height, texI, curX, vy, h);
           }
        }
     }
  }
}

typedef struct
{
  char o;
  char dummy;
  int x;
  unsigned int y;
  signed char x_L;
  signed char x_R;
}
objxy;

objxy unsorted[8];
char sorted[8];
char numSorted;
char numTransparent;
objxy transparent[12];

void drawObjectsInSector(char sectorIndex, signed char x_L, signed char x_R)
{
  char numObj = getNumObjects();
  int vx, vy;
  objxy *objInst;
  char o, i, j;
  numSorted = 0;
  
  // loop through the objects
  for (o = 0; o < numObj; ++o)
  {
	 char thisObjSec = getObjectSector(o);
     if (thisObjSec == sectorIndex)
     {
        // inverse transform
        vx = transformxy_withParams(getObjectX(o), getObjectY(o));
        vy = transformy();
        
        if (vy > 256)
        {
           sorted[numSorted] = numSorted;

           objInst = &unsorted[numSorted];
           objInst->x = vx;
           objInst->y = vy;
           objInst->o = o;

           ++numSorted;
        }
     }
  }

  if (numSorted > 0)
  {
	  // sort
	  for (i = 0; i < numSorted - 1; ++i)
	  {
		 for (j = i + 1; j < numSorted; ++j)
		 {
			if (unsorted[sorted[i]].y > unsorted[sorted[j]].y)
			{
			   o = sorted[j];
			   sorted[j] = sorted[i];
			   sorted[i] = o;
			}
		 }
	  }	  

	  // draw
	  for (i = 0; i < numSorted; ++i)
	  {
	     char type;
		 objInst = &unsorted[sorted[i]];
		 type = getObjectType(objInst->o);
		 if (objtypes[type].solid)
		 {
		   drawObjectInSector(objInst->o, objInst->x, objInst->y, x_L, x_R);
		 }
	  }
	}
}

void queueTransparentObjects(signed char x_L, signed char x_R)
{
   char i, type;
   objxy *objInst, *transp;
   for (i = 0; i < numSorted; ++i)
   {
        objInst = &unsorted[sorted[i]];
		type = getObjectType(objInst->o);
		if (!objtypes[type].solid)
        {
           transp = &transparent[numTransparent];
           transp->o = objInst->o;
           transp->x = objInst->x;
           transp->y = objInst->y;
           transp->x_L = x_L;
           transp->x_R = x_R;
           ++numTransparent;
        }
    }
}

void drawTransparentObjects()
{
   int i;
   objxy *objInst;
   for (i = numTransparent-1; i >= 0; --i)
   {
		objInst = &transparent[i];
		drawTransparentObject(objInst->o, objInst->x, objInst->y, objInst->x_L, objInst->x_R);
   }
}

char __fastcall__ getOtherSector(char sectorIndex, char edgeIndex);
char __fastcall__ getNextEdge(char sectorIndex, char edgeIndex);

void drawSpans()
{
  signed char stackTop = 0;
  char cameraOutsideSector = 0;
  char sectorIndex;
  signed char x_L, x_R;
  signed char firstEdge;
  char curEdge;
  signed char curX;
  char nextEdge;
  short nextX;
  signed char thatSector;

  clearFilled();
  numTransparent = 0;
  typeAtCenterOfView = 0;

  spanStackSec[0] = camera->sector;
  spanStackL[0] = -HALFSCREENWIDTH;
  spanStackR[0] = HALFSCREENWIDTH;

  while (stackTop >= 0)
  {
     sectorIndex = spanStackSec[stackTop];
     x_L = spanStackL[stackTop];
     x_R = spanStackR[stackTop];
     --stackTop;

     // STEP 1 - draw objects belonging to this sector!
     // fill in the table of written columns as we progress
     drawObjectsInSector(sectorIndex, x_L, x_R);

     //POKE(0x900f, 11);
     transformSectorToScreenSpace(sectorIndex);
     //POKE(0x900f, 13);

     firstEdge = findFirstEdgeInSpan(cameraOutsideSector, x_L, x_R);
     // any non-zero value means the camera is outside
     cameraOutsideSector++;
     // didn't find a first edge - must be behind
     if (firstEdge == -1)
     {
        continue;
     }
     
     // now fill the span buffer with these edges

     curEdge = firstEdge;
     curX = x_L;
     while (curX < x_R)
     {
        // update the edge
        nextEdge = getNextEdge(sectorIndex, curEdge);
        nextX = getScreenX(nextEdge);
        if (nextX < curX || nextX > x_R) nextX = x_R;

        thatSector = getOtherSector(sectorIndex, curEdge); //edges[sec->edges[curEdge]].sector;
        if (thatSector != -1)
        {
           if (getEdgeTexture(sectorIndex, curEdge) == 4)
           {
              curX = drawDoor(sectorIndex, curEdge, nextEdge, curX, nextX);
           }
           if (curX < nextX)
           {
			   // come back to this
			   ++stackTop;
			   spanStackSec[stackTop] = thatSector;
			   spanStackL[stackTop] = curX;
			   spanStackR[stackTop] = nextX;
		   }           
        }
        else
        {
           drawWall(sectorIndex, curEdge, nextEdge, curX, nextX);
        }
        curX = nextX;
        curEdge = nextEdge;
     }
     
     queueTransparentObjects(x_L, x_R);
  }

  drawTransparentObjects();
}

unsigned int sqrt(unsigned long x)
{
  unsigned long m = 0x40000000;
  unsigned long y = 0;
  unsigned long b;
  while (m != 0)
  {
     b = y | m;
     y = y >> 1;
     if (x >= b)
     {
        x -= b;
        y = y | m;
     }
     m = m >> 2;
  }
  return ((unsigned int)y);
}

// THIS IS THE NEXT TARGET OF OPTIMIZATION!

char __fastcall__ getNumVerts(char sectorIndex);
signed char __fastcall__ getSectorVertexX(char sectorIndex, char vertexIndex);
signed char __fastcall__ getSectorVertexY(char sectorIndex, char vertexIndex);

int push_out(object *obj)
{
  // probably a good idea to check the edges we can cross first
  // if any of them teleport us, move, then push_out in the new sector

  char thatSector;
  char i, ni;
  int v1x, v1y, v2x, v2y;
  long ex;
  long ey;
  long px, py;
  long height;
  long edgeLen;
  long dist;
  long distanceToPush;
  long dx, dy;
  char curSector = obj->sector;
  char secNumVerts = getNumVerts(curSector);
  
  // see which edge the new coordinate is behind
  for (i = 0; i < secNumVerts; ++i)
  {
	 ni = getNextEdge(curSector, i);
     v1x = getSectorVertexX(curSector, i);
     v1y = getSectorVertexY(curSector, i);
     v2x = getSectorVertexX(curSector, ni);
     v2y = getSectorVertexY(curSector, ni);
     ex = ((long)v2x) - v1x;
     ey = ((long)v2y) - v1y;
     px = obj->x - 256*v1x;
     py = obj->y - 256*v1y;
     // need to precalc 65536/edge.len
     edgeLen = getEdgeLen(curSector, i);
     height = (px * ey - py * ex) / edgeLen;
     if (height < INNERCOLLISIONRADIUS)
     {
        // check we're within the extents of the edge
        dist = (px * ex + py * ey)/edgeLen;
        if (dist > 0 && dist < 256*edgeLen)
        {
           thatSector = getOtherSector(curSector, i);
           if (thatSector != -1)
           {
              if (height < 0)
              {
                 obj->sector = thatSector;
                 gotoxy(1,0);
                 cprintf("sec%d ed%d ned%d ex%d ey%d. ", thatSector, i, ni, (int)ex, (int)ey);
                 return 1;
              }
           }
           else
           {
              // try just pushing out
              distanceToPush = OUTERCOLLISIONRADIUS - height;
              obj->x += distanceToPush * ey / edgeLen;
              obj->y -= distanceToPush * ex / edgeLen;
              return 1;
           }
        }
        else if (dist > -INNERCOLLISIONRADIUS
          && dist < 256*edgeLen + INNERCOLLISIONRADIUS)
        {
          if (dist <= 0)
			{
			   height = sqrt(px * px + py * py);
			   distanceToPush = INNERCOLLISIONRADIUS - height;
			   if (distanceToPush > 0)
			   {
				  distanceToPush += COLLISIONDELTA;
				  obj->x += distanceToPush * px / height;
				  obj->y += distanceToPush * py / height;
				  return 1;
			   }
			}
			else
			{
			   dx = obj->x - 256*v2x;
			   dy = obj->y - 256*v2y;
			   height = sqrt(dx * dx + dy * dy);
			   distanceToPush = INNERCOLLISIONRADIUS - height;
			   if (distanceToPush > 0)
			   {
				  distanceToPush += COLLISIONDELTA;
				  obj->x += distanceToPush * dx / height;
				  obj->y += distanceToPush * dy / height;
				  return 1;
			   }
			}
		}
     }
  }
  return 0;
}

void clearSecondBuffer(void);
void copyToPrimaryBuffer(void);

char keys;
char counter = 0;
char turnSpeed = 0;
char shells = 40;
char armor = 0;
char health = 100;
char shotgunStage = 0;
char changeLookTime = 7;
char lookDir = 0;

char soundToPlay = 0;
  
int main()
{
  int i, x, y;

  POKE(0x900E, 16*6 + (PEEK(0x900E)&15)); // blue aux color
  POKE(0x900F, 8 + 5); // green border, and black screen

  for (i = 0; i < 512; ++i)
  {
      // fill the screen with spaces
	  POKE(0x1000 + i, 32);
	  // set the color memory
	  POKE(0x9400 + i, 2); // main colour red, multicolour
  }
  // write an 8x8 block for the graphics
  // into the middle of the screen
  for (x = 0; x < 8; ++x)
  {
    for (y = 0; y < 8; ++y)
    {
      POKE(0x1000 + (x + 7) + 22*(y + 4), 64 + 8*x + y);
      POKE(0x9400 + (x + 7) + 22*(y + 4), 8 + 2);
    }
  }
  // set the character set to $1400
  POKE(0x9005, 13 + (PEEK(0x9005)&240));
  textcolor(6);
  gotoxy(0,17);
  cprintf("######################");
  gotoxy(0,19);
  cprintf("######################");
  textcolor(2);
  gotoxy(0,18);
  cprintf("knee deep in the dead!");
  gotoxy(0,21);
  textcolor(3);
  cprintf(" &40");
  textcolor(5);
  cprintf(" :000 ");
  textcolor(7);
  cprintf("()");
  textcolor(2);
  cprintf(" /100");
  textcolor(6);
  cprintf(" ;;;");
  gotoxy(10,20);
  textcolor(7);
  cprintf("$%%");
  gotoxy(10,22);
  cprintf("*+");
  
  playSoundInitialize();

  while (1)
  {
	  // note: XXXXYZZZ (X = screen, Y = reverse mode, Z = border)
	  POKE(0x900F, 8 + 5); // green border, and black screen

 	  // query the keyboard line containing <Ctrl>ADGJL;<Right>
	  POKE(0x9120, 0xFB);
	  keys = PEEK(0x9121);
	  if ((keys & 16) == 0)
	  {
	    if (turnSpeed < 2)
	    {
    	    turnSpeed++;
    	}
	    player.angle -= turnSpeed;
	  }
	  else if ((keys & 32) == 0)
	  {
	    if (turnSpeed < 2)
	    {
    	    turnSpeed++;
    	}
	    player.angle += turnSpeed;
	  }
	  else
	  {
	    turnSpeed = 0;
	  }
	  player.angle &= 63;
	  setCameraAngle(player.angle);
	  if ((keys & 2) == 0)
	  {
		player.x -= 4*get_cos(player.angle);
		player.y += 4*get_sin(player.angle);
	  }
	  if ((keys & 4) == 0)
	  {
		player.x += 4*get_cos(player.angle);
		player.y -= 4*get_sin(player.angle);
	  }

	  // query the keyboard line containing <Left>WRYIP*<Ret>
	  POKE(0x9120, 0xFD);
	  keys = PEEK(0x9121);
	  if ((keys & 2) == 0)
	  {
		player.x += 8*get_sin(player.angle);
		player.y += 8*get_cos(player.angle);
	  }
	  if (shotgunStage > 0) shotgunStage--;
	  if ((keys & 16) == 0)
	  {
		// pressed fire
		if (shells > 0 && shotgunStage == 0)
		{
		  shells--;
		  gotoxy(2,21);
		  textcolor(3);
		  cprintf("%02d", shells);
		  POKE(0x900F, 8+1);
		  shotgunStage = 7;
		  
		  playSound(soundToPlay);
		  soundToPlay++;
		  soundToPlay %= 12;
#if 0		  
		  POKE(0x900E,15);
		  for (i = 0; i < dpclaw[0]; ++i)
		  {
		    for (x = 0; x < 64; ++x);
		    POKE(0x900C, 160 + dpclaw[1+i]);		    
		    POKE(0x900D, 160 + dpclaw[1+i]);		    
		  }
		  POKE(0x900E,0);
#endif
		}
	  }

	  // query the keyboard line containing <CBM>SFHK:=<F3>
	  POKE(0x9120, 0xDF);
	  keys = PEEK(0x9121);
	  if ((keys & 2) == 0)
	  {
		player.x -= 8*get_sin(player.angle);
		player.y -= 8*get_cos(player.angle);
	  }
	  if ((keys & 16) == 0)
	  {
	      gotoxy(0,16);
	      cprintf("hi %d. ", typeAtCenterOfView);
	    // tried to open a door (pressed K)
	    if (typeAtCenterOfView == TYPE_DOOR)
	    {
          doorOpenAmount[itemAtCenterOfView] = 255 - doorOpenAmount[itemAtCenterOfView];
		  playSound(SOUND_DOROPN);
        }
      }

      //POKE(0x900f, 11);
	  if (push_out(&player))
	  {
		push_out(&player);
	  }
      //POKE(0x900f, 13);

      setCameraX(player.x);
      setCameraY(player.y);

      clearSecondBuffer();
	  // draw to second buffer
	  drawSpans();
	  // this takes about 30 raster lines
	  copyToPrimaryBuffer();
	  
	  ++frame;
	  frame &= 7;
	  
	  --changeLookTime;
	  if (changeLookTime == 0)
	  {
	    lookDir = 1 - lookDir;
	    gotoxy(10,21);
		textcolor(7);
		if (lookDir == 0)
		{
  	      changeLookTime = 12;
  		  cprintf("[\\");
  		}
  		else
  		{
  	      changeLookTime = 6;
  		  cprintf("()");
  		}
      }
	  
	  counter++;
	  POKE(0x1000, counter);
	}  
  
  return EXIT_SUCCESS;
}
