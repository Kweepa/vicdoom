#include "player.h"
#include "mapAsm.h"
#include "updateInput.h"
#include "drawColumn.h"
#include "playSound.h"

#include "automap.h"

#pragma staticlocals(on)

#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

#define MAX_OFFSET 200
#define OFFSET_STEP 4

int offsetX, offsetY;
int zoom = 2;

void __fastcall__ automap_draw(int offsetX, int offsetY, char zoom);
void __fastcall__ automap_resetEdges(void);

void __fastcall__ automap_reset(void)
{
  automap_resetEdges();
}

void __fastcall__ automap_enter(void)
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

char __fastcall__ automap_update(void)
{
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
  automap_draw(offsetX - playerx/256, offsetY + playery/256, zoom);
  // draw arrow to indicate the player
  copyToPrimaryBuffer();
  
  return 1;
}

void __fastcall__ automap(void)
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
