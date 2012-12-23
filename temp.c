#include <stdio.h>

#include "util.h"

unsigned int __fastcall__ sqrt24(unsigned long x)
{
  // reduced from 0x40000000 (see only usage in push_out_from_edge)
  unsigned long m = 0x00100000;
  unsigned long y = 0;
  unsigned long b;
  POKE(0x900f,11);
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
  POKE(0x900f,13);
  return ((unsigned int)y);
}

