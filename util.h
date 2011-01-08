#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

void __fastcall__ eraseMessage(void);

void setTextColor(char c);
void printIntAtXY(char i, char x, char y, char prec);
void printCentered(char *str, char y);
void waitforraster(void);
unsigned int sqrt(unsigned long x);
