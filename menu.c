#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "updateInput.h"
#include "playSound.h"

void __fastcall__ drawLogo(void);

#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

char *caMenus[6][3] =
{
  {
	"new game",
	"options",
	"credits"
  },
  {
	"knee deep in the dead",
	"the shores of hell",
	"inferno"
  },
  {
	"effects volume 15",
	"music volume 10",
	"keys"
  },
  {
    "can i play daddy?",
	"hurt me plenty",
	"ultra-violence"
  },
  {
	"wasd for movement",
	"j and l to turn",
	"i fire k use ctrl map"
  },
  {
    "code by steve mccrea",
    "cc65 by u. bassewitz",
    "doom by # software"
  }
};

char nextMenu[6][3] = { { 1, 2, 5 }, { 3, 3, 3 }, { -1, -1, 4 }, { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 } };

char menu = 0;
char item = 0;

char episode = 0;
char difficulty = 2;

char menuStack[3];
char itemStack[3];
char stackDepth = 0;

char oldKeys = 0;
char oldCtrlKeys = 0;
char moveKeys = 0;
char ctrlKeys = 0;

char keyPressed(char keyMask)
{
  return ((moveKeys & keyMask) && !(oldKeys & keyMask));
}

char ctrlKeyPressed(char keyMask)
{
  return ((ctrlKeys & keyMask) && !(oldCtrlKeys & keyMask));
}

void waitForEscReleased(void)
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

void drawMenuItem(int i)
{
  char y = MENU_Y + (i<<1);
  char *itemStr = caMenus[menu][i];
  char len = strlen(itemStr);
  char x = 11 - len/2;
  cputsxy(x, y, itemStr);
}

void drawMenu(void)
{
  char i, y;
  // draw the menu
  for (i = 0; i < 3; ++i)
  {
    y = MENU_Y + (i<<1);
	cputsxy(0, y, "                      ");
    if (i != item)
    {
      textcolor(TEXT_COLOR);
    }
    else
    {
      textcolor(HILITE_COLOR);
      cputsxy(0, y, "@");
    }
    drawMenuItem(i);
  }
  textcolor(HILITE_COLOR);
}

void enterNumberInMenuItem(char *place, char num)
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

void addToEffectsVolume(char add)
{
  char effectsVolume = getEffectsVolume() + add;
  if (effectsVolume < 16)
  {
    enterNumberInMenuItem(caMenus[menu][0] + 15, effectsVolume);
    drawMenuItem(item);
    setEffectsVolume(effectsVolume);
    playSound(SOUND_PISTOL);
  }
}

void addToMusicVolume(char add)
{
  char musicVolume = getMusicVolume() + add;
  if (musicVolume < 16)
  {
	enterNumberInMenuItem(caMenus[menu][1] + 13, musicVolume);
	drawMenuItem(item);
    setMusicVolume(musicVolume);
  }
}

// returns 1 if should restart
char runMenu(char canReturn)
{
#if 1
   return 0;
#else
   if (canReturn)
   {
     playSound(SOUND_STNMOV);
   }
   waitForEscReleased();
   
   drawLogo();
   
   enterNumberInMenuItem(caMenus[2][0] + 15, getEffectsVolume());
   enterNumberInMenuItem(caMenus[2][1] + 13, getMusicVolume());

   menu = 0;
   item = 0;
   stackDepth = 0;
   
   drawMenu();
   
   while (menu != 255)
   {
     oldKeys = moveKeys;
     oldCtrlKeys = ctrlKeys;
	 moveKeys = readInput();
	 ctrlKeys = getControlKeys();
	 
	 if (keyPressed(KEY_FORWARD))
	 {
	   textcolor(TEXT_COLOR);
	   cputsxy(0, MENU_Y+(item<<1), " ");
	   drawMenuItem(item);
	   --item;
	   if (item == 255) item = 2;
	   textcolor(HILITE_COLOR);
	   cputsxy(0, MENU_Y+(item<<1), "@");
	   drawMenuItem(item);
	   playSound(SOUND_STNMOV);
	 }
	 if (keyPressed(KEY_BACK))
	 {
	   textcolor(TEXT_COLOR);
	   cputsxy(0, MENU_Y+(item<<1), " ");
	   drawMenuItem(item);
	   ++item;
	   if (item == 3) item = 0;
	   textcolor(HILITE_COLOR);
	   cputsxy(0, MENU_Y+(item<<1), "@");
	   drawMenuItem(item);
	   playSound(SOUND_STNMOV);
	 }
     if (menu == 2)
     {
		 if (keyPressed(KEY_MOVERIGHT))
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
			drawMenu();
		}
	  }
	  if (ctrlKeyPressed(KEY_RETURN))
	  {
	    if (menu == 1) episode = item;
	    if (menu == 3)
	    {
	      difficulty = item;
	      return 1;
	    }

	    if (nextMenu[menu][item] != -1)
	    {
          playSound(SOUND_PISTOL);
	      menuStack[stackDepth] = menu;
	      itemStack[stackDepth] = item;
	      ++stackDepth;
	      menu = nextMenu[menu][item];
	      item = 0;
	      drawMenu();
	    }
	  }
	}

#endif
}
