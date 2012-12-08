#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "updateInput.h"
#include "playSound.h"

#include "util.h"
#include "vicdoom.h"

#pragma staticlocals(on)

void __fastcall__ victoryScreen(void)
{
  int i;
  char j, x;
  char caFileName[] = "pvictory1.txt";

  playMusic("pvictormus");

  for (j = 0; j < 2; ++j)
  {
    // clear screen
    clearScreen();
    for (i = 0; i < 506; ++i)
    {
      POKE(0x9400+i, 0);
    }

    // read in the text, then reveal it by painting red slowly.
    caFileName[8] = '1' + j;
    load_data_file(caFileName);

    for (i = 0; i < 506; ++i)
    {
      x = PEEK(0x1000+i);
      if (x > 96) x -= 96;

      POKE(0x1000+i, x);
      POKE(0x9400+i, 2);

      if (x != 32)
      {
        waitForRaster(4);
      }
    }
  
    POKE(198, 0);
    while (PEEK(198) == 0) ;
  }

  // clear screen
  clearScreen();
  stopMusic();
}
