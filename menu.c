#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "updateInput.h"
#include "playSound.h"
#include "util.h"

#pragma staticlocals(on)

void __fastcall__ drawLogo(void);

char *textScreens[] = { "pcredits.txt", "phelp.txt", "porder.txt" };

char *caMenus[][4] =
{
  {
	  "new game",
	  "options",
	  "credits",
    "back"
  },
  {
	  "knee deep in the dead",
	  "the shores of hell",
	  "inferno",
    "back"
  },
  {
	  "effects volume 15",
	  "music volume 10",
	  "controls",
    "back"
  },
  {
    "i'm too young to die",
    "hurt me plenty",
	  "ultra-violence",
    "back"
  }
};

char nextMenu[4][4] = { { 1, 2, -1, -66 }, { 3, -3, -3, 0 }, { -10, -10, -2, 0 }, { -10, -10, -10, 1 } };

char menu = 0;
char item = 0;
char menuSize = 4;

char episode = 0;
char difficulty = 2;

char menuStack[3];
char itemStack[3];
char stackDepth = 0;

char oldKeys = 0;
char oldCtrlKeys = 0;
char moveKeys = 0;
char ctrlKeys = 0;

char __fastcall__ keyPressed(char keyMask)
{
  return ((moveKeys & keyMask) && !(oldKeys & keyMask));
}

char __fastcall__ ctrlKeyPressed(char keyMask)
{
  return ((ctrlKeys & keyMask) && !(oldCtrlKeys & keyMask));
}

void __fastcall__ waitForEscReleased(void)
{
  do
  {
    oldKeys = moveKeys;
    oldCtrlKeys = ctrlKeys;
    moveKeys = readInput();
    ctrlKeys = getControlKeys();
  }
  while (ctrlKeys & KEY_ESC);
}

#define TEXT_COLOR 2
#define HILITE_COLOR 7
#define MENU_Y 11

void __fastcall__ drawMenuItem(char i)
{
  char y = MENU_Y + (i<<1);
  char *itemStr = caMenus[menu][i];
  if (i != item)
  {
    textcolor(TEXT_COLOR);
    cputsxy(0, y, "                      ");
  }
  else
  {
    textcolor(HILITE_COLOR);
    cputsxy(0, y, "@                     ");
  }
  printCentered(itemStr, y);
}

void __fastcall__ drawMenu(char canReturn)
{
  char i;
  for (i = 0; i < 198; ++i)
  {
    POKE(0x1000 + 242 + i, 32);
    POKE(0x9400 + 242 + i, 7);
  }

  if (!canReturn)
  {
    textcolor(TEXT_COLOR);
    cputsxy(3, 21, "down");
    cputsxy(15, 21, "select");
    textcolor(HILITE_COLOR);
    cputsxy(1, 21, "s");
    cputsxy(8, 21, "return");
  }

  menuSize = 4;
  if (stackDepth == 0 && !canReturn)
  {
    menuSize = 3;
  }
	 
  // draw the menu
  for (i = 0; i < menuSize; ++i)
  {
    drawMenuItem(i);
  }
  textcolor(HILITE_COLOR);
}

void __fastcall__ enterNumberInMenuItem(char *place, char num)
{
  if (num > 9)
  {
    place[0] = '1';
    place[1] = '0' + num - 10;
  }
  else
  {
    place[0] = ' ';
    place[1] = '0' + num;
  }
}

void __fastcall__ addToEffectsVolume(char add)
{
  char effectsVolume = getEffectsVolume() + add;
  effectsVolume &= 15;
  enterNumberInMenuItem(caMenus[menu][0] + 15, effectsVolume);
  drawMenuItem(item);
  setEffectsVolume(effectsVolume);
  playSound(SOUND_PISTOL);
}

void __fastcall__ addToMusicVolume(char add)
{
  char musicVolume = getMusicVolume() + add;
  musicVolume &= 15;
	enterNumberInMenuItem(caMenus[menu][1] + 13, musicVolume);
	drawMenuItem(item);
  setMusicVolume(musicVolume);
}

// returns 1 if should restart
char __fastcall__ runMenu(char canReturn)
{
  if (canReturn)
  {
    playSound(SOUND_STNMOV);
  }
  waitForEscReleased();
   
  drawLogo();
//  printCentered("akronyme  analogiker", 0);
//  printCentered("presents", 1);

  enterNumberInMenuItem(caMenus[2][0] + 15, getEffectsVolume());
  enterNumberInMenuItem(caMenus[2][1] + 13, getMusicVolume());

  menu = 0;
  item = 0;
  stackDepth = 0;
   
  drawMenu(canReturn);
   
  while (menu != 255)
  {
    oldKeys = moveKeys;
    oldCtrlKeys = ctrlKeys;
    moveKeys = readInput();
    ctrlKeys = getControlKeys();

    if (keyPressed(KEY_FORWARD))
    {
	    char oldItem = item;
	    --item;
      if (item == 255) item = menuSize-1;
	    drawMenuItem(oldItem);
	    drawMenuItem(item);
	    playSound(SOUND_STNMOV);
    }
    if (keyPressed(KEY_BACK))
    {
	    char oldItem = item;
	    ++item;
      if (item == menuSize) item = 0;
	    drawMenuItem(oldItem);
	    drawMenuItem(item);
	    playSound(SOUND_STNMOV);
    }
    if (menu == 2)
    {
	    if (keyPressed(KEY_MOVERIGHT) || ctrlKeyPressed(KEY_RETURN))
	    {
		    if (item == 0)
		    {
  		    addToEffectsVolume(1);
		    }
		    else if (item == 1)
		    {
		      addToMusicVolume(1);
		    }
	    }
	    if (keyPressed(KEY_MOVELEFT))
	    {
		    if (item == 0)
		    {
          addToEffectsVolume(255);
		    }
		    else if (item == 1)
		    {
		      addToMusicVolume(255);
		    }
	    }
    }
	  if (ctrlKeyPressed(KEY_ESC))
	  {
	    if (stackDepth == 0)
	    {
        if (canReturn)
        {
          playSound(SOUND_OOF);
          waitForEscReleased();
          return 0;
		    }
		  }
		  else
		  {
        playSound(SOUND_OOF);
			  --stackDepth;
			  menu = menuStack[stackDepth];
			  item = itemStack[stackDepth];
			  drawMenu(canReturn);
		  }
	  }
	  if (ctrlKeyPressed(KEY_RETURN))
	  {
	    signed char next;
	    if (menu == 1) episode = item;

      next = nextMenu[menu][item];
      if (next == -66)
      {
        playSound(SOUND_OOF);
        return 0;
      }
      else if (next != -10)
      {
        playSound(SOUND_PISTOL);
			  if (next >= 0)
			  {
          if (next > menu)
          {
			      menuStack[stackDepth] = menu;
			      itemStack[stackDepth] = item;
			      ++stackDepth;
  			    item = 0;
          }
          else
          {
            --stackDepth;
            item = itemStack[stackDepth];
          }
			    menu = next;
			    drawMenu(canReturn);
			  }
			  else
			  {
			    next = (-next)-1;
          clearScreen();
          waitForRaster(30);
          load_full_text_screen(textScreens[next]);
          setupBitmap(8 + 2); // multicolor red
          drawLogo();
			    drawMenu(canReturn);
			  }
  		}
	    else if (menu == 3)
	    {
        stopMusic();
        playSound(SOUND_PISTOL);
        clearScreen();
        waitForRaster(30);
	      difficulty = item;
	      return 1;
	    }
	  }
	}
}
