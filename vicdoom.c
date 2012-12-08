// doom for the vic-20
// written for cc65
// see make.bat for how to compile

// done
// X 1. move angle to sin/cos and logsin/logcos to a separate function and just use those values
// X 2. fix push_out code
// X 3. add transparent objects
// X 4. finish map and use that instead of the test map
// X 5. enemy AI (update only visible enemies, plus enemies in the current sector)
// X 5.5. per sector objects (link list)
// X 6. add keys and doors
// X 7. add health
// X 8. advance levels
// X 9. menus
// X 11. use a double buffer scheme that draws to two different sets of characters and just copies the characters over
// X 13. projectiles!
// X 14. remote doors need to be openable from the other side - new edge prop that opens a door with e=e-1
// X 16. make acid do damage

// todo
// 2.5. fix push_out code some more
// 7.5. and weapons (maybe?)
// 10. more optimization?
// 12. optimize push_out code and more importantly the ai try_move code
// 12.1. any walls that can be collided with should be straight or 45 degree diagonal
// 12.2. then the push out code can be done in 16 bit, asm
// 15. scale x on map (see drawLine.s (*0.75)) - also need to scale the input position
// 16. make acid do damage

// memory map:
// see the .cfg file for how to do this
// startup code is $82 bytes long - fix with fill = yes
// 1000-11FF screen
// 1200-13FF startup + random data
// 1400-15FF character font
// 1600-17FF 8x8 bitmapped display character ram
// 1800-7FFF code/data
// A000-BDFF texture data, level data, music, sound
// BE00-BFFF back buffer

#define IDDQD 0

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

texFrame texFrames[] =
{
  { 8, 1, 1, 5 }, // possessed
  { 11, 1, 1, 5 }, // imp
  { 14, 1, 1, 3 }, // demon
  { 17, 1, 1, 3 }, // caco
  { 11, 1, 1, 5 }, // baron
  { 22, 0, 0, 5, 8, 8, 0, 16 }, // green armor
  { 22, 0, 0, 5, 0, 8, 0, 16 }, // blue armor
  { 23, 0, 0, 5, 8, 8, 0, 16 }, // bullets
  { 23, 0, 0, 8, 24, 8, 0, 8 }, // medikit
  { 23, 0, 0, 8, 24, 8, 8, 8 }, // red keycard
  { 23, 0, 0, 8, 16, 8, 0, 8 }, // green keycard
  { 23, 0, 0, 8, 16, 8, 8, 8 }, // blue keycard
  { 24, 0, 0, 8, 16, 16, 0, 16 }, // barrel
  { 21, 0, 1, 5 }, // pillar (was width 5)
  { 24, 0, 0, 8, 0, 16, 0, 16 }, // skullpile
  { 22, 0, 0, 2, 16, 4, 0, 16 }, // acid
  { 20, 0, 0, 4, 0, 8, 0, 16 }, // possessed corpse (with bullets)
  { 20, 0, 0, 4, 0, 8, 0, 16 }, // possessed corpse
  { 20, 0, 0, 4, 8, 8, 0, 16 }, // imp corpse
  { 20, 0, 0, 3, 16, 8, 0, 16 }, // demon corpse
  { 19, 0, 0, 3, 0, 16, 0, 16 }, // caco corpse
  { 20, 0, 0, 3, 24, 8, 0, 16 }, // baron corpse
  { 19, 0, 0, 4, 16, 16, 0, 16 }, // imp shot
  { 25, 0, 0, 2, 0, 32, 0, 16 }, // exploding barrel
};

char *pickupNames[] =
{
  "green armor!",
  "blue armor!",
  "bullets!",
  "medikit!",
  "red keycard!",
  "green keycard!",
  "blue keycard!"
};

int playerx;
int playery;
char playera;
char playerSector;

// a render problem (e1m1):
//int playerx = 13*256 + 109;
//int playery = 34*256 + 240;
//char playera = 10;
//char playerSector = 8;

char keyCards[8];
char shells = 40;
char armor = 0;
char combatArmor = 0;
signed char health = 100;

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

unsigned char frame = 0;

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
               }
               else
               {
                   // switch, door or techwall, so fit to wall
                   texI = t >> 4;
               }
               texI &= 15; // 16 texel wide texture
               
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
    textureIndex = texFrames[objectType].texture;
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
  w = getWidthFromHeight(texFrames[objectType].widthScale, hc);
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
    char armorColor = 5;
    if (combatArmor == 1) armorColor = 6;
    textcolor(armorColor);
    cputsxy(5, 21, "^");
    setTextColor(armorColor);
    print3DigitNumToScreen(armor, 0x1000 + 22*21 + 6);
}

