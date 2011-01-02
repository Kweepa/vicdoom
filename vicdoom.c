// doom for the vic-20
// written for cc65
// see make.bat for how to compile

// todo
// X 1. move angle to sin/cos and logsin/logcos to a separate function and just use those values
// X 2. fix push_out code
// 2.5. fix push_out code some more
// X 3. add transparent objects
// X 4. finish map and use that instead of the test map
// X 5. enemy AI (update only visible enemies, plus enemies in the current sector)
// X 5.5. per sector objects (link list)
// 6. add keys and doors
// X 7. add health
// 7.5. and weapons
// 8. advance levels
// X 9. menus
// 10. more optimization?
// X 11. use a double buffer scheme that draws to two different sets of characters and just copies the characters over
// 12. optimize push_out code and more importantly the ai try_move code

// memory map:
// see the .cfg file for how to do this
// startup code is $82 bytes long - fix with fill = yes
// 1000-11FF screen
// 1200-13FF startup + random data
// 1400-15FF character font
// 1600-17FF 8x8 bitmapped display character ram
// 1800-7FFF code/data
// A000-BDFF texture data, level data, music
// BE00-BFFF back buffer

#define IDDQD 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dbg.h>
#include <cbm.h>

#include "updateInput.h"
#include "playSound.h"
#include "mapAsm.h"
#include "drawColumn.h"
#include "automap.h"
#include "p_enemy.h"

#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

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

char P_Random(void);

// these get the player sin/cos
signed char __fastcall__ get_sin(void);
signed char __fastcall__ get_cos(void);

char spanStackSec[10];
signed char spanStackL[10];
signed char spanStackR[10];

typedef struct
{
  char texture;
  char dummy;
  char solid;
  char widthScale;
  char startY; // from the bottom of the texture
  char height;
  char startX;
  char width; // either 8 or 16
}
texFrame;

enum EObjType
{
   kOT_Possessed,
   kOT_Imp,
   kOT_Demon,
   kOT_Cacodemon,
   kOT_Baron,
   kOT_GreenArmor,
   kOT_BlueArmor,
   kOT_Bullets,
   kOT_Medkit,
   kOT_RedKeycard,
   kOT_GreenKeycard,
   kOT_BlueKeycard,
   kOT_Barrel,
   kOT_Pillar,
   kOT_Skullpile,
   kOT_Acid,
   kOT_PossessedCorpse,
   kOT_ImpCorpse,
   kOT_DemonCorpse,
   kOT_CacodemonCorpse,
   kOT_BaronCorpse
};

texFrame texFrames[] =
{
  { 5, 1, 1, 5 }, // possessed
  { 8, 1, 1, 5 }, // imp
  { 11, 1, 1, 3 }, // demon
  { 13, 1, 1, 3 }, // caco
  { 8, 1, 1, 5 }, // baron
  { 18, 0, 0, 5, 8, 8, 0, 16 }, // green armor
  { 18, 0, 0, 5, 0, 8, 0, 16 }, // blue armor
  { 19, 0, 0, 5, 8, 8, 0, 16 }, // bullets
  { 19, 0, 0, 8, 24, 8, 0, 8 }, // medikit
  { 19, 0, 0, 8, 24, 8, 8, 8 }, // red keycard
  { 19, 0, 0, 8, 16, 24, 0, 8 }, // green keycard
  { 19, 0, 0, 8, 16, 8, 8, 8 }, // blue keycard
  { 20, 0, 0, 8, 16, 16, 0, 16 }, // barrel
  { 17, 0, 1, 5 }, // pillar
  { 20, 0, 0, 8, 16, 16, 0, 16 }, // skullpile
  { 18, 0, 0, 2, 16, 4, 0, 16 }, // acid
  { 16, 0, 0, 5, 0, 8, 0, 16 }, // possessed corpse
  { 16, 0, 0, 5, 8, 8, 0, 16 }, // imp corpse
};

int playerx = -17*256, playery = -11*256;
char playera = 8;
char playerSector = 0;

