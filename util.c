#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "util.h"
#include "playSound.h"

void __fastcall__ printCentered(char *str, char y)
{
  char len = strlen(str);
  cputsxy(11 - len/2, y, str);
}

unsigned int __fastcall__ sqrt24(unsigned long x)
{
  // reduced from 0x40000000 (see only usage in push_out_from_edge)
  unsigned long m = 0x00100000;
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

unsigned int __fastcall__ sqrt(unsigned long x)
{
  // reduced from 0x40000000 (see only usage in push_out_from_edge)
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
