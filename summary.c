#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "updateInput.h"
#include "playSound.h"

#include "mapAsm.h"
#include "util.h"
#include "p_enemy.h"
#include "vicdoom.h"

#pragma staticlocals(on)

extern char *caLevelNames[];
extern char level;

void __fastcall__ waitASecond(void)
{
  char i = 60;
  do
  {
	  if (PEEK(198) == 0)
	  {
		  waitForRaster(1);
	  }
  	--i;
  }
  while (i > 0);
}

void __fastcall__ rollInPercentage(char pc, int scr)
{
  signed char i = -4;
  POKE(scr + 3,'%');
  do
  {
    i += 4;
    if (i > pc) i = pc;
    print3DigitNumToScreen(i, scr);
    playSound(SOUND_PISTOL);
    if (PEEK(198) == 0)
    {
		  waitForRaster(2);
    }
  }
  while (i < pc);
}

void __fastcall__ rollInTime(int t, int scr)
{
  int i = -5;
  char ih, il;
  POKE(scr + 2,':');
  do
  {
    i += 5;
    if (i > t) i = t;
    ih = i / 60;
    il = i - 60 * ih;
    print2DigitNumToScreen(ih, scr);
    print2DigitNumToScreen(il, scr + 3);
    playSound(SOUND_PISTOL);
    if (PEEK(198) == 0)
    {
		  waitForRaster(2);
    }
  }
  while (i < t);
}

void __fastcall__ summaryScreen(void)
{
  // display kills, items, secrets, time
  // like this
  // MAP NAME
  // FINISHED
  // KILLS %
  // ITEMS %
  // SECRET %
  // TIME 00:00
  // PAR 00:00
  
  char kills = p_enemy_getKillPercentage();
  char items = getItemPercentage();
  char secret = (100*getNumVisitedSecrets())/getNumSecrets();
  int time = getMapTime();
  int par = getParTime();

  // clear screen
  clearScreen();
  
  textcolor(1);
  printCentered(caLevelNames[level-1], 1);
  textcolor(2);
  printCentered("finished", 2);
  cputsxy(4, 5, "kills");
  cputsxy(4, 7, "items");
  cputsxy(4, 9, "secret");
  cputsxy(4, 12, "time");
  cputsxy(4, 14, "par");

  setTextColor(2);
  POKE(198, 0);
  rollInPercentage(kills, 0x1000+22*5+14);
  waitASecond();
  rollInPercentage(items, 0x1000+22*7+14);
  waitASecond();
  rollInPercentage(secret, 0x1000+22*9+14);
  waitASecond();
  if (time > 10*60-1)
  {
    cputsxy(13,12,"sucks");
    playSound(SOUND_PISTOL);
  }
  else
  {
    rollInTime(time, 0x1000+22*12+13);
  }
  waitASecond();
  rollInTime(par, 0x1000+22*14+13);
  waitASecond();
  printCentered("press a key", 20);
  POKE(198, 0);
  while (PEEK(198) == 0) ;

  // clear screen
  clearScreen();
}