char keyCards = 0;
char shells = 40;
char armor = 0;
signed char health = 100;

#define TYPE_DOOR 1
#define TYPE_OBJECT 2
char typeAtCenterOfView;
char itemAtCenterOfView;
char doorClosedAmount[128];
char openDoors[4];
char doorOpenTime[4];
char numOpenDoors = 0;

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

void drawWall(char sectorIndex, char curEdgeIndex, char nextEdgeIndex, signed char x_L, signed char x_R)
{
  char textureIndex = getEdgeTexture(sectorIndex, curEdgeIndex);
  char edgeLen = getEdgeLen(sectorIndex, curEdgeIndex);

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
  
  automap_sawEdge(getEdgeIndex(sectorIndex, curEdgeIndex));

  // add 128 to correct for sampling in the center of the column
  x4 = (256*x_L + 128)/HALFSCREENWIDTH;
  for (curX = x_L; curX < x_R; ++curX)
  {
     //x4 = (256*curX + 128)/HALFSCREENWIDTH;
     x4 += 16;
     if (testFilled(curX) == 0x7f)
     {
        // denom = dx - x4 * dy / 256;
        denom = muladd88(-x4, dy, dx);
        if (denom > 0)
        {
           // numer = x4 * ((long)y1) / 256 - x1;
           numer = muladd88(x4, y1, -x1);
           if (numer > 0)
           {
			   // t = 256 * numer / denom;
			   t = div88(numer, denom);
		   }
		   else
		   {
		      t = 0;
		   }
           if (t > 256) t = 256;
           // curY = y1 + t * dy / 256;
           curY = muladd88(t, dy, y1);
           setFilled(curX, curY);
           if (curY > 0)
           {
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
	           
			   if (curX == 0 && textureIndex == 4)
			   {
				 char edgeGlobalIndex = getEdgeIndex(sectorIndex, curEdgeIndex);
				 typeAtCenterOfView = TYPE_DOOR;
				 itemAtCenterOfView = edgeGlobalIndex;
			   }

			   drawColumn(textureIndex, texI, curX, curY, h);
			}
        }
     }
  }
}

signed char drawDoor(char sectorIndex, char curEdgeIndex, char nextEdgeIndex, signed char x_L, signed char x_R)
{
  char edgeGlobalIndex = getEdgeIndex(sectorIndex, curEdgeIndex);
  char doorClosedAmount = doorClosedAmount[edgeGlobalIndex];

  if (doorClosedAmount == 0)
  {
    return x_L;
  }
  drawWall(sectorIndex, curEdgeIndex, nextEdgeIndex, x_L, x_R);
  return x_R;
}

void drawObjectInSector(char o, int vx, int vy, signed char x_L, signed char x_R)
{
  // perspective transform (see elsewhere for optimization)
  //int h = (SCREENHEIGHT/16) * 512 / (vy/16);
  unsigned int h = div88(128, vy);

  char texFrameIndex = getObjectType(o);
  char animate = 0;
  char textureIndex;
  unsigned int w;
  int sx;
  int leftX;
  int rightX;
  int startX;
  int endX;
  signed char curX;
  char texI;

  if (texFrameIndex < 5)
  {
    textureIndex = p_enemy_get_texture(o);
    if (textureIndex & TEX_ANIMATE)
    {
      animate = 1;
      textureIndex &= ~TEX_ANIMATE;
    }
  }
  else
  {
    textureIndex = texFrames[texFrameIndex].texture;
  }
  w = h/texFrames[texFrameIndex].widthScale;
  if (w > 0)
  {
     //sx = vx / (vy / HALFSCREENWIDTH);
     sx = leftShift4ThenDiv(vx, vy);
     leftX = sx - w;
     rightX = sx + w;
     startX = leftX;
     endX = rightX;
     if (startX < -16) startX = -16;
     if (endX > 16) endX = 16;
     if (startX < x_R && endX > x_L)
     {
        if (startX < endX)
        {
          p_enemy_wasseenthisframe(o);
        }
        for (curX = startX; curX < endX; ++curX)
        {
           if (testFilledWithY(curX, vy) >= 0)
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
              if (animate)
              {
                // change the animation speed?
                if ((frame & 2) != 0) texI = (TEXWIDTH - 1) - texI;
              }
              drawColumn(textureIndex, texI, curX, vy, h);
           }
        }
     }
  }
}

