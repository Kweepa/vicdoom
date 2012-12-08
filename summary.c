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

void __fastcall__ rollInPercentage(char pc, char y)
{
  char i = 0;
  cputsxy(17, y, "%");
  do
  {
    print3DigitNumToScreen(i, 0x1000+22*y+14);
    playSound(SOUND_PISTOL);
    i++;
    if (PEEK(198) == 0)
    {
		  waitForRaster(2);
    }
  }
  while (i <= pc);
}

void __fastcall__ rollInTime(int t, char y)
{
  int i = -5;
  char ih, il;
  cputsxy(15, y, ":");
  do
  {
    int screenPos;
    i += 5;
    if (i > t) i = t;
    ih = i / 60;
    il = i % 60;
    screenPos = 0x1000 + 22*y + 13;
    print2DigitNumToScreen(ih, screenPos);
    screenPos += 3;
    print2DigitNumToScreen(il, screenPos);
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
  printCentered(getMapName(), 1);
  textcolor(2);
  printCentered("finished", 2);
  cputsxy(4, 5, "kills");
  cputsxy(4, 7, "items");
  cputsxy(4, 9, "secret");
  cputsxy(4, 12, "time");
  cputsxy(4, 14, "par");

  setTextColor(2);  
  POKE(198, 0);
  rollInPercentage(kills, 5);
  waitASecond();
  rollInPercentage(items, 7);
  waitASecond();
  rollInPercentage(secret, 9);
  waitASecond();
  rollInTime(time, 12);
  waitASecond();
  rollInTime(par, 14);
  waitASecond();
  printCentered("press a key", 20);
  POKE(198, 0);
  while (PEEK(198) == 0) ;

  // clear screen
  clearScreen();
}
