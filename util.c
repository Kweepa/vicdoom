#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "util.h"

char textColor = 1;

void setTextColor(char c)
{
  textColor = c;
}

void printIntAtXY(char i, char x, char y, char prec)
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

void printCentered(char *str, char y)
{
  char len = strlen(str);
  cputsxy(11 - len/2, y, str);
}

// use this to line up raster timing lines
void waitforraster(void)
{
    while (PEEK(0x9004) > 16) ;
    while (PEEK(0x9004) < 16) ;
}

unsigned int sqrt(unsigned long x)
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
