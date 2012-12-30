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
  char j;
  char caFileName[] = "pvictory1.txt";

  playMusic("pvictormus");

  for (j = 0; j < 2; ++j)
  {
    caFileName[8] = '1' + j;
    load_full_text_screen(caFileName);
  }

  stopMusic();
}
