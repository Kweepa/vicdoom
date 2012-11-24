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
  char caFileName[] = "victory1.txt";

  playMusic("victormus");

  for (j = 0; j < 2; ++j)
  {
    // clear screen
    putchar(147);
    putchar(13);
    for (i = 0; i < 506; ++i)
    {
      POKE(0x9400+i, 0);
    }

    // read in the text, then reveal it by painting red slowly.
    caFileName[7] = '1' + j;
    read_data_file(caFileName, 0x1000, 506);

    for (i = 0; i < 506; ++i)
    {
      x = PEEK(0x1000+i);
      if (x > 96) x -= 96;

      POKE(0x1000+i, x);
      POKE(0x9400+i, 2);

      if (x != 32)
      {
        waitforraster();
        waitforraster();
        waitforraster();
        waitforraster();
      }
    }
  
    POKE(198, 0);
    while (PEEK(198) == 0) ;
  }

  // clear screen
  putchar(147);
  putchar(13);
  stopMusic();
}