void __fastcall__ drawHudAmmo(void)
{
  // shells
  textcolor(3);
  cputsxy(1, 21, "&");
  setTextColor(3);
  print2DigitNumToScreen(shells, 0x1000 + 22*21 + 2);
}

void __fastcall__ drawHudHealth(void)
{
  // health
  textcolor(2);
  cputsxy(13, 21, "/");
  setTextColor(2);
  print3DigitNumToScreen(health, 0x1000 + 22*21 + 14);
}

char keyCardColors[3] = { 2, 5, 6 };
char *keyCardNames[3] = { " red", "green", "blue" };

void drawHudKeys(void)
{
  char k;
  // cards
  for (k = 1; k < 4; ++k)
  {
    if (keyCards[k] == 1)
    {
      textcolor(keyCardColors[k-1]);
    }
    else
    {
      textcolor(0);
    }
    cputsxy(17+k, 21, ";");
  }
}

extern char difficulty;

void __fastcall__ damagePlayer(char damage)
{
  if (difficulty == 0)
  {
    damage = damage/2;
  }
#if IDDQD == 0
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
#endif
  if (health < 0)
  {
     health = 0;
  }
  drawHudHealth();
  
  // flash border red
  flashBorderTime = 1;
  POKE(0x900F, 8+2);
}

char numItemsGot;

char __fastcall__ getItemPercentage(void)
{
  return (100*numItemsGot)/getNumItems();
}

char barrelAtCenterOfScreen = -1;

