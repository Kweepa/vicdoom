#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

void __fastcall__ eraseMessage(void);
void __fastcall__ waitForRaster(char count);
void __fastcall__ meltScreen(char health);

void __fastcall__ setTextColor(char c);
//void __fastcall__ printIntAtXY(char i, char x, char y, char prec);
void __fastcall__ print3DigitNumToScreen(char i, int addr);
void __fastcall__ print2DigitNumToScreen(char i, int addr);
void __fastcall__ printCentered(char *str, char y);
unsigned int __fastcall__ sqrt24(unsigned long x);
unsigned int __fastcall__ sqrt(unsigned long x);
void __fastcall__ read_data_file(char *name, unsigned int addr, int maxSize);
void __fastcall__ playMusic(char *name);

void __fastcall__ load_data_file(char *fname);
void __fastcall__ load_file(char *fname, char fnamelen);