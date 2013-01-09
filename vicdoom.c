// doom for the vic-20
// written for cc65
// see make.bat for how to compile

// done
// X 1. move angle to sin/cos and logsin/logcos to a separate function and just use those values
// X 2. fix push_out code
// X 2.5. fix push_out code some more
// X 3. add transparent objects
// X 4. finish map and use that instead of the test map
// X 5. enemy AI (update only visible enemies, plus enemies in the current sector)
// X 5.5. per sector objects (link list)
// X 6. add keys and doors
// X 7. add health
// X 8. advance levels
// X 9. menus
// X 10. more optimization?
// X 11. use a double buffer scheme that draws to two different sets of characters and just copies the characters over
// X 12. optimize push_out code
// X 12.5 and the ai try_move code
// X 13. projectiles!
// X 14. remote doors need to be openable from the other side - new edge prop that opens a door with e=e-1
// X 16. make acid do damage

// todo
// 17. fix sound glitches when loading data
// 18. add weapons (maybe?)

// memory map:
// see the .cfg file for how to do this
// startup code is $82 bytes long - fix with fill = yes
// 0400-0FFF look up tables and code
// 1000-11FF screen
// 1200-13FF startup + random data
// 1400-15FF character font
// 1600-17FF 8x8 bitmapped display character ram
// 1800-76FF code/data
// 7700-77FF C stack
// 7800-7FFF fast multiply tables
// A000-BDFF texture data, level data, music, sound, code
// BE00-BFFF back buffer

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
#include "util.h"
#include "summary.h"
#include "victory.h"
#include "fastmath.h"
#include "enemy.h"

#pragma staticlocals(on)

void __fastcall__ setCameraAngle(unsigned char a);
void __fastcall__ setCameraX(int x);
void __fastcall__ setCameraY(int y);
void __fastcall__ preTransformSectors(void);
void __fastcall__ transformSectorToScreenSpace(char sectorIndex);
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

char *pickupNames[] =
{
  "green armor!",
  "blue armor!",
  "ammo!",
  "medikit!",
  "red keycard!",
  "green keycard!",
  "blue keycard!",
  "", "", "", "",
  "chainsaw!",
  "shotgun!",
  "chaingun!"
};

char *weaponNames[] =
{
  "pistol",
  "shotgun",
  "chaingun"
};

int playerx;
int playery;
char playera;
char playerSector;

int playeroldx;
int playeroldy;

// a render problem (e1m1):
//int playerx = 13*256 + 109;
//int playery = 34*256 + 240;
//char playera = 10;
//char playerSector = 8;

char shells;
char bullets;
char weapons[3];
char weapon;
char armor;
char combatArmor;
signed char health = 0;

char endLevel;
char level;

#define TYPE_DOOR 1
#define TYPE_OBJECT 2
#define TYPE_SWITCH 3
char typeAtCenterOfView;
char itemAtCenterOfView;
unsigned char openDoors[4];
char doorOpenTime[4];
char numOpenDoors = 0;

char eraseMessageAfter = 0;

#define SCREENWIDTH 32
#define HALFSCREENWIDTH (SCREENWIDTH/2)
#define SCREENHEIGHT 64
#define HALFSCREENHEIGHT (SCREENHEIGHT/2)
#define PIXELSPERMETER 4
#define TEXWIDTH 16
#define TEXHEIGHT 32

#define EDGE_TYPE_MASK 0xC0
#define EDGE_PROP_MASK 0x38
#define EDGE_TEX_MASK  0x07
#define EDGE_TYPE_SHIFT 6
#define EDGE_PROP_SHIFT 3

#define EDGE_TYPE_NORMAL 0
#define EDGE_TYPE_DOOR (1<<6)
#define EDGE_TYPE_JAMB (2<<6)
#define EDGE_TYPE_SWITCH (3<<6)

#define SWITCH_TYPE_ENDLEVEL 0
#define SWITCH_TYPE_OPENDOOR 1
#define SWITCH_TYPE_REMOVEDOOR 2
#define SWITCH_TYPE_OPENDOORP 3

#define DOOR_TYPE_SHOT 4
#define DOOR_TYPE_ONEWAY 6

char frame = 0;

void __fastcall__ drawColumn(char textureIndex, char texI, signed char curX, short curY, unsigned char h);
void __fastcall__ drawColumnSameY(char textureIndex, char texI, signed char curX, short curY, unsigned char h);
void __fastcall__ drawColumnTransparent(char textureIndex, char texYStart, char texYEnd, char texI, signed char curX, short curY, unsigned char h);

void __fastcall__ drawWall(char sectorIndex, char curEdgeIndex, char nextEdgeIndex, signed char x_L, signed char x_R)
{
  char edgeGlobalIndex = getEdgeIndex(sectorIndex, curEdgeIndex);
  char textureIndex = getEdgeTexture(edgeGlobalIndex);
  char type = textureIndex & EDGE_TYPE_MASK;
  char prop = textureIndex & EDGE_PROP_MASK;
  char edgeLen = getEdgeLen(edgeGlobalIndex);

  // intersect the view direction and the edge
  // http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
  int x1 = getTransformedX(curEdgeIndex);
  int y1 = getTransformedY(curEdgeIndex);
  int dx = getTransformedX(nextEdgeIndex) - x1;
  int dy = getTransformedY(nextEdgeIndex) - y1;

  //int x3 = 0;
  //int y3 = 0;
  //int y4 = 256*1;

  signed char x4;
  int denom;
  signed char curX;
  int numer;
  unsigned int t;
  unsigned int texI;
  unsigned int curY;
  unsigned int h;
  char fit;
  char transp;

  automap_sawEdge(edgeGlobalIndex);

  textureIndex &= EDGE_TEX_MASK;
  // techwall, switch, door
  fit = (textureIndex == 2 || textureIndex == 5 || textureIndex == 6);
  transp = (textureIndex == 4);
  
  // add 128 to correct for sampling in the center of the column
  //x4 = (256*x_L + 128)/HALFSCREENWIDTH;
  x4 = 2*x_L + 1; // need to multiply by 8 later
  for (curX = x_L; curX < x_R; ++curX)
  {
    //x4 = (256*curX + 128)/HALFSCREENWIDTH;
    x4 += 2;
    if (testFilled(curX) == 0x7f)
    {
      // denom = dx - x4 * dy / 256;
      fastMultiplySetup16x8(x4);
      denom = dx - (fastMultiply16x8(dy)<<3); // here's the x8
      if (denom > 0)
      {
        // numer = x4 * ((long)y1) / 256 - x1;
        numer = (fastMultiply16x8(y1)<<3) - x1; // and x8 here
        //           if (numer > 0)
        {
          // t = 256 * numer / denom;
          t = div88(numer, denom);
        }
        //           else
        {
        //              t = 0;
        }
        if (t > 255) t = 255;
        // curY = y1 + t * dy / 256;
        fastMultiplySetup16x8(t>>1);
        curY = (fastMultiply16x8(dy)<<1) + y1;
        setFilled(curX, curY);
        if (curY > 0)
        {
          // perspective transform
          // Ys = Yw * (Ds/Dw) ; Ys = screenY, Yw = worldY, Ds = dist to screen, Dw = dist to point
          // h = (SCREENHEIGHT/16)*512/(curY/16);

          h = div88(128, curY);
               
          if (type == EDGE_TYPE_JAMB)
          {
            texI = prop >> EDGE_PROP_SHIFT;
            textureIndex = 7;
          }
          else if (!fit)
          {
            //texI = (t * edgeLen) >> 6; // 256/PIXELSPERMETER
            fastMultiplySetup8x8(t>>1);
            texI = fastMultiply8x8(edgeLen)>>5;
            texI &= 15; // 16 texel wide texture
          }
          else
          {
            // switch, door or techwall, so fit to wall
            texI = t >> 4;
          }
               
          if (curX == 0)
          {
            if (type == EDGE_TYPE_DOOR)
            {
              typeAtCenterOfView = TYPE_DOOR;
              itemAtCenterOfView = edgeGlobalIndex;
            }
            else if (type == EDGE_TYPE_SWITCH)
            {
              typeAtCenterOfView = TYPE_SWITCH;
              itemAtCenterOfView = edgeGlobalIndex;
            }
          }

          drawColumn(textureIndex, texI, curX, curY, h);
        }
      }
    }
  }
}

