#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

void __fastcall__ eraseMessage(void);

void __fastcall__ setTextColor(char c);
void __fastcall__ printIntAtXY(char i, char x, char y, char prec);
void __fastcall__ printCentered(char *str, char y);
void __fastcall__ waitforraster(void);
unsigned int __fastcall__ sqrt(unsigned long x);
void __fastcall__ read_data_file(char *name, unsigned int addr, int maxSize);