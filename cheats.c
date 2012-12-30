#include <stdio.h>

#include "util.h"

char *cheats[] =
{
  "iddqd",
  "idkfa",
  "idclev",
  "iddt"
};
char cheatp[] = { 0, 0, 0, 0 };

char updateCheatCodes(void)
{
  char i, c, b, m, r;
  r = -1;
  b = PEEK(198);
  for (m = 0; m < b; ++m)
  {
    c = PEEK(631 + m);
    for (i = 0; i < sizeof(cheatp); ++i)
    {
      if (cheats[i][cheatp[i]] == c)
      {
        ++cheatp[i];
        if (cheats[i][cheatp[i]] == 0)
        {
          cheatp[i] = 0;
          r = i;
        }
      }
      else
      {
        cheatp[i] = 0;
      }
    }
  }
  POKE(198, 0);

  return r;
}

