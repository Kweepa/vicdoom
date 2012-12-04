unsigned int __fastcall__ div88(unsigned int x, unsigned int y);
int __fastcall__ transformxy(int x, int y);
int __fastcall__ transformx(void);
int __fastcall__ leftShift4ThenDiv(int p, unsigned int q);

void __fastcall__ generateMulTab(void);
void __fastcall__ fastMultiplySetup8x8(signed char a);
int __fastcall__ fastMultiply8x8(signed char b);
void __fastcall__ fastMultiplySetup16x8(signed char a);
int __fastcall__ fastMultiply16x8(int b);
void __fastcall__ fastMultiplySetup16x8e24(signed char a);
long __fastcall__ fastMultiply16x8e24(int b);