void printIntAtXY(char i, char x, char y, char prec)
{
  int screenloc = 0x1000 + 22*y + x;
  char digit = 0;
  if (prec == 3)
  {
	  while (i >= 100) { ++digit; i -= 100; }
	  POKE(screenloc, '0' + digit);
      ++screenloc;
	  digit = 0;
  }
  while (i >= 10) { ++digit; i -= 10; }
  POKE(screenloc, '0' + digit);
  ++screenloc;
  POKE(screenloc, '0' + i);
}

char flashRedTime = 0;

void damagePlayer(char damage)
{
#if IDDQD == 0
  health -= damage;
#endif
  if (health < 0)
  {
     health = 0;
  }
  printIntAtXY(health, 14, 21, 3);
  
  // flash border red
  flashRedTime = 1;
  POKE(0x900F, 8+2);
}

void drawTransparentObject(char o, int vx, int vy, signed char x_L, signed char x_R)
{
  // perspective transform (see elsewhere for optimization)
  //int h = (SCREENHEIGHT/16) * 512 / (vy/16);
  unsigned int h = div88(128, vy);
  char texFrameIndex = getObjectType(o);
  unsigned int w = h/texFrames[texFrameIndex].widthScale;
  char textureIndex;
  int sx;
  int leftX;
  int rightX;
  int startX;
  int endX;
  signed char curX;
  char texI;
  char startY, height;

  if (w > 0)
  {
     //sx = vx / (vy / HALFSCREENWIDTH);
     sx = leftShift4ThenDiv(vx, vy);
     leftX = sx - w;
     rightX = sx + w;
     startX = leftX;
     endX = rightX;
     if (startX < -16) startX = -16;
     if (endX > 16) endX = 16;
     if (startX < x_R && endX > x_L)
     {
        textureIndex = texFrames[texFrameIndex].texture;
        startY = texFrames[texFrameIndex].startY;
        height = texFrames[texFrameIndex].height;
        for (curX = startX; curX < endX; ++curX)
        {
           if (testFilledWithY(curX, vy) > 0)
           {
              if (curX == 0 && (vy/256) < 3)
              {
                 if (texFrameIndex >= kOT_GreenArmor && texFrameIndex < kOT_BlueKeycard)
                 {
                   char pickedUp = 0;
                   switch (texFrameIndex)
                   {
                   case kOT_GreenArmor:
                      if (armor < 100)
                      {
                         armor = 100;
 						 printIntAtXY(100, 6, 21, 3);
                         pickedUp = 1;
					  }
                      break;
                   case kOT_BlueArmor:
                      if (armor < 200)
                      {
                         armor = 200;
 						 printIntAtXY(200, 6, 21, 3);
                         pickedUp = 1;
					  }
                      break;
                   case kOT_Bullets:
                      if (shells < 80)
                      {
                         shells += 10;
                         if (shells > 80) shells = 80;
                         printIntAtXY(shells, 2, 21, 2);
                         pickedUp = 1;
                      }
                      break;
                   case kOT_Medkit:
                      if (health < 100)
                      {
                        health += 25;
                        if (health > 100) health = 100;
                        printIntAtXY(health, 13, 21, 3);
                        pickedUp = 1;
                      }  
                      break;
                   case kOT_RedKeycard:
                      POKE(0x9400 + 22*21 + 17, 2);
                      keyCards |= 1;
                      pickedUp = 1;
                      break;
                   case kOT_GreenKeycard:
                      POKE(0x9400 + 22*21 + 18, 5);
                      keyCards |= 2;
                      pickedUp = 1;
                      break;
                   case kOT_BlueKeycard:
                      POKE(0x9400 + 22*21 + 19, 3);
                      keyCards |= 4;
                      pickedUp = 1;
                      break;
                   }
					 if (pickedUp)
					 {
						playSound(SOUND_ITEMUP);
						setObjectSector(o, -1);
					 }
                 }
              }
              // compensate for pixel samples being mid column
              //texI = TEXWIDTH * (2*(curX - leftX) + 1) / (4 * w);
              texI = getObjectTexIndex(w, curX - leftX);
              if (texFrames[texFrameIndex].width != 16)
              {
                 texI = texFrames[texFrameIndex].startX + (texI>>1);
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
  int vx, vy;
  objxy *objInst;
  char o, i, j;
  numSorted = 0;
  
  // loop through the objects
  for (o = getFirstObjectInSector(sectorIndex); o != 0xff; o = getNextObjectInSector(o))
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
		 if (texFrames[type].solid)
		 {
		   drawObjectInSector(objInst->o, objInst->x, objInst->y, x_L, x_R);
		   p_enemy_add_thinker(objInst->o);
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
		if (!texFrames[type].solid)
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

  spanStackSec[0] = playerSector;
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

        thatSector = getOtherSector(sectorIndex, curEdge);
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

// THIS IS THE NEXT TARGET OF FIXING & OPTIMIZATION!

char __fastcall__ getNumVerts(char sectorIndex);
signed char __fastcall__ getSectorVertexX(char sectorIndex, char vertexIndex);
signed char __fastcall__ getSectorVertexY(char sectorIndex, char vertexIndex);

char curSector;
char thatSector;
char nextSector;

char ni;
signed char v1x, v1y, v2x, v2y, ex, ey;
long px, py;
long height;
long edgeLen;
long dist;
long distanceToPush;
char edgeGlobalIndex;
char dgz, dle;

typedef enum
{
  kPOR_Nada,
  kPOR_Wall,
  kPOR_Sector
}
EPushOutResult;

// could cache ex/edgelen and ey/edgelen
// would be 512 bytes - could be worse!

EPushOutResult push_out_from_edge(char i)
{
	 ni = getNextEdge(curSector, i);
     v1x = getSectorVertexX(curSector, i);
     v1y = getSectorVertexY(curSector, i);
     v2x = getSectorVertexX(curSector, ni);
     v2y = getSectorVertexY(curSector, ni);
     ex = v2x - v1x;
     ey = v2y - v1y;
     px = playerx - 256*v1x;
     py = playery - 256*v1y;
     // need to precalc 65536/edge.len
     edgeLen = getEdgeLen(curSector, i);
     height = px * ey - py * ex;
     if (height < INNERCOLLISIONRADIUS*edgeLen)
     {
        // check we're within the extents of the edge
        dist = px * ex + py * ey;
        dgz = (dist >= 0);
        dle = (dist < 256*edgeLen*edgeLen);
        if (dgz & dle)
        {
           thatSector = getOtherSector(curSector, i);
           edgeGlobalIndex = getEdgeIndex(curSector, i);
           if (thatSector != -1 && doorClosedAmount[edgeGlobalIndex] == 0)
           {
              if (height < 0)
              {
                 nextSector = thatSector;
                 //gotoxy(1,0);
                 //cprintf("sec%d ed%d ned%d ex%d ey%d. ", thatSector, i, ni, (int)ex, (int)ey);
              }
              return kPOR_Sector;
           }
           else
           {
              // try just pushing out
              distanceToPush = OUTERCOLLISIONRADIUS*edgeLen - height;
              playerx += distanceToPush * ey / (edgeLen*edgeLen);
              playery -= distanceToPush * ex / (edgeLen*edgeLen);
              return kPOR_Wall;
           }
        }
        else if (!dgz && (dist > -INNERCOLLISIONRADIUS*edgeLen)
          || (!dle && dist < (256*edgeLen + INNERCOLLISIONRADIUS)*edgeLen))
        {
          if (dgz)
          {
		    px = playerx - 256*v2x;
		    py = playery - 256*v2y;
		  }
		   height = px * px + py * py;
		   if (height < INNERCOLLISIONRADIUS*INNERCOLLISIONRADIUS)
		   {
		      height = sqrt(height);
   		      distanceToPush = OUTERCOLLISIONRADIUS - height;
			  playerx += distanceToPush * px / height;
			  playery += distanceToPush * py / height;
			  return kPOR_Wall;
		   }
		}
     }
     return kPOR_Nada;
}

char touchedSector;

char possibleWallsToTouch[2];
char numPossibleWallsToTouch;

void push_out(void)
{
  // probably a good idea to check the edges we can cross first
  // if any of them teleport us, move, then push_out in the new sector
  
  // so, consider at most two edges to push out from
  // whether or not we get pushed in this sector, check any
  // portal edges we touch and try pushing out from edges in the neighbouring sectors
  // hopefully should only touch the one portal edge  

  char i, secNumVerts;
  EPushOutResult r;
  
  touchedSector = 0xff;
  nextSector = 0xff;

  curSector = playerSector;
  secNumVerts = getNumVerts(curSector);
  
  numPossibleWallsToTouch = 0;
  
  // see which edge the new coordinate is behind
  for (i = 0; i < secNumVerts; ++i)
  {
     r = push_out_from_edge(i);
     if (r == kPOR_Sector)
     {
       touchedSector = thatSector;
     }
     else if (r == kPOR_Wall)
     {
       // add the neighbouring edges
       if (i > 0)
       {
         possibleWallsToTouch[numPossibleWallsToTouch++] = i-1;
       }
       if (i == secNumVerts-1)
       {
         possibleWallsToTouch[numPossibleWallsToTouch++] = 0;
       }
     }
  }
  
  if (touchedSector != 0xff)
  {
    curSector = touchedSector;
    secNumVerts = getNumVerts(curSector);
    for (i = 0; i < secNumVerts; ++i)
    {
      r = push_out_from_edge(i);
      if (r == kPOR_Wall)
      {
         break;
      }
    }
  }
  if (numPossibleWallsToTouch > 0)
  {
    curSector = playerSector;
    for (i = 0; i < numPossibleWallsToTouch; ++i)
    {
      push_out_from_edge(possibleWallsToTouch[i]);
    }
  }
  if (nextSector != 0xff)
  {
    playerSector = nextSector;
  }
}

char counter = 0;
char turnSpeed = 0;
char shotgunStage = 0;
char changeLookTime = 7;
char lookDir = 0;

char soundToPlay = 0;

void setUpScreenForBitmap(void)
{
  int i, x, y;
  // clear the screen
  for (i = 0; i < 22*23; ++i)
  {
    POKE(0x1000 + i, 32);
  }
  // write an 8x8 block for the graphics
  // into the middle of the screen
  for (x = 0; x < 8; ++x)
  {
    for (y = 0; y < 8; ++y)
    {
      POKE(0x1000 + (x + 7) + 22*(y + 2), 64 + 8*x + y);
      POKE(0x9400 + (x + 7) + 22*(y + 2), 8 + 2);
    }
  }
}

void setUpScreenForMenu(void)
{
  int i;
  cputsxy(6, 1, "          ");
  cputsxy(6, 10, "          ");
  for (i = 2; i < 10; ++i)
  {
    cputsxy(6, i, " ");
    cputsxy(15, i, " ");
  }
}

void setUpScreenForGameplay(void)
{
  char i;
  char x;
  for (i = 0; i < 110; ++i)
  {
     POKE(0x1000 + 11*22 + i, 32);
  }
  clearSecondBuffer();
  copyToPrimaryBuffer();
  textcolor(6);
  cputsxy(0, 17, "######################");
  cputsxy(0, 19, "######################");
  cputsxy(6, 1, "##########");
  cputsxy(6, 10, "##########");
  for (i = 2; i < 10; ++i)
  {
    cputsxy(6, i, "#");
    cputsxy(15, i, "#");
    for (x = 0; x < 8; ++x)
    {
      // multicolor red
      POKE(0x9400 + 22*i + 7 + x, 8 + 2);
    }
  }
  POKE(0x900F, 8 + 5); // green border, and black screen
}

void runMenu(char canReturn);

void read_data_file(char *name, int addr, int maxSize)
{
  FILE *fp = fopen(name, "r");
  if (fp != NULL)
  {
    fread((void *)addr, maxSize, 1, fp);
    fclose(fp);
  }
}

// use this to line up raster timing lines
void waitforraster(void)
{
    while (PEEK(0x9004) > 16) ;
    while (PEEK(0x9004) < 16) ;
}

int main()
{
  char keys;
  char ctrlKeys;
  char i;
  signed char ca, sa;
  char numObj;
  
  read_data_file("e1m1mus", 0xB200, 0x900);
  read_data_file("sounds", 0xBB00, 0xC00);

  playSoundInitialize();

  read_data_file("sluts", 0x400, 0x400);
  read_data_file("textures", 0xA000, 0xC00);

  POKE(0x900E, (6<<4) + (PEEK(0x900E)&0x0f)); // blue aux color
  POKE(0x900F, 8 + 5); // green border, and black screen
  
  // set the character set to $1400
  POKE(0x9005, 13 + (PEEK(0x9005)&0xf0));

again:  
  setUpScreenForBitmap();

  setUpScreenForMenu();
  runMenu(0);
  setUpScreenForGameplay();

  read_data_file("e1m1", 0xAC00, 0x600);

  for (i = 0; i < 128; ++i)
  {
    if (getGlobalEdgeTexture(i) == 4)
    {
      doorClosedAmount[i] = 255;
    }
  }
  
  for (i = 0; i < numObj; ++i)
  {
    if (getObjectType(i) < 5)
    {
      allocMobj(i);
    }  
  }
  
  addObjectsToSectors();
  
  health = 100;
  armor = 0;
  shells = 40;
  keyCards = 0;
  playerx = getPlayerSpawnX();
  playery = getPlayerSpawnY();
  playera = getPlayerSpawnAngle();
  playerSector = getPlayerSpawnSector();

  // name of level
  textcolor(2);
  cputsxy(0, 18, "        hangar        ");
  // shells
  textcolor(3);
  cputsxy(0, 21, " &40");
  // armor
  textcolor(5);
  cputsxy(4, 21, " :000 ");
  // health
  textcolor(2);
  cputsxy(12, 21, " /100");
  // cards
  textcolor(6);
  cputsxy(17, 21, " ;;;");
  // face
  textcolor(7);
  cputsxy(10, 20, "$%");
  cputsxy(10, 21, "()");
  cputsxy(10, 22, "*+");
  
  while (health > 0)
  {
	  if (!flashRedTime)
	  {
 	    // note: XXXXYZZZ (X = screen, Y = reverse mode, Z = border)
  	    POKE(0x900F, 8 + 5); // green border, and black screen
  	  }
	  if (flashRedTime > 0)
	  {
	    --flashRedTime;
	  }

	  keys = readInput();
	  ctrlKeys = getControlKeys();
	  
	  if (ctrlKeys & KEY_ESC)
	  {
	    setUpScreenForMenu();
	    runMenu(1);
	    setUpScreenForGameplay();
	  }
	  else if (ctrlKeys & KEY_CTRL)
	  {
	    automap();
	    setUpScreenForGameplay();
	  }

	  if (keys & KEY_TURNLEFT)
	  {
	    if (turnSpeed < 2)
	    {
    	    turnSpeed++;
    	}
	    playera -= turnSpeed;
	  }
	  else if (keys & KEY_TURNRIGHT)
	  {
	    if (turnSpeed < 2)
	    {
    	    turnSpeed++;
    	}
	    playera += turnSpeed;
	  }
	  else
	  {
	    turnSpeed = 0;
	  }
	  playera &= 63;
	  setCameraAngle(playera);
	  ca = get_cos();
	  sa = get_sin();
	  if (keys & KEY_MOVELEFT)
	  {
		playerx -= 2*ca;
		playery += 2*sa;
	  }
	  if (keys & KEY_MOVERIGHT)
	  {
		playerx += 2*ca;
		playery -= 2*sa;
	  }

	  if (keys & KEY_FORWARD)
	  {
		playerx += 4*sa;
		playery += 4*ca;
	  }
	  if (keys & KEY_BACK)
	  {
		playerx -= 2*sa;
		playery -= 2*ca;
	  }
	  if (shotgunStage > 0)
	  {
	    shotgunStage--;
	    if (shotgunStage == 3)
	    {
			playSound(SOUND_SGCOCK);
		}
	  }
	  if (keys & KEY_FIRE)
	  {
		// pressed fire
		if (shells > 0 && shotgunStage == 0)
		{
		  shells--;
		  printIntAtXY(shells, 2, 21, 2);
		  POKE(0x900F, 8+1);
		  shotgunStage = 7;
		  
		  playSound(SOUND_PISTOL);
		  if (typeAtCenterOfView == TYPE_OBJECT)
		  {
		    p_enemy_damage(itemAtCenterOfView, 2 + (P_Random()&7));
		  }   
		}
	  }

      for (i = 0; i < 4; ++i)
      {
        if (doorOpenTime[i] > 0)
        {
          if (--doorOpenTime[i] == 0)
          {
            // try to close the door - should just get pushed out, so go ahead
            doorClosedAmount[openDoors[i]] = 255;
            playSound(SOUND_DORCLS);
          }
        }
      }
          
	  if (keys & KEY_USE)
	  {
	      //gotoxy(0,16);
	      //cprintf("hi %d. ", typeAtCenterOfView);
	    // tried to open a door (pressed K)
	    if (typeAtCenterOfView == TYPE_DOOR && testFilled(0) < 4)
	    {
	      for (i = 0; i < 4; ++i)
	      {
	        if (doorOpenTime[i] == 0)
	        {
				openDoors[i] = itemAtCenterOfView;
				doorOpenTime[i] = 20;
				break;
			}
	      }
          doorClosedAmount[itemAtCenterOfView] = 0;
		  playSound(SOUND_DOROPN);
        }
      }

      POKE(0x900f, 11);
      {
	    push_out();
	  }
      POKE(0x900f, 13);

      setCameraX(playerx);
      setCameraY(playery);

      p_enemy_startframe();
      clearSecondBuffer();
	  // draw to second buffer
	  drawSpans();
	  // this takes about 30 raster lines
	  copyToPrimaryBuffer();
	  p_enemy_think();
	  
	  ++frame;
	  frame &= 7;
	  
	  --changeLookTime;
	  if (changeLookTime == 0)
	  {
	    lookDir = 1 - lookDir;
		if (lookDir == 0)
		{
  	      changeLookTime = 12;
  	      POKE(0x1000 + 10 + 22*21, 27);
  	      POKE(0x1000 + 11 + 22*21, 28);
  		}
  		else
  		{
  	      changeLookTime = 6;
  	      POKE(0x1000 + 10 + 22*21, 40);
  	      POKE(0x1000 + 11 + 22*21, 41);
  		}
      }
	  
	  counter++;
	  POKE(0x1000, counter);
	}
	
	// screen melt
	do
	{
	  char x = 7 + (P_Random() & 7);
	  char y;

      textcolor(2);
	  cputsxy(5, 13, "you are dead");
	  cputsxy(5, 15, "press return");
	  
	  waitforraster();
	  
	  for (y = 9; y > 2; --y)
	  {
	     POKE(0x1000 + 22*y + x, PEEK(0x1000 + 22*(y-1) + x));
	  }
	  POKE(0x1000 + 44 + x, 32);
	  
	  keys = readInput();
	  ctrlKeys = getControlKeys();
	}
	while (!(ctrlKeys & KEY_RETURN));
	
	goto again;
	
	return EXIT_SUCCESS;
}
