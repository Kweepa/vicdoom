typedef struct
{
   char speed;
   char seesound;
   char activesound;
   char painsound;
   char meleesound;
   char missilesound;
   
   char spawnhealth;
   char painchance;

   char spawnstate;
   char chasestate;
   char painstate;
   char meleestate;
   char shootstate;
   char deathstate;
}
mobjInfo_t;

#define STATE_POSLOOK 0
#define STATE_POSCHASE 1
#define STATE_POSPAIN 2
#define STATE_POSSHOOT 3
#define STATE_POSFALL 4

#define MOBJINFO_POSSESSED 0

typedef struct
{
   char allocated;
   char mobjIndex;
   int x;
   int y;
   signed char momx;
   signed char momy;
   char sector;
   char movedir;
   char flags;
   char reactiontime;
   signed char movecount;
   signed char health;
   char infoType;
   char stateIndex;
}
mobj_t;

extern mobj_t mobjs[];

char allocMobj(void);
