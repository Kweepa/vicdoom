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

void automap_drawEdgeToSecondBuffer(int x0, int y0, int x1, int y1)
{
   signed char err, e2;
   char drewAPixel = 0;
   int maxx = x1;
   int maxy = y1;
   int minx = x0;
   int miny = y0;
   signed char sx = 1;
   signed char sy = 1;
   signed char dx = x1-x0;
   signed char dy = y1-y0;
   if (dx < 0)
   {
     sx = -1;
     dx = -dx;
     maxx = x0;
     minx = x1;
   }
   if (dy < 0)
   {
     sy = -1;
     dy = -dy;
     maxy = y0;
     miny = y1;
   }
   if (maxx >= 0 && minx < 64 && maxy >= 0 && miny < 64)
   {
	   err = (dx > dy) ? dx>>1 : -(dy>>1);
	   do
	   {
		 signed char chk = (x0 & 0xc0) | (y0 & 0xc0);
		 if (!chk)
		 {
		   int a = 0x1800 + ((x0&0xf8)<<3) + y0;
		   POKE(a, PEEK(a) | shift[x0&7]);
		   drewAPixel = 1;
		 }
		 else
		 {
		   if (drewAPixel) return;
		 }
		 if (x0 == x1 && y0 == y1) return;
		 e2 = err;
		 if (e2 > -dx)
		 {
		   err -= dy;
		   x0 += sx;
		 }
		 if (e2 < dy)
		 {
		   err += dx;
		   y0 += sy;
		 }
	   }
	   while (1);
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