void processTransparentObjectAtCenterOfScreen(char o, char vyhi)
{
  char objectType = getObjectType(o);

  if (objectType == kOT_Barrel)
  {
    barrelAtCenterOfScreen = o;
  }

  if (vyhi < 3)
  {
    char pickupType = objectType;
    char remove = 1;
    if (objectType == kOT_PossessedCorpseWithAmmo)
    {
      setObjectType(o, kOT_PossessedCorpse);
      remove = 0;
      pickupType = kOT_Bullets;
    }

    if (pickupType >= kOT_GreenArmor && pickupType <= kOT_BlueKeycard)
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
        if (shells < 80)
        {
            shells += 4;
            if (shells > 80) shells = 80;
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
        keyCards[1] = 1;
        drawHudKeys();
        pickedUp = 1;
        break;
      case kOT_GreenKeycard:
        keyCards[2] = 1;
        drawHudKeys();
        pickedUp = 1;
        break;
      case kOT_BlueKeycard:
        keyCards[3] = 1;
        drawHudKeys();
        pickedUp = 1;
        break;
      }
      if (pickedUp)
      {
        playSound(SOUND_ITEMUP);
        if (remove)
        {
          setObjectSector(o, -1);
          ++numItemsGot;
        }
        // flash border cyan
        flashBorderTime = 1;
        POKE(0x900F, 8 + 3);

        eraseMessage();
        textcolor(7);
        printCentered("you got the", 14);
        printCentered(pickupNames[pickupType - kOT_GreenArmor], 15);
        eraseMessageAfter = 8;
      }
    }
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
  w = getWidthFromHeight(texFrames[objectType].widthScale, hc);
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
          textureIndex = texFrames[objectType].texture;
          startY = texFrames[objectType].startY;
          height = texFrames[objectType].height;
          for (curX = startX; curX < endX; ++curX)
          {
             if (testFilledWithY(curX, vy) > 0)
             {
               if (curX == 0)
               {
                 processTransparentObjectAtCenterOfScreen(o, vy>>8);
               }
                // compensate for pixel samples being mid column
                //texI = TEXWIDTH * (2*(curX - leftX) + 1) / (4 * w);
                texI = getObjectTexIndex(w, curX - leftX);
                if (texFrames[objectType].width != 16)
                {
                   texI = texFrames[objectType].startX + (texI>>1);
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
    acidBurn--;
  }
  if (acidBurn == 0)
  {
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
          acidBurn = 4;
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
         if (texFrames[type].solid)
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
    if (!texFrames[type].solid)
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

EPushOutResult __fastcall__ push_out_from_edge(char i)
{
  totalCheckedEdges++;
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
  edgeLen = getEdgeLen(edgeGlobalIndex);

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

  // height < INNERCOLLISIONRADIUS*edgeLen
  if (height < 0 || (height>>9) < edgeLen)
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
    //dle = (dist < 256*edgeLen*edgeLen);
    {
      long len2 = 0;
      fastMultiplySetup8x8(edgeLen);
      len2 = ((long)fastMultiply8x8(edgeLen))<<8;
      dle = dist < len2;
    }
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
          fastMultiplySetup16x8e24(edgeLen);
          distanceToPush = fastMultiply16x8e24(OUTERCOLLISIONRADIUS) - height;
          if (reversedEdge)
          {
            ex = -ex;
            ey = -ey;
          }
          //playerx += distanceToPush * ey / (edgeLen*edgeLen);
          //playery -= distanceToPush * ex / (edgeLen*edgeLen);
          {
            int edgeLen2;
            fastMultiplySetup8x8(edgeLen);
            edgeLen2 = fastMultiply8x8(edgeLen);
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
      if (!dgz && ((dist>>9) > -edgeLen))
      {
        checkVert = 1;
      }
      else
      {
        if (!dle)
        {
          //if (dist < (256*edgeLen + INNERCOLLISIONRADIUS)*edgeLen))
          int paddedEdgeLen = ((int)(edgeLen+2))<<8;
          fastMultiplySetup16x8e24(edgeLen);
          if (dist < fastMultiply16x8e24(paddedEdgeLen))
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
        //POKE(0x900f, 11);
          height = px * px + py * py;
        //POKE(0x900f, 13);
          //if (height < INNERCOLLISIONRADIUS*INNERCOLLISIONRADIUS)
          if ((height>>18) == 0)
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

char touchedSector;

char possibleWallsToTouch[2];
char numPossibleWallsToTouch;

void __fastcall__ push_out(void)
{
  // probably a good idea to check the edges we can cross first
  // if any of them teleport us, move, then push_out in the new sector
  
  // so, consider at most two edges to push out from
  // whether or not we get pushed in this sector, check any
  // portal edges we touch and try pushing out from edges in the neighbouring sectors
  // hopefully should only touch the one portal edge  

  char i, secNumVerts;
  EPushOutResult r;

  totalCheckedEdges = 0;
  
  touchedSector = 0xff;
  nextSector = 0xff;

  curSector = playerSector;
  secNumVerts = getNumVerts(curSector);
  
  numPossibleWallsToTouch = 0;
  
  // see which edge the new coordinate is behind
  for (i = 0; i < secNumVerts; ++i)
  {
    //if (getOtherSector(getEdgeIndex(curSector, i), curSector) != -1)
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

//  print3DigitNumToScreen(totalCheckedEdges, 0x1000 + 110);
}

char explodingBarrelsObject[4];
char explodingBarrelsTime[4];

void addExplodingBarrel(char o)
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
  char i, t, o, k, d;
  int dx, dy;
  for (i = 0; i < 4; ++i)
  {
    o = explodingBarrelsObject[i];
    if (o != -1)
    {
      explodingBarrelsTime[i]--;
      t = explodingBarrelsTime[i];
      if (t == 2)
      {
        // damage stuff around
        for (k = 0; k < getNumObjects(); ++k)
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
      else if (t == 0)
      {
        setObjectSector(o, -1);
        explodingBarrelsObject[i] = -1;
      }
    }
  }
}

char turnSpeed = 0;
char shotgunStage = 0;
char changeLookTime = 7;
char lookDir = 0;

char soundToPlay = 0;

void __fastcall__ setUpScreenForBitmap(void)
{
  clearScreen();
  setupBitmap(8 + 2); // multicolor red
}

void __fastcall__ setUpScreenForMenu(void)
{
  char i;
  cputsxy(6, 1, "          ");
  cputsxy(6, 10, "          ");
  for (i = 2; i < 10; ++i)
  {
    cputsxy(6, i, " ");
    cputsxy(15, i, " ");
  }
}

void __fastcall__ setUpScreenForGameplay(void)
{
  clearMenuArea();
  setupBitmap(8 + 2); // multicolor red
  POKE(0x900F, 8 + 5); // green border, and black screen
  drawBorders();
  playMapTimer();
}

char __fastcall__ runMenu(char canReturn);

char caLevel[] = "pe1m1";
char caMusic[] = "pe1m1mus";

int main()
{
  char keys;
  char ctrlKeys;
  char i;
  signed char ca, sa;
  char numObj;
  char *mapName;

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

  POKE(0x900E, (6<<4) + (PEEK(0x900E)&0x0f)); // blue aux color
  POKE(0x900F, 8 + 5); // green border, and black screen
  
  // set the character set to $1400
  POKE(0x9005, 13 + (PEEK(0x9005)&0xf0));

start:
  setUpScreenForBitmap();
  setUpScreenForMenu();

  playMusic("pe1m9mus");

  runMenu(0);
  level = 1;
  
nextLevel:

  caMusic[4] = '0' + level;
  playMusic(caMusic);

  setUpScreenForBitmap();
  setUpScreenForGameplay();

  caLevel[4] = '0' + level;
  load_data_file(caLevel);
  mapName = getMapName();
  numObj = getNumObjects();

  resetDoorClosedAmounts();
  doorOpenTime[0] = 0;
  doorOpenTime[1] = 0;
  doorOpenTime[2] = 0;
  doorOpenTime[3] = 0;
  
  p_enemy_resetMap();
  automap_reset();
  for (i = 0; i < numObj; ++i)
  {
    if (getObjectType(i) < 5)
    {
      allocMobj(i);
    }  
  }
  
  addObjectsToSectors();
  
  resetSectorsVisited();
  
  if (health <= 0)
  {
    health = 100;
    armor = 0;
    shells = 40;
  }
  keyCards[0] = 1;
  keyCards[1] = 0;
  keyCards[2] = 0;
  keyCards[3] = 0;
  keyCards[4] = 0;
  keyCards[5] = 0;
  keyCards[6] = 0;
  keyCards[7] = 0;
  numItemsGot = 0;
  playerx = getPlayerSpawnX();
  playery = getPlayerSpawnY();
  playera = getPlayerSpawnAngle();
  playerSector = getPlayerSpawnSector();
  endLevel = 0;
  barrelAtCenterOfScreen = -1;
  explodingBarrelsObject[0] = -1;
  explodingBarrelsObject[1] = -1;
  explodingBarrelsObject[2] = -1;
  explodingBarrelsObject[3] = -1;

  // name of level
  textcolor(2);
  printCentered(mapName, 18);
  drawHudAmmo();
  drawHudArmor();
  drawHudHealth();
  drawHudKeys();
  // face
  textcolor(7);
  cputsxy(10, 20, "[£");
  POKE(0x1000 + 22*20 + 11, 28);
  cputsxy(10, 21, "#$");
  cputsxy(10, 22, "*+");
  
  playMapTimer();
  resetMapTime();

  while (health > 0 && !endLevel)
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
        if (!(testFilled(0) < 4 && typeAtCenterOfView == TYPE_OBJECT))
        {
          playerx += 4*sa;
          playery += 4*ca;
        }
      }
      if (keys & KEY_BACK)
      {
        playerx -= 2*sa;
        playery -= 2*ca;
      }
//      gotoxy(0, 14);
//      cprintf("%d %d %d %d. ", playerSector, playerx, playery, playera);
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
          drawHudAmmo();
          POKE(0x900F, 8+1);
          shotgunStage = 7;
          
          playSound(SOUND_PISTOL);
          if (barrelAtCenterOfScreen != -1)
          {
            addExplodingBarrel(barrelAtCenterOfScreen);
          }
          else if (typeAtCenterOfView == TYPE_OBJECT)
          {
            char damage = 2 + (P_Random()&7);
            if (difficulty == 0)
            {
              damage <<= 1;
            }
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
        if (doorOpenTime[i] > 0)
        {
          if (--doorOpenTime[i] == 0)
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
                if (keyCards[prop] == 1)
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
                  prop--;
                  textcolor(keyCardColors[prop]);
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
      
      --changeLookTime;
      if (changeLookTime == 0)
      {
        lookDir = 1 - lookDir;
        if (lookDir == 0)
        {
            changeLookTime = 12;
            POKE(0x11D8, 35); // 10,22
            POKE(0x11D9, 36); // 11,22
          }
          else
          {
            changeLookTime = 6;
            POKE(0x11D8, 40); // 10,22
            POKE(0x11D9, 41); // 11,22
          }
      }
      
      if (eraseMessageAfter != 0)
      {
        --eraseMessageAfter;
        if (!eraseMessageAfter)
        {
          eraseMessage();
        }
      }
    }
    pauseMapTimer();
    
    textcolor(2);
    if (health <= 0)
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
    
    if (health > 0)
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