signed char __fastcall__ drawDoor(char sectorIndex, char curEdgeIndex, char nextEdgeIndex, signed char x_L, signed char x_R)
{
  char edgeGlobalIndex = getEdgeIndex(sectorIndex, curEdgeIndex);

  if (isDoorClosed(edgeGlobalIndex))
  {
    drawWall(sectorIndex, curEdgeIndex, nextEdgeIndex, x_L, x_R);
    return x_R;
  }
  return x_L;
}

unsigned char getWidthFromHeight(char ws, unsigned char h)
{
  unsigned char w;
  switch (ws)
  {
  case 2:
    w = h>>1;
    break;
  case 3:
    w = (h + (h>>2))>>2;
    break;
  case 4:
    w = h>>2;
    break;
  case 5:
    w = (h + (h>>1))>>3;
    break;
  case 8:
    w = h>>3;
    break;
  }
  return w;
}

char objO[8];
int objX[8];
int objY[8];

void __fastcall__ drawObjectInSector(char objIndex, signed char x_L, signed char x_R)
{
  // perspective transform (see elsewhere for optimization)
  //int h = (SCREENHEIGHT/16) * 512 / (vy/16);
  int vy = objY[objIndex];
  unsigned int h = div88(128, vy);
  unsigned char hc;

  char o = objO[objIndex];
  char objectType = getObjectType(o);
  char animate = 0;
  char textureIndex;
  unsigned char w;
  int sx;
  signed char leftX;
  signed char rightX;
  signed char startX;
  signed char endX;
  signed char curX;
  char texI;

  if (objectType < 5)
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
    textureIndex = texFrameTexture(objectType);
  }
  //w = h/texFrames[objectType].widthScale;
  if (h < 128)
  {
    hc = h;
  }
  else
  {
    hc = 127;
  }
  w = getWidthFromHeight(texFrameWidthScale(objectType), hc);
  if (w > 0)
  {
     //sx = vx / (vy / HALFSCREENWIDTH);
     int vx = objX[objIndex];
     sx = leftShift4ThenDiv(vx, vy);
     if (sx > -64 && sx < 64)
     {
       leftX = sx - w;
       rightX = sx + w;
       startX = leftX;
       endX = rightX;
       if (startX < -16) startX = -16;
       if (endX > 16) endX = 16;
       if (startX < x_R && endX > x_L)
       {
         char first = 1;
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
                if (first)
                {
                  first = 0;
                  drawColumn(textureIndex, texI, curX, vy, hc);
                }
                else
                {
                  drawColumnSameY(textureIndex, texI, curX, vy, hc);
                }
             }
          }
       }
     }
  }
}

char flashBorderTime = 0;

void __fastcall__ drawHudArmor(void)
{
    char armorColor = 5 + combatArmor;
    POKE(0x1000 + 22*21 + 13, 30); // armor symbol in font
    POKE(0x9400 + 22*21 + 13, armorColor);
    setTextColor(armorColor);
    print3DigitNumToScreen(armor, 0x1000 + 22*21 + 14);
}

char weaponSymbol[3] = { 38, 31, 34 };

void __fastcall__ drawHudAmmo(void)
{
  // weapon and ammo
  POKE(0x1000 + 22*21 + 1, weaponSymbol[weapon]);
  POKE(0x9400 + 22*21 + 1, 3);
  setTextColor(3);
  print2DigitNumToScreen(weapon == 1 ? shells : bullets, 0x1000 + 22*21 + 2);
}

void __fastcall__ drawHudHealth(void)
{
  // health
  POKE(0x1000 + 22*21 + 5, '/');
  POKE(0x9400 + 22*21 + 5, 2);
  setTextColor(2);
  print3DigitNumToScreen(health, 0x1000 + 22*21 + 6);
}

char *keyCardNames[3] = { " red", "green", "blue" };

extern char difficulty;

char godMode = 0;

void __fastcall__ damagePlayer(char damage)
{
  if (!godMode)
  {
    if (difficulty == 0)
    {
      // approximation for /3
      damage = damage/4 + damage/16;
    }
    else if (difficulty == 1)
    {
      damage = damage/2;
    }
    if (armor > 0)
    {
      char armorDamage = 0;
      if (combatArmor == 1)
      {
        armorDamage = damage/2;
      }
      else
      {
        // approximation for /3
        armorDamage = damage/4 + damage/16;
      }
      damage = damage - armorDamage;
      armor -= armorDamage;
      if (armor > 200)
      {
        armor = 0;
        combatArmor = 0;
      }
      drawHudArmor();
    }
    health -= damage;
    if (health < 0)
    {
       health = 0;
    }
    drawHudHealth();
  
    // flash border red
    flashBorderTime = 1;
    POKE(0x900F, 8+2);
  }
}

char numItemsGot;

char __fastcall__ getItemPercentage(void)
{
  return (100*numItemsGot)/getNumItems();
}

signed char barrelAtCenterOfScreen = -1;

