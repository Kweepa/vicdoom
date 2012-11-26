#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  FILE *fpi = fopen(argv[1], "rb");
  if (fpi != NULL)
  {
    char caFileName[64];
    sprintf(caFileName, "p%s", argv[1]);
    {
      FILE *fpo = fopen(caFileName, "wb");
      if (fpo != NULL)
      {
        char *addrStr = argv[2];
        int addr = 0;
        int i = 0;
        int len;
        char *alloc;
        while (i < strlen(addrStr))
        {
          char ch = addrStr[i];
          addr *= 16;
          if (ch >= '0' && ch <= '9') addr += ch - '0';
          else if (ch >= 'a' && ch <= 'f') addr += ch - 'a' + 10;
          else if (ch >= 'A' && ch <= 'F') addr += ch - 'A' + 10;
          i++;
        }
        fputc(addr&255, fpo);
        fputc(addr>>8, fpo);
        fseek(fpi, 0, SEEK_END);
        len = ftell(fpi);
        fseek(fpi, 0, SEEK_SET);
        alloc = malloc(len);
        fread(alloc, 1, len, fpi);
        fwrite(alloc, 1, len, fpo);
        fclose(fpo);
      }
    }
    fclose(fpi);
  }
}
