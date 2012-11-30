#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "util.h"
#include "playSound.h"

char textColor = 1;

void __fastcall__ setTextColor(char c)
{
  textColor = c;
}

void __fastcall__ printIntAtXY(char i, char x, char y, char prec)
{
  unsigned int screenloc = 0x1000 + 22*y + x;
  unsigned int colorloc = 0x9400 + 22*y + x;
  char digit = 0;
  if (prec == 3)
  {
	  while (i >= 100) { ++digit; i -= 100; }
	  POKE(screenloc, '0' + digit);
	  POKE(colorloc, textColor);
      ++screenloc;
      ++colorloc;
	  digit = 0;
  }
  while (i >= 10) { ++digit; i -= 10; }
  POKE(screenloc, '0' + digit);
  POKE(colorloc, textColor);
  ++screenloc;
  ++colorloc;
  POKE(screenloc, '0' + i);
  POKE(colorloc, textColor);
}

void __fastcall__ printCentered(char *str, char y)
{
  char len = strlen(str);
  cputsxy(11 - len/2, y, str);
}

unsigned int __fastcall__ sqrt(unsigned long x)
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

void __fastcall__ load_data_file(char *fname)
{
  load_file(fname, strlen(fname));
}

void __fastcall__ playMusic(char *name)
{
  stopMusic();
  load_data_file(name);
  startMusic();
}