void processTransparentObjectAtCenterOfScreen(char o)
{
  char objectType = getObjectType(o);

  if (objectType == kOT_Barrel)
  {
    barrelAtCenterOfScreen = o;
  }
}

char transO[12];
int transX[12];
int transY[12];
signed char transSXL[12];
signed char transSXR[12];

void __fastcall__ drawTransparentObject(char transIndex)
{
  // perspective transform (see elsewhere for optimization)
  //int h = (SCREENHEIGHT/16) * 512 / (vy/16);
  int vy = transY[transIndex];
  unsigned int h = div88(128, vy);
  unsigned char hc;
  char o = transO[transIndex];
  char objectType = getObjectType(o);
  char textureIndex;
  int sx;
  signed char leftX;
  signed char rightX;
  signed char startX;
  signed char endX;
  signed char curX;
  char texI;
  char startY, height;
  unsigned char w;

  //w = h/texFrames[objectType].widthScale;
  if (h < 128)
  {
    hc = h;
  }
  else
  {
    hc = 127;
  }
  w = getWidthFromHeight(texFrameWidthScale(objectType), hc);
  if (w > 0)
  {
     //sx = vx / (vy / HALFSCREENWIDTH);
     int vx = transX[transIndex];
     sx = leftShift4ThenDiv(vx, vy);
     if (sx > -64 && sx < 64)
     {
       leftX = sx - w;
       rightX = sx + w;
       startX = leftX;
       endX = rightX;
       if (startX < -16) startX = -16;
       if (endX > 16) endX = 16;
       if (startX < transSXR[transIndex] && endX > transSXL[transIndex])
       {
          textureIndex = texFrameTexture(objectType);
          startY = texFrameStartY(objectType);
          height = texFrameHeight(objectType);
          for (curX = startX; curX < endX; ++curX)
          {
             if (testFilledWithY(curX, vy) > 0)
             {
               if (curX == 0)
               {
                 processTransparentObjectAtCenterOfScreen(o);
               }
                // compensate for pixel samples being mid column
                //texI = TEXWIDTH * (2*(curX - leftX) + 1) / (4 * w);
                texI = getObjectTexIndex(w, curX - leftX);
                if (texFrameWidth(objectType) != 16)
                {
                   texI = texFrameStartX(objectType) + (texI>>1);
                }
                drawColumnTransparent(textureIndex, startY, height, texI, curX, vy, hc);
             }
          }
        }
     }
  }
}

char acidBurn = 0;
void __fastcall__ updateAcid(void)
{
  char o, d;
  int dx, dy;

  if (acidBurn > 0)
  {
    --acidBurn;
  }
  if (acidBurn == 0)
  {
    acidBurn = 4;
    for (o = getFirstObjectInSector(playerSector); o != 0xff; o = getNextObjectInSector(o))
    {
      if (getObjectType(o) == kOT_Acid)
      {
        dx = getObjectX(o) - playerx;
        dy = getObjectY(o) - playery;

        d = P_ApproxDistance(dx, dy);

        if (d < 3)
        {
          playSound(SOUND_OOF);
          damagePlayer(1 + (P_Random()&7));
          break;
        }
      }
    }
  }
}

char sorted[8];
char numSorted;
char numTransparent;

void __fastcall__ drawObjectsInSector(char sectorIndex, signed char x_L, signed char x_R)
{
  int vx, vy;
  char o, i, j;
  numSorted = 0;
  
  // loop through the objects
  for (o = getFirstObjectInSector(sectorIndex); o != 0xff; o = getNextObjectInSector(o))
  {
    // inverse transform
    vy = transformxy(getObjectX(o), getObjectY(o));
    
    if (vy > 256)
    {
       vx = transformx();
       sorted[numSorted] = numSorted;

       objO[numSorted] = o;
       objX[numSorted] = vx;
       objY[numSorted] = vy;

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
            if (objY[sorted[i]] > objY[sorted[j]])
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
         char index;
         index = sorted[i];
         type = getObjectType(objO[index]);
         if (texFrameSolid(type))
         {
           drawObjectInSector(index, x_L, x_R);
           p_enemy_add_thinker(objO[index]);
         }
      }
    }
}

void __fastcall__ queueTransparentObjects(signed char x_L, signed char x_R)
{
  char i, type;
  for (i = 0; i < numSorted; ++i)
  {
    char objIndex = sorted[i];
    type = getObjectType(objO[objIndex]);
    if (!texFrameSolid(type))
    {
      transO[numTransparent] = objO[objIndex];
      transX[numTransparent] = objX[objIndex];
      transY[numTransparent] = objY[objIndex];
      transSXL[numTransparent] = x_L;
      transSXR[numTransparent] = x_R;
      ++numTransparent;
    }
  }
}

void __fastcall__ drawTransparentObjects(void)
{
  signed char i;
  barrelAtCenterOfScreen = -1;
  // draw back to front
  for (i = numTransparent-1; i >= 0; --i)
  {
    drawTransparentObject(i);
  }
}

// find first edge in sector
signed char __fastcall__ ffeis(char curSec, signed char x_L, signed char x_R)
{
   char i, numVerts;
   numVerts = getNumVerts(curSec);
   for (i = 0; i < numVerts; ++i)
   {
     signed char sx1, sx2;
      int ty1, ty2;
      char ni;
      ni = (i + 1);
      if (ni == numVerts) ni = 0;
      sx1 = getScreenX(i);
      sx2 = getScreenX(ni);
      ty1 = getTransformedY(i);
      ty2 = getTransformedY(ni);
      // preprocess
      if (curSec == playerSector)
      {
        // when inside the sector, adjust the edges clipping the camera plane
        // so that they are definitely facing the player
        if (ty1 <= 0 && ty2 > 0) sx1 = x_L;
        if (ty1 > 0 && ty2 <= 0) sx2 = x_R;
        if (sx1 <= x_L && sx2 > x_L)
        {
          return i;
        }
      }
      else
      {
        char firstVertex = getVertexIndex(curSec, i);
        if (sx1 <= x_L && sx2 > x_L && (ty1 >= 0 || ty2 >= 0))
        {
          return i;
        }
      }
   }
   return -1;
}

