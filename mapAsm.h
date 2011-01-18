enum EObjType
{
   kOT_Possessed,
   kOT_Imp,
   kOT_Demon,
   kOT_Cacodemon,
   kOT_Baron,
   kOT_GreenArmor,
   kOT_BlueArmor,
   kOT_Bullets,
   kOT_Medkit,
   kOT_RedKeycard,
   kOT_GreenKeycard,
   kOT_BlueKeycard,
   kOT_Barrel,
   kOT_Pillar,
   kOT_Skullpile,
   kOT_Acid,
   kOT_PossessedCorpse,
   kOT_ImpCorpse,
   kOT_DemonCorpse,
   kOT_CacodemonCorpse,
   kOT_BaronCorpse,
   kOT_ImpShot
};

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
void __fastcall__ setObjectType(char o, char type);

char __fastcall__ getNumVerts(char sec);
char __fastcall__ getEdgeIndex(char sec, char i);
char __fastcall__ getEdgeSector(char i);
char __fastcall__ getOtherSector(char i, char sec);
char __fastcall__ getEdgeLen(char i);
char __fastcall__ getEdgeTexture(char i);
char __fastcall__ getNextEdge(char sec, char i);

char __fastcall__ getNumSectors(void);
char __fastcall__ getVertexIndex(char sec, char i);
signed char __fastcall__ getVertexX(char i);
signed char __fastcall__ getVertexY(char i);

int __fastcall__ getPlayerSpawnX(void);
int __fastcall__ getPlayerSpawnY(void);
char __fastcall__ getPlayerSpawnAngle(void);
char __fastcall__ getPlayerSpawnSector(void);

void __fastcall__ addObjectsToSectors(void);
void __fastcall__ addObjectToSector(char sec, char i);
void __fastcall__ removeObjectFromSector(char i);
char __fastcall__ getFirstObjectInSector(char sec);
char __fastcall__ getNextObjectInSector(char i);

char * __fastcall__ getMapName(void);

char __fastcall__ getNumEnemies(void);
char __fastcall__ getNumItems(void);
char __fastcall__ getNumSecrets(void);
char __fastcall__ getParTime(void);

void __fastcall__ resetSectorsVisited(void);
void __fastcall__ setSectorVisited(int i);
char __fastcall__ getNumVisitedSecrets(void);
