int __fastcall__ getScreenX(char i);
int __fastcall__ getTransformedX(char i);
int __fastcall__ getTransformedY(char i);

char __fastcall__ getNumObjects(void);
char __fastcall__ getObjectSector(char o);
int __fastcall__ getObjectX(char o);
int __fastcall__ getObjectY(char o);
char __fastcall__ getObjectType(char o);
void __fastcall__ setObjectX(char o, int x);
void __fastcall__ setObjectY(char o, int y);
void __fastcall__ setObjectSector(char o, char sectorIndex);

char __fastcall__ getNumVerts(char sec);
char __fastcall__ getEdgeIndex(char sec, char i);
char __fastcall__ getEdgeSector(char sec, char i);
char __fastcall__ getOtherSector(char sec, char i);

char __fastcall__ getNumSectors(void);
signed char __fastcall__ getSectorVertexX(char sec, char i);
signed char __fastcall__ getSectorVertexY(char sec, char i);