void __fastcall__ drawSpans(void)
{
  signed char stackTop;
  char sectorIndex;
  signed char x_L, x_R;
  signed char firstEdge;
  char curEdge;
  char edgeGlobalIndex;
  signed char curX;
  char nextEdge;
  signed char nextX;
  signed char thatSector;

  clearFilled();
  numTransparent = 0;
  typeAtCenterOfView = 0;

  stackTop = 0;
  spanStackSec[0] = playerSector;
  spanStackL[0] = -HALFSCREENWIDTH;
  spanStackR[0] = HALFSCREENWIDTH;

  preTransformSectors();

  while (stackTop >= 0)
  {
     sectorIndex = spanStackSec[stackTop];
     x_L = spanStackL[stackTop];
     x_R = spanStackR[stackTop];
     --stackTop;

     // STEP 1 - draw objects belonging to this sector!
     // fill in the table of written columns as we progress
     //POKE(0x900f, 11);
     drawObjectsInSector(sectorIndex, x_L, x_R);
     //POKE(0x900f, 13);

     //POKE(0x900f, 11);
     transformSectorToScreenSpace(sectorIndex);
     //POKE(0x900f, 13);

     firstEdge = ffeis(sectorIndex, x_L, x_R);
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

        edgeGlobalIndex = getEdgeIndex(sectorIndex, curEdge);
        thatSector = getOtherSector(edgeGlobalIndex, sectorIndex);
        if (thatSector != -1)
        {
           if (isEdgeDoor(edgeGlobalIndex))
           {
              curX = drawDoor(sectorIndex, curEdge, nextEdge, curX, nextX);
           }
           if (curX < nextX)
           {
               // come back to this
               if (stackTop < 10)
               {
                 ++stackTop;
                 spanStackSec[stackTop] = thatSector;
                 spanStackL[stackTop] = curX;
                 spanStackR[stackTop] = nextX;
               }
               else
               {
                 print2DigitNumToScreen(thatSector, 0x1000 + 5*22);
               }
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

void __fastcall__ openDoor(char edgeGlobalIndex)
{
    char i;
    if (isDoorClosed(edgeGlobalIndex))
    {
        for (i = 0; i < 4; ++i)
        {
          if (doorOpenTime[i] == 0)
          {
              openDoors[i] = edgeGlobalIndex;
              doorOpenTime[i] = 30;
              break;
          }
        }
        basicOpenDoor(edgeGlobalIndex);
        playSound(SOUND_DOROPN);
    }
}

void __fastcall__ doEdgeSpecial(char edgeGlobalIndex)
{
    char textureIndex = getEdgeTexture(edgeGlobalIndex);
    char type = textureIndex & EDGE_TYPE_MASK;
    if (type == EDGE_TYPE_SWITCH)
    {
      char prop = (textureIndex & EDGE_PROP_MASK) >> EDGE_PROP_SHIFT;
      if (prop == SWITCH_TYPE_ENDLEVEL)
      {
        playSound(SOUND_PISTOL);
        endLevel = 1;
      }
      else if (prop == SWITCH_TYPE_OPENDOOR)
      {
        // arranged so that the door to open is the next edge in the global list
        openDoor(edgeGlobalIndex + 1);
      }
      else if (prop == SWITCH_TYPE_OPENDOORP)
      {
        openDoor(edgeGlobalIndex - 1);
      }
      else if (prop == SWITCH_TYPE_REMOVEDOOR)
      {
        ++edgeGlobalIndex;
        if (isDoorClosed(edgeGlobalIndex))
        {
          playSound(SOUND_DOROPN);
          basicOpenDoor(edgeGlobalIndex);
        }
      }
    }
}

// THIS IS THE NEXT TARGET OF FIXING & OPTIMIZATION!

signed char curSector;
signed char thatSector;
signed char nextSector;

char ni;
signed char v1x, v1y, v2x, v2y, ex, ey;
long px, py;
long height;
long edgeLen;
long edgeLen2;
long dist;
long distanceToPush;
char edgeGlobalIndex;
char vertGlobalIndex, vert2GlobalIndex;
char dgz, dle;
char reversedEdge;

typedef enum
{
  kPOR_Nada,
  kPOR_Wall,
  kPOR_Sector
}
EPushOutResult;

char totalCheckedEdges;

#define INNERCOLLISIONRADIUS 512
#define OUTERCOLLISIONRADIUS 528
#define COLLISIONDELTA (OUTERCOLLISIONRADIUS - INNERCOLLISIONRADIUS)

void breakpoint(void)
{
  // completely free of side effects :)
  POKE(0x9700, 1);
}

EPushOutResult __fastcall__ push_out_from_edge(char i)
{
  ++totalCheckedEdges;

  ni = getNextEdge(curSector, i);
  vertGlobalIndex = getVertexIndex(curSector, i);
  vert2GlobalIndex = getVertexIndex(curSector, ni);

  reversedEdge = 0;
  if (vertGlobalIndex > vert2GlobalIndex)
  {
    edgeGlobalIndex = vertGlobalIndex;
    vertGlobalIndex = vert2GlobalIndex;
    vert2GlobalIndex = edgeGlobalIndex;
    reversedEdge = 1;
  }

  v1x = getVertexX(vertGlobalIndex);
  v1y = getVertexY(vertGlobalIndex);
  v2x = getVertexX(vert2GlobalIndex);
  v2y = getVertexY(vert2GlobalIndex);
  ex = v2x - v1x;
  ey = v2y - v1y;
  px = playerx - (((short)v1x)<<8);
  py = playery - (((short)v1y)<<8);
  // need to precalc 65536/edge.len
  edgeGlobalIndex = getEdgeIndex(curSector, i);

  // get edge len^2 (16.0)
  fastMultiplySetup8x8(ex);
  edgeLen2 = fastMultiply8x8(ex);
  fastMultiplySetup8x8(ey);
  edgeLen2 += fastMultiply8x8(ey);
  // make it 16.8
  edgeLen2 <<= 8;

  // edgeLen is 8.4
  edgeLen = sqrt24(edgeLen2);
  // make it 8.8
  edgeLen <<= 4;

  //edgeLen = getEdgeLen(edgeGlobalIndex);

  //height = px * ey - py * ex;
  {
    long p1, p2;
    fastMultiplySetup16x8e24(ey);
    p1 = fastMultiply16x8e24(px);
    fastMultiplySetup16x8e24(ex);
    p2 = fastMultiply16x8e24(py);
    if (reversedEdge)
    {
      height = p2 - p1;
    }
    else
    {
      height = p1 - p2;
    }
  }
  // height is 16.8

  // height < INNERCOLLISIONRADIUS*edgeLen
  if (height <= 0 || height < (edgeLen<<1))
  {
    // check we're within the extents of the edge
    //dist = px * ex + py * ey;
    {
      long p1, p2;
      p2 = fastMultiply16x8e24(px);
      fastMultiplySetup16x8e24(ey);
      p1 = fastMultiply16x8e24(py);
      dist = p1 + p2;
    }
    dgz = (dist >= 0);
    dle = (dist < edgeLen2);
#if 0
    {
      long len2 = 0;
      fastMultiplySetup8x8(edgeLen);
      len2 = ((long)fastMultiply8x8(edgeLen))<<8;
      dle = dist < len2;
    }
#endif
    if (dgz & dle)
    {
      thatSector = getOtherSector(edgeGlobalIndex, curSector);
      if (thatSector != -1 && !isDoorClosed(edgeGlobalIndex))
      {
        if (height < 0)
        {
          nextSector = thatSector;
                 
          // crossed a line, so check for special
          doEdgeSpecial(edgeGlobalIndex);
        }
        return kPOR_Sector;
      }
      else
      {
        // try just pushing out
        //distanceToPush = OUTERCOLLISIONRADIUS*edgeLen - height;
        // distanceToPush is X.8
        distanceToPush = (edgeLen<<1) + (edgeLen>>4) - height;
        //fastMultiplySetup16x8e24(edgeLen);
        //distanceToPush = fastMultiply16x8e24(OUTERCOLLISIONRADIUS) - height;
        if (reversedEdge)
        {
          ex = -ex;
          ey = -ey;
        }
        //playerx += distanceToPush * ey / (edgeLen*edgeLen);
        //playery -= distanceToPush * ex / (edgeLen*edgeLen);
        {
          //int edgeLen2;
          //fastMultiplySetup8x8(edgeLen);
          //edgeLen2 = fastMultiply8x8(edgeLen);
          // need a X.8 value on the right
          edgeLen2 >>= 8;
          fastMultiplySetup16x8e24(ey);
          playerx += fastMultiply16x8e24(distanceToPush) / edgeLen2;
          fastMultiplySetup16x8e24(ex);
          playery -= fastMultiply16x8e24(distanceToPush) / edgeLen2;
        }
        return kPOR_Wall;
      }
    }
    else
    {
      char checkVert = 0;
      // if (!dgz && (dist > -INNERCOLLISIONRADIUS*edgeLen))
      //if (!dgz && ((dist>>9) > -edgeLen))
      if (!dgz && dist > -(edgeLen<<1))
      {
        checkVert = 1;
      }
      else
      {
        if (!dle)
        {
          //if (dist < (256*edgeLen + INNERCOLLISIONRADIUS)*edgeLen))
          // if (dist < (edgeLen+2)*edgeLen)
          long lim = edgeLen2 + (edgeLen<<1);
          //int paddedEdgeLen = ((int)(edgeLen+2))<<8;
          //fastMultiplySetup16x8e24(edgeLen);
          //if (dist < fastMultiply16x8e24(paddedEdgeLen))
          if (dist < lim)
          {
            checkVert = 1;
            // check the far end
            px = playerx - (((int)v2x)<<8);
            py = playery - (((int)v2y)<<8);
          }
        }
      }
      if (checkVert)
      {
        height = px * px + py * py;
        //if (height < INNERCOLLISIONRADIUS*INNERCOLLISIONRADIUS)
        if ((height&0xfffc0000) == 0)
        {
          height = sqrt24(height);
          distanceToPush = OUTERCOLLISIONRADIUS - height;
          playerx += distanceToPush * px / height;
          playery += distanceToPush * py / height;
          return kPOR_Wall;
        }
      }
    }
  }
  return kPOR_Nada;
}

int getPlayerX(void)
{
  return playerx;
}

int getPlayerY(void)
{
  return playery;
}

char getCurSector(void)
{
  return curSector;
}

char __fastcall__ playerOverlapsEdge(char i);

char oldPlayerInFrontOfEdge(char i)
{
  signed char ppx, ppy;
  int pxey, pyex;
  ni = getNextEdge(curSector, i);
  vertGlobalIndex = getVertexIndex(curSector, i);
  vert2GlobalIndex = getVertexIndex(curSector, ni);
  v1x = getVertexX(vertGlobalIndex);
  v1y = getVertexY(vertGlobalIndex);
  ex = getVertexX(vert2GlobalIndex) - v1x;
  ey = getVertexY(vert2GlobalIndex) - v1y;

  ppx = ((playeroldx + 127)>>8) - v1x;
  ppy = ((playeroldy + 127)>>8) - v1y;

  //height = px * ey - py * ex;
  fastMultiplySetup8x8(ppx);
  pxey = fastMultiply8x8(ey);
  fastMultiplySetup8x8(ppy);
  pyex = fastMultiply8x8(ex);

  return pxey >= pyex;
}

char sectorsToCheck[8];
char numSectorsToCheck;

char sectorStack[8];
char sectorStackTop;

char wallsToCheck_Sector[16];
char wallsToCheck_Edge[16];
char wallsToCheck_Checked[16];
char numWallsToCheck;

char crossablesToCheck_Sector[8];
char crossablesToCheck_Edge[8];
char numCrossablesToCheck;

void push_out(void)
{
  char h, i, j, k, secNumVerts;
  EPushOutResult r;
  char numPushedOutFrom = 0;
  // here's the new plan
  // collect up the edges we're overlapping (bbox) (and that are facing the player's old position)
  // push out in turn from each
  // once we push out from one, remove it from the list and recheck the rest
  // then check the crossable edges

  totalCheckedEdges = 0;

  // collect edges
  sectorsToCheck[0] = playerSector;
  numSectorsToCheck = 1;

  sectorStack[0] = playerSector;
  sectorStackTop = 1;

  numWallsToCheck = 0;
  numCrossablesToCheck = 0;

  h = 1;
  while (sectorStackTop != 0)
  {
    h = 0;
    --sectorStackTop;
    curSector = sectorStack[sectorStackTop];
    secNumVerts = getNumVerts(curSector);

    for (i = 0; i < secNumVerts; ++i)
    {
      edgeGlobalIndex = getEdgeIndex(curSector, i);
      thatSector = getOtherSector(edgeGlobalIndex, curSector);
      if (thatSector != -1 && !isDoorClosed(edgeGlobalIndex))
      {
        k = 0;
        for (j = 0; j < numSectorsToCheck; ++j)
        {
          if (thatSector == sectorsToCheck[j])
          {
            k = 1;
            break;
          }
        }
        if (k == 0)
        {
          if (playerOverlapsEdge(i))
          {
            sectorStack[sectorStackTop] = thatSector;
            ++sectorStackTop;
            sectorsToCheck[numSectorsToCheck] = thatSector;
            ++numSectorsToCheck;
            crossablesToCheck_Sector[numCrossablesToCheck] = curSector;
            crossablesToCheck_Edge[numCrossablesToCheck] = i;
            ++numCrossablesToCheck;
            h = 1;
          }
        }
      }
      else
      {
        if (playerOverlapsEdge(i))
        {
          if (oldPlayerInFrontOfEdge(i))
          {
            wallsToCheck_Sector[numWallsToCheck] = curSector;
            wallsToCheck_Edge[numWallsToCheck] = i;
            wallsToCheck_Checked[numWallsToCheck] = 0;
            ++numWallsToCheck;
          }
        }
      }
    }
  }

  h = 1;
  while (h != 0)
  {
    h = 0;
    for (i = 0; i < numWallsToCheck; ++i)
    {
      if (!wallsToCheck_Checked[i])
      {
        curSector = wallsToCheck_Sector[i];
        r = push_out_from_edge(wallsToCheck_Edge[i]);
        if (r == kPOR_Wall)
        {
          wallsToCheck_Checked[i] = 1;
          ++numPushedOutFrom;
          h = 1;
          break;
        }
      }
    }
  }

  // then check crossables
  for (i = 0; i < numCrossablesToCheck; ++i)
  {
    curSector = crossablesToCheck_Sector[i];
    nextSector = -1;
    r = push_out_from_edge(crossablesToCheck_Edge[i]);
    if (nextSector != -1)
    {
      playerSector = nextSector;
    }
  }

//  print3DigitNumToScreen(playerSector, 0x1000 + 66);
//  print3DigitNumToScreen(totalCheckedEdges, 0x1000 + 88);
//  print3DigitNumToScreen(numSectorsToCheck, 0x1000 + 110);
//  print3DigitNumToScreen(numWallsToCheck, 0x1000 + 132);
//  print3DigitNumToScreen(numPushedOutFrom, 0x1000 + 155);
//  print3DigitNumToScreen(numCrossablesToCheck, 0x1000 + 176);
}


signed char explodingBarrelsObject[4];
char explodingBarrelsTime[4];

void __fastcall__ addExplodingBarrel(char o)
{
  char i;
  setObjectType(o, kOT_ExplodingBarrel);
  for (i = 0; i < 4; ++i)
  {
    if (explodingBarrelsObject[i] == -1)
    {
      explodingBarrelsObject[i] = o;
      explodingBarrelsTime[i] = 3;
      break;
    }
  }
  playSound(SOUND_SHOTGN);
}

void updateBarrels(void)
{
  char i, t, k, d;
  signed char o;
  int dx, dy;
  for (i = 0; i < 4; ++i)
  {
    o = explodingBarrelsObject[i];
    if (o != -1)
    {
      --explodingBarrelsTime[i];
      t = explodingBarrelsTime[i];
      if (t == 2)
      {
        // damage stuff around
        for (k = 0; k < getNumObjects(); ++k)
        {
          if (getObjectSector(k) != 255)
          {
            char ot = getObjectType(k);
            if (ot < 5 || ot == kOT_Barrel)
            {
              dx = getObjectX(k) - getObjectX(o);
              dy = getObjectY(k) - getObjectY(o);

              d = P_ApproxDistance(dx, dy);

              if (d < 7)
              {
                if (ot == kOT_Barrel)
                {
                  addExplodingBarrel(k);
                }
                else
                {
                  p_enemy_damage(k, 30);
                }
              }
            }
          }
        }

        // also damage player
        dx = getObjectX(o) - playerx;
        dy = getObjectY(o) - playery;

        d = P_ApproxDistance(dx, dy);

        // be more lenient for the player
        if (d < 5)
        {
          damagePlayer(30);
        }
      }
      else if (t == 0)
      {
        setObjectSector(o, -1);
        explodingBarrelsObject[i] = -1;
      }
    }
  }
}

void preparePickupMessage(void)
{
  playSound(SOUND_ITEMUP);
  POKE(0x900F, 8 + 3); // cyan border
  flashBorderTime = 1;
  eraseMessage();
  textcolor(7);
  eraseMessageAfter = 8;
}

void __fastcall__ checkSectorForPickups(char sec)
{
  char o, objectType, d;
  int dx, dy;
  for (o = getFirstObjectInSector(sec); o != 0xff; o = getNextObjectInSector(o))
  {
    objectType = getObjectType(o);
    if (isPickup(objectType))
    {
      dx = getObjectX(o) - playerx;
      dy = getObjectY(o) - playery;

      d = P_ApproxDistance(dx, dy);

      if (d < 3)
      {
        char pickupType = objectType;
        char remove = 1;
        if (objectType == kOT_PossessedCorpseWithAmmo)
        {
          setObjectType(o, kOT_PossessedCorpse);
          remove = 0;
          pickupType = kOT_Bullets;
        }
        if (objectType == kOT_Shotgun && weapons[1])
        {
          pickupType = kOT_Bullets;
        }
        if (objectType == kOT_Chaingun && weapons[2])
        {
          pickupType = kOT_Bullets;
        }

        {
          char pickedUp = 0;

          switch (pickupType)
          {
          case kOT_GreenArmor:
            if (armor < 100)
            {
              armor = 100;
              combatArmor = 0;
              drawHudArmor();
              pickedUp = 1;
            }
            break;
          case kOT_BlueArmor:
            if (armor < 200)
            {
              armor = 200;
              combatArmor = 1;
              drawHudArmor();
              pickedUp = 1;
            }
            break;
          case kOT_Bullets:
            if (weapons[1])
            {
              if (shells < 50 && (P_Random()&64))
              {
                shells += 4;
                if (shells > 50) shells = 50;
                drawHudAmmo();
                pickedUp = 1;
              }
            }
            if (!pickedUp && bullets < 200)
            {
              bullets += 10;
              if (bullets > 200) bullets = 200;
              drawHudAmmo();
              pickedUp = 1;
            }
            break;
          case kOT_Medkit:
            if (health < 100)
            {
              health += 25;
              if (health > 100) health = 100;
              drawHudHealth();
              pickedUp = 1;
            }  
            break;
          case kOT_RedKeycard:
            addKeyCard(2);
            pickedUp = 1;
            break;
          case kOT_GreenKeycard:
            addKeyCard(4);
            pickedUp = 1;
            break;
          case kOT_BlueKeycard:
            addKeyCard(8);
            pickedUp = 1;
            break;
          case kOT_Shotgun:
            weapons[1] = 1;
            pickedUp = 1;
            break;
          case kOT_Chaingun:
            weapons[2] = 1;
            pickedUp = 1;
            break;
          }
          if (pickedUp)
          {
            preparePickupMessage();
            printCentered("you got the", 14);
            printCentered(pickupNames[pickupType - kOT_GreenArmor], 15);

            if (remove)
            {
              setObjectSector(o, -1);
              ++numItemsGot;
            }
            break;
          }
        }
      }
    }
  }
}

void checkForPickups(void)
{
  char i, secNumVerts;
  // just check this sector and its neighbours
  checkSectorForPickups(playerSector);
  secNumVerts = getNumVerts(playerSector);

  for (i = 0; i < secNumVerts; ++i)
  {
    char edgeGlobalIndex = getEdgeIndex(curSector, i);
    char thatSector = getOtherSector(edgeGlobalIndex, curSector);
    if (thatSector != 0xff && !isDoorClosed(edgeGlobalIndex))
    {
      checkSectorForPickups(thatSector);
    }
  }
}

char turnLeftSpeed = 0;
char turnRightSpeed = 0;
char shotgunStage = 0;
char pistolStage = 0;

#if 0
char changeLookTime = 7;
char lookDir = 0;
#endif

char soundToPlay = 0;

void __fastcall__ setUpScreenForBitmap(void)
{
  clearScreen();
  setupBitmap(8 + 2); // multicolor red
}

void __fastcall__ setUpScreenForMenu(void)
{
  drawBorders(32);
}

void __fastcall__ setUpScreenForGameplay(void)
{
  clearMenuArea();
  setupBitmap(8 + 2); // multicolor red
  POKE(0x900F, 8 + 5); // green border, and black screen
  drawBorders(29);
  // name of level
  textcolor(2);
  printCentered(getMapName(), 18);
  playMapTimer();

  drawHudAmmo();
  drawHudArmor();
  drawHudHealth();
  addKeyCard(1);
  // face
  colorFace(0);
  drawFace();
  
}

signed char updateCheatCodes(void);
char *cheatText[] =
{
  "god mode",
  "keys, full ammo",
  "change level",
  "reveal map",
};

void handleCheatCodes(void)
{
  signed char i = updateCheatCodes();
  if (i != -1)
  {
    preparePickupMessage();
    printCentered(cheatText[i], 15);
    if (i == 0)
    {
      health = 100;
      drawHudHealth();
      godMode = 1-godMode;
      colorFace(godMode);
    }
    else if (i == 1)
    {
      addKeyCard(2+4+8);
      weapons[0] = 1;
      weapons[1] = 1;
      weapons[2] = 1;
      bullets = 200;
      shells = 50;
      drawHudAmmo();
      armor = 200;
      drawHudArmor();
    }
    else if (i == 2)
    {
      endLevel = 1;
    }
    else if (i == 3)
    {
      automap_setEdges();
    }
  }
}

char __fastcall__ runMenu(char canReturn);

char caLevel[] = "pe1m1";
char caMusic[] = "pe1m1mus";

char *caLevelNames[10] =
{
  "",
  "hangar",
  "nuclear plant",
  "toxin refinery",
  "command control",
  "phobos lab",
  "central processing",
  "computer station",
  "phobos anomaly",
  "military base"
};

int main()
{
  char keys;
  char ctrlKeys;
  char i;
  int ca, sa;
  char numObj;

  // needed for clearScreen
  load_data_file("pmidcode");

  // clear screen
  clearScreen();
  cputsxy(0, 1, "R_Init: Init DOOM");
  cputsxy(0, 2, "refresh daemon...");

  load_data_file("psounds");
  load_data_file("plowcode");
  load_data_file("phicode");

  playSoundInitialize();

  generateMulTab();
  load_data_file("psluts");
  load_data_file("ptextures");

start:
  playMusic("pe1m9mus");

  POKE(0x900E, (6<<4) | (PEEK(0x900E)&0x0f)); // blue aux color
  
  // set the character set to $1400
  POKE(0x9005, 13 | (PEEK(0x9005)&0xf0));

  setUpScreenForBitmap();
  setUpScreenForMenu();

  runMenu(0);
  level = 1;
  godMode = 0;

nextLevel:

  POKE(0x900F, 8 + 5); // green border, and black screen

  {
    char p = '0' + level;
    caMusic[4] = p;
    caLevel[4] = p;
  }
  textcolor(2);
  printCentered("entering", 8);
  textcolor(1);
  printCentered(caLevelNames[level], 10);
  load_data_file(caLevel);
  playMusic(caMusic);

  if (health == 0)
  {
    health = 100;
    armor = 0;
    combatArmor = 0;
    bullets = 50;
    shells = 20;
    weapons[0] = 1;
    weapons[1] = 0;
    weapons[2] = 0;
    weapon = 0;
  }
  resetKeyCard();

  setUpScreenForBitmap();
  setUpScreenForGameplay();

  numObj = getNumObjects();

  resetDoorClosedAmounts();
  doorOpenTime[0] = 0;
  doorOpenTime[1] = 0;
  doorOpenTime[2] = 0;
  doorOpenTime[3] = 0;
  
  p_enemy_resetMap();
  automap_resetEdges();
  for (i = 0; i < numObj; ++i)
  {
    if (getObjectType(i) < 5)
    {
      allocMobj(i);
    }  
  }
  
  addObjectsToSectors();
  
  resetSectorsVisited();
  
  numItemsGot = 0;
  playerx = getPlayerSpawnX();
  playery = getPlayerSpawnY();
  playeroldx = playerx;
  playeroldy = playery;
  playera = getPlayerSpawnAngle();
  playerSector = getPlayerSpawnSector();
  endLevel = 0;
  barrelAtCenterOfScreen = -1;
  explodingBarrelsObject[0] = -1;
  explodingBarrelsObject[1] = -1;
  explodingBarrelsObject[2] = -1;
  explodingBarrelsObject[3] = -1;

  playMapTimer();
  resetMapTime();

  while (health != 0 && !endLevel)
  {
      if (!flashBorderTime)
      {
         // note: XXXXYZZZ (X = screen, Y = reverse mode, Z = border)
          POKE(0x900F, 8 + 5); // green border, and black screen
      }
      if (flashBorderTime > 0)
      {
        --flashBorderTime;
      }

      updateBarrels();
      checkForPickups();

      keys = readInput();
      ctrlKeys = getControlKeys();
      
      if (ctrlKeys & KEY_ESC)
      {
        pauseMapTimer();
        setUpScreenForMenu();
        if (runMenu(1) == 1)
        {
          // reset
          level = 1;
          health = 0;
          goto nextLevel;
        }
        setUpScreenForGameplay();
      }
      else if (ctrlKeys & KEY_CTRL)
      {
        pauseMapTimer();
        automap();
        setUpScreenForGameplay();
      }

      if (keys & KEY_TURNLEFT)
      {
        turnRightSpeed = 0;
        if (turnLeftSpeed < 3)
        {
            ++turnLeftSpeed;
        }
        playera -= turnLeftSpeed;
      }
      else if (keys & KEY_TURNRIGHT)
      {
        turnLeftSpeed = 0;
        if (turnRightSpeed < 3)
        {
            ++turnRightSpeed;
        }
        playera += turnRightSpeed;
      }
      else
      {
        turnLeftSpeed = 0;
        turnRightSpeed = 0;
      }
      playera &= 63;
      setCameraAngle(playera);
      ca = ((int)get_cos())<<1;
      sa = ((int)get_sin())<<1;
      playeroldx = playerx;
      playeroldy = playery;
      if (keys & KEY_MOVELEFT)
      {
        playerx -= ca;
        playery += sa;
      }
      if (keys & KEY_MOVERIGHT)
      {
        playerx += ca;
        playery -= sa;
      }

      if (keys & KEY_FORWARD)
      {
        if (!(testFilled(0) < 4 && typeAtCenterOfView == TYPE_OBJECT))
        {
          playerx += (sa<<1);
          playery += (ca<<1);
        }
      }
      if (keys & KEY_BACK)
      {
        playerx -= sa;
        playery -= ca;
      }

      if (shotgunStage != 0)
      {
        --shotgunStage;
        if (shotgunStage == 3)
        {
          playSound(SOUND_SGCOCK);
        }
      }
      if (pistolStage != 0)
      {
        --pistolStage;
      }
      if (keys & KEY_FIRE)
      {
        // pressed fire
        // take the high nybble, because it's more random
        char damage = 0;
        if (weapon == 1)
        {
          if (shells > 0 && shotgunStage == 0)
          {
            --shells;
            shotgunStage = 7;

            damage = P_Random()>>4;
            if (difficulty == 0)
            {
              damage = (damage&1) + 10;
            }
            else if (difficulty == 1)
            {
              damage = (damage&3) + 8;
            }
            else
            {
              damage = (damage&7) + 4;
            }
            // shotgun: if close, boost damage
            if (testFilled(0) < 4)
            {
              damage += 4;
            }
          }
        }
        else
        {
          if (bullets > 0 && pistolStage == 0)
          {
            --bullets;
            damage = P_Random()>>4;

            if (weapon == 0)
            {
              pistolStage = 3;
              if (difficulty == 0)
              {
                damage = 4;
              }
              else if (difficulty == 1)
              {
                damage = (damage&1) + 3;
              }
              else
              {
                damage = (damage&3) + 1;
              }
            }
            else
            {
              if (difficulty == 0)
              {
                damage = 2;
              }
              else
              {
                damage = 1 + (damage&1);
              }
            }
          }
        }

        if (damage != 0)
        {
          playSound(SOUND_PISTOL);

          drawHudAmmo();
          POKE(0x900F, 8+1);
          
          if (barrelAtCenterOfScreen != -1)
          {
            addExplodingBarrel(barrelAtCenterOfScreen);
          }
          else if (typeAtCenterOfView == TYPE_OBJECT)
          {
            p_enemy_damage(itemAtCenterOfView, damage);
          }
          else if (typeAtCenterOfView == TYPE_DOOR)
          {
            char tex = getEdgeTexture(itemAtCenterOfView);
            char prop = (tex & EDGE_PROP_MASK) >> EDGE_PROP_SHIFT;
            if (prop == DOOR_TYPE_SHOT)
            {
              openDoor(itemAtCenterOfView);
            }
          }
        }
      }

      for (i = 0; i < 4; ++i)
      {
        char dot = doorOpenTime[i];
        if (dot > 0)
        {
          --dot;
          doorOpenTime[i] = dot;
          if (dot == 0)
          {
            // try to close the door - should just get pushed out, so go ahead
            basicCloseDoor(openDoors[i]);
            playSound(SOUND_DORCLS);
          }
        }
      }
          
      if (keys & KEY_USE)
      {
        // tried to open a door (pressed K)
        if (testFilled(0) < 4)
        {
            if (typeAtCenterOfView == TYPE_DOOR)
            {
              char tex = getEdgeTexture(itemAtCenterOfView);
              char prop = (tex & EDGE_PROP_MASK) >> EDGE_PROP_SHIFT;
              if (prop < 4)
              {
                if (haveKeyCard(prop))
                {
                  openDoor(itemAtCenterOfView);
                }
                else
                {
                  playSound(SOUND_OOF);
                  eraseMessage();
                  textcolor(7);
                  cputsxy(1, 14, "you need a       key");
                  cputsxy(2, 15, "to open this door!");
                  textcolor(keyCardColor(prop));
                  --prop;
                  cputsxy(12, 14, keyCardNames[prop]);
                  eraseMessageAfter = 8;
                }
              }
              else if (prop == DOOR_TYPE_ONEWAY)
              {
                char otherSec = getOtherSector(itemAtCenterOfView, playerSector);
                if (otherSec < playerSector)
                {
                  openDoor(itemAtCenterOfView);
                }
              }
            }
            else if (typeAtCenterOfView == TYPE_SWITCH)
            {
              doEdgeSpecial(itemAtCenterOfView);
            }
        }
      }

      updateAcid();

      {
        setTickCount();
        push_out();
        print2DigitNumToScreen(getTickCount(), 0x1000);
      }

      setSectorVisited(playerSector);

      setCameraX(playerx);
      setCameraY(playery);

      p_enemy_startframe();
      clearSecondBuffer();
      // draw to second buffer
      setTickCount();
      drawSpans();
      print2DigitNumToScreen(getTickCount(), 0x1016);
      // this takes about 30 raster lines
      copyToPrimaryBuffer();
      setTickCount();
      p_enemy_think();
      print2DigitNumToScreen(getTickCount(), 0x102C);
      
      ++frame;
      frame &= 7;
      
      updateFace();

      if (eraseMessageAfter != 0)
      {
        --eraseMessageAfter;
        if (!eraseMessageAfter)
        {
          eraseMessage();
        }
      }

      if (PEEK(198) > 0)
      {
        char w = PEEK(631) - 49;
        if (w < 3 && w != weapon && weapons[w])
        {
          preparePickupMessage();
          printCentered(weaponNames[w], 14);
          weapon = w;
          drawHudAmmo();
        }
      }

      handleCheatCodes();
    }
    pauseMapTimer();
    
    textcolor(2);
    if (health == 0)
    {
      cputsxy(5, 13, "you are dead");
      cputsxy(5, 15, "press return");
    }
    else
    {
      cputsxy(5, 13, "map complete");
      playMusic("pintermus");
      ++level;
    }

    meltScreen(health);
    
    if (health != 0)
    {
      summaryScreen();
      if (level == 9)
      {
        victoryScreen();
        goto start;
      }
    }
    stopMusic();
    goto nextLevel;
    
    return EXIT_SUCCESS;
}
