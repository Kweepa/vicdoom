#include "player.h"
#include "mapAsm.h"
#include "updateInput.h"
#include "drawColumn.h"
#include "playSound.h"

#include "automap.h"

#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

#define MAX_OFFSET 200
#define OFFSET_STEP 8

char edgesSeen[128/8];
char shift[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

int offsetX, offsetY;
int zoom = 2;

void automap_reset(void)
{
  // clear the edges seen
  char i;
  for (i = 0; i < 16; ++i)
  {
    edgesSeen[i] = 0;
  }
}

void automap_sawEdge(char i)
{
  edgesSeen[i>>3] |= shift[i&7];
}

void automap_enter(void)
{
  // write white/mono to the colour memory
  char x, y;
  for (x = 0; x < 8; ++x)
  {
    for (y = 0; y < 8; ++y)
    {
      POKE(0x9400 + 22*(y+2) + (x+7), 1);
    }
  }
  // reset offset
  offsetX = 0;
  offsetY = 0;
}

void automap_drawEdgeToSecondBuffer(int x1, int y1, int x2, int y2)
{
   int maxx, maxy, minx, miny;
   signed char xStepDir, yStepDir;
   if (x1 > x2)
   {
      maxx = x1;
      minx = x2;
      xStepDir = -1;
   }
   else
   {
      minx = x1;
      maxx = x2;
      xStepDir = 1;
   }
   if (y1 > y2)
   {
      maxy = y1;
      miny = y2;
      yStepDir = -1;
   }
   else
   {
      miny = y1;
      maxy = y2;
      yStepDir = 1;
   }
   if (maxx >= 0 && minx < 64 && maxy >= 0 && miny < 64)
   {
      if (maxx - minx > maxy - miny)
      {
         // horizontal
         int bigY = y1<<8;
         int yStep = y2 - y1;
         if (yStep)
         {
           yStep = 256*((y2 - y1))/(maxx - minx);
         }
         for (; x1 != x2; x1 += xStepDir)
         {
            signed char y = bigY>>8;
            //if (x1 >= 0 && x1 < 64 && y >= 0 && y < 64)
            signed char chk = (x1 & 0xc0) | (y & 0xc0);
            if (!chk)
            {
              int a = 0x1800 + ((x1&0xf8)<<3) + y;
              POKE(a, PEEK(a) | shift[x1&7]);
            }
            bigY += yStep;
         }
      }
      else
      {
         int bigX = x1<<8;
         int xStep = x2 - x1;
         if (xStep)
         {
           xStep = 256*((x2 - x1))/(maxy - miny);
         }
         for (; y1 != y2; y1 += yStepDir)
         {
            signed char x = bigX>>8;
            //if (x >= 0 && x < 64 && y1 >= 0 && y1 < 64)
            signed char chk = (x & 0xc0) | (y1 & 0xc0);
            if (!chk)
            {
              int a = 0x1800 + ((x&0xf8)<<3) + y1;
              POKE(a, PEEK(a) | shift[x&7]);
            }
            bigX += xStep;
         }
      }
   }
}

char automap_update(void)
{
  char s, i, ns, nv, e, t;

  // check for scrolling
  char keys = readInput();
  char ctrlKeys = getControlKeys();
  if (ctrlKeys & KEY_CTRL)
  {
    do
    {
      readInput();
      ctrlKeys = getControlKeys();
    }
    while (ctrlKeys & KEY_CTRL);
    return 0;
  }
  if ((keys & KEY_MOVERIGHT) && offsetX > -MAX_OFFSET)
  {
    offsetX -= OFFSET_STEP;
  }
  if ((keys & KEY_MOVELEFT) && offsetX < MAX_OFFSET)
  {
    offsetX += OFFSET_STEP;
  }
  if ((keys & KEY_FORWARD) && offsetY < MAX_OFFSET)
  {
    offsetY += OFFSET_STEP;
  }
  if ((keys & KEY_BACK) && offsetY > -MAX_OFFSET)
  {
    offsetY -= OFFSET_STEP;
  }
  if ((keys & KEY_FIRE) && zoom != 2)
  {
    zoom = 2;
  }
  if ((keys & KEY_USE) && zoom != 1)
  {
    zoom = 1;
  }

  clearSecondBuffer();
  ns = getNumSectors();
  for (s = 0; s < ns; ++s)
  {
    nv = getNumVerts(s);
    for (i = 0; i < nv; ++i)
    {
      t = getOtherSector(s, i);
      if (t == 0xff)
      {
		  e = getEdgeIndex(s, i);
	      if (edgesSeen[e>>3] & shift[e&7])
		  {
			int x1, y1, x2, y2;
			char ii = i + 1;
			if (ii == nv) ii = 0;
			x1 = zoom*((int)(getSectorVertexX(s, i)) - playerx/256 + offsetX) + 32;
			y1 = zoom*((int)(-getSectorVertexY(s, i)) + playery/256 + offsetY) + 32;
			x2 = zoom*((int)(getSectorVertexX(s, ii)) - playerx/256 + offsetX) + 32;
			y2 = zoom*((int)(-getSectorVertexY(s, ii)) + playery/256 + offsetY) + 32;
			automap_drawEdgeToSecondBuffer(x1, y1, x2, y2);
		  }
	  }
    }
  }
  // draw arrow to indicate the player
  copyToPrimaryBuffer();
  
  return 1;
}

void automap(void)
{
  char ctrlKeys;
  automap_enter();
  do
  {
    readInput();
    ctrlKeys = getControlKeys();
  }
  while (ctrlKeys & KEY_CTRL);
  while (automap_update()) ;
}
