#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "util.h"
#include "playSound.h"

#pragma staticlocals(on)

void __fastcall__ printCentered(char *str, char y)
{
  char len = strlen(str);
  cputsxy(11 - len/2, y, str);
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

void __fastcall__ load_full_text_screen(char *fname)
{
  int i, k, m, q;
  char x, c;

  POKE(0x900f,8); //black border
  POKE(198, 0);

  // read in the text, then type it out
  load_data_file(fname);

  // clear screen
  clearScreen();

  k = 0x1600;
  i = 0x1000;
  m = i;
  q = 0x9400;
  c = 2;
  while (1)
  {
    x = PEEK(k);
    if (x == '@') break;
      
    if (x == '\n')
    {
      i += 22;
      m = i;
      q = m + 0x8400;
    }
    else if (x == '^')
    {
      // toggle between red and yellow
      c = 9 - c;
    }
    else if (x != '\r')
    {
      if (x > 96) x -= 96;
      POKE(m, x);
      POKE(q, c);
      ++m;
      ++q;
      POKE(m, 29);
      if (x != 32)
      {
        char wait = 4;
        if (x == '.' || x == '?' || x == '!') wait = 48;
        if (x == ',') wait = 16;
        if (PEEK(198) == 0) waitForRaster(wait);
      }
      POKE(m, 32);
    }
    ++k;
  }
  
  POKE(198, 0);
  while (1)
  {
    waitForRaster(32);
    POKE(m,29);
    if (PEEK(198)) break;
    waitForRaster(32);
    POKE(m,32);
    if (PEEK(198)) break;
  }

  clearScreen();
}