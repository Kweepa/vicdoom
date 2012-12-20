// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Enemy thinking, AI.
//	Action Pointer Functions
//	that are associated with states/frames. 
//
//-----------------------------------------------------------------------------
//
// Ported to the VIC-20 and cc65 in 2010-2012 by Steve McCrea
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <conio.h>

#include "p_enemy.h"

#include "playSound.h"
#include "p_enemy.h"
#include "player.h"
#include "mapAsm.h"
#include "util.h"
#include "fastmath.h"

#pragma staticlocals(on)

#define fixed_t int
#define boolean char
#define false 0
#define true 1

#define MELEERANGE 4

#define MF_JUSTATTACKED 1
#define MF_THOUGHTTHISFRAME 2
#define MF_WASSEENTHISFRAME 4

// states are specific to enemy types
#define STATE_POSCHASE 0
#define STATE_POSPAIN 1
#define STATE_POSSHOOT 2
#define STATE_POSFALL 3
#define STATE_IMPCHASE 4
#define STATE_IMPPAIN 5
#define STATE_IMPCLAW 6
#define STATE_IMPMISSILE 7
#define STATE_IMPFALL 8
#define STATE_DMNCHASE 9
#define STATE_DMNPAIN 10
#define STATE_DMNBITE 11
#define STATE_DMNFALL 12
#define STATE_CACCHASE 13
#define STATE_CACPAIN 14
#define STATE_CACCLAW 15
#define STATE_CACMISSILE 16
#define STATE_CACFALL 17
#define STATE_IMPSHOTFLY 18

typedef struct
{
   char speed;
   signed char seesound;
   signed char activesound;
   signed char painsound;
   signed char meleesound;
   signed char missilesound;

   char missiledamage;
   char spawnhealth;
   char painchance;

   signed char chasestate;
   signed char painstate;
   signed char meleestate;
   signed char shootstate;
   signed char deathstate;
   
   char deathObjectType;
}
mobjInfo_t;

#define MOBJINFO_POSSESSED 0
#define MOBJINFO_IMP 1
#define MOBJINFO_DEMON 2
#define MOBJINFO_CACODEMON 3
#define MOBJINFO_IMPSHOT 4

mobjInfo_t mobjinfo[] =
{
  { 3, -1, SOUND_GURGLE, SOUND_POPAIN, -1, SOUND_PISTOL, 0, 10, 2,
    STATE_POSCHASE, STATE_POSPAIN, -1, STATE_POSSHOOT, STATE_POSFALL, kOT_PossessedCorpseWithAmmo },
  { 4, -1, SOUND_GURGLE, SOUND_POPAIN, SOUND_CLAW, SOUND_CLAW, 10, 20, 3,
    STATE_IMPCHASE, STATE_IMPPAIN, STATE_IMPCLAW, STATE_IMPMISSILE, STATE_IMPFALL, kOT_ImpCorpse },
  { 6, -1, SOUND_GURGLE, SOUND_POPAIN, SOUND_CLAW, -1, 0, 20, 4,
    STATE_DMNCHASE, STATE_DMNPAIN, STATE_DMNBITE, -1, STATE_DMNFALL, kOT_DemonCorpse },
  { 5, -1, SOUND_GURGLE, SOUND_POPAIN, SOUND_CLAW, SOUND_CLAW, 30, 99, 5,
    STATE_CACCHASE, STATE_CACPAIN, STATE_CACCLAW, STATE_CACMISSILE, STATE_CACFALL, kOT_CacodemonCorpse },
};

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

void __fastcall__ A_Chase(void);
void __fastcall__ A_Flinch(void);
void __fastcall__ A_Melee(void);
void __fastcall__ A_Shoot(void);
void __fastcall__ A_Missile(void);
void __fastcall__ A_Fall(void);
void __fastcall__ A_Fly(void);

typedef void (*ActionFn)(void);

ActionFn actions[] = { A_Chase, A_Flinch, A_Melee, A_Shoot, A_Missile, A_Fall, A_Fly };

// actions are global
#define ACTION_CHASE 0
#define ACTION_FLINCH 1
#define ACTION_MELEE 2
#define ACTION_SHOOT 3
#define ACTION_MISSILE 4
#define ACTION_FALL 5
#define ACTION_FLY 6

typedef struct
{
   char texture;
   char actionIndex;
}
mobjState_t;

mobjState_t states[] =
{
  { TEX_ANIMATE + 8, ACTION_CHASE },
  { 10, ACTION_FLINCH },
  { 9, ACTION_SHOOT },
  { 10, ACTION_FALL },

  { TEX_ANIMATE + 11, ACTION_CHASE },
  { 13, ACTION_FLINCH },
  { 12, ACTION_MELEE },
  { 12, ACTION_MISSILE },
  { 13, ACTION_FALL },
  
  { TEX_ANIMATE + 14, ACTION_CHASE },
  { 16, ACTION_FLINCH },
  { 15, ACTION_MELEE },
  { 16, ACTION_FALL },

  { TEX_ANIMATE + 17, ACTION_CHASE },
  { 18, ACTION_FLINCH },
  { 17, ACTION_MELEE },
  { 17, ACTION_MISSILE },
  { 18, ACTION_FALL },
  
  { 0, ACTION_FLY }
};

mobj_t *actor;
mobjInfo_t *info;
mobjState_t *state;
char distanceFromPlayer;

char newChaseDirThisFrame = 0;

//#define PRINT_ACTION

#ifdef PRINT_ACTION
void __fastcall__ printAction(void)
{
   gotoxy(5,0);
   switch (state->actionIndex)
   {
   case ACTION_CHASE:
     cputs("chase. ");
     break;
   case ACTION_FLINCH:
     cputs("flinch. ");
     break;
   case ACTION_MELEE:
     cputs("melee. ");
     break;
   case ACTION_SHOOT:
     cputs("shoot. ");
     break;
   case ACTION_MISSILE:
     cputs("missile. ");
     break;
   case ACTION_FALL:
     cputs("fall. ");
     break;
 case ACTION_FLY:
     cputs("fly. ");
     break;
   }
}
#endif

char __fastcall__ getTexture(mobj_t *obj)
{
   return states[obj->stateIndex].texture;
}

char numMobj = 0;
// this needs to be one more than the value in the editor
// so that the imps can throw projectiles
#define MAX_MOBJ 21
// if you change this change the values in mapAsm.s
// for the object low bytes
#define MAX_OBJ 48

mobj_t mobjs[MAX_MOBJ];
char objForMobj[MAX_MOBJ];
char mobjForObj[MAX_OBJ];

char numEnemies;
char numKills;

char __fastcall__ p_enemy_getKillPercentage(void)
{
  return (100*numKills)/numEnemies;
}

void __fastcall__ p_enemy_resetMap(void)
{
  char i;
  for (i = 0; i < MAX_MOBJ; ++i)
  {
    mobjs[i].allocated = false;
  }
  numEnemies = 0;
  numKills = 0;
}

char allocated = 0;
char firstTime = 1;

char __fastcall__ allocMobj(char o)
{
  char i;
  mobj_t *mobj;
  mobjInfo_t *info;
//  if (allocated) return -1;
//  allocated = 1;
  for (i = 0; i < MAX_MOBJ; ++i)
  {
  	mobj = &mobjs[i];
    if (mobj->allocated == false)
    {
      ++numEnemies;
      objForMobj[i] = o;
      mobjForObj[o] = i;

      // temp to test
      // setObjectType(o, kOT_Demon);

      mobj->allocated = true;
      mobj->mobjIndex = i;
      mobj->x = getObjectX(o);
      mobj->y = getObjectY(o);
      mobj->momx = 0;
      mobj->momy = 0;
      mobj->sector = getObjectSector(o);
      mobj->movedir = 0;
      mobj->flags = 0;
      mobj->reactiontime = 2;
      mobj->movecount = 0;
      mobj->infoType = getObjectType(o);
      info = &mobjinfo[mobj->infoType];
      mobj->health = info->spawnhealth;
      mobj->stateIndex = info->chasestate;
      
      return i;
    }
  }
  return -1;
}

char thinkercap;
char thinkers[MAX_MOBJ];

void __fastcall__ p_enemy_add_thinker(char o);

void __fastcall__ p_enemy_startframe(void)
{
   char i;
   thinkercap = 0;
   for (i = 0; i < MAX_MOBJ; ++i)
   {
     actor = &mobjs[i];
     actor->flags &= ~(MF_WASSEENTHISFRAME|MF_THOUGHTTHISFRAME);
     // think dying dudes and projectiles
     if (actor->health <= 0 || i == MAX_MOBJ-1)
     {
       p_enemy_add_thinker(objForMobj[i]);
     }
   }
   
   newChaseDirThisFrame = 0;
}

void __fastcall__ p_enemy_add_thinker(char o)
{
  char t = getObjectType(o);
  if (t < 5)
  {
    char i = mobjForObj[o];
    if (!(mobjs[i].flags & MF_THOUGHTTHISFRAME))
    {
      mobjs[i].flags |= MF_THOUGHTTHISFRAME;
      thinkers[thinkercap] = i;
      ++thinkercap;
    }
  }
}

void __fastcall__ p_enemy_wasseenthisframe(char o)
{
   char t = getObjectType(o);
   if (t < 5)
   {
     char i = mobjForObj[o];
     mobjs[i].flags |= MF_WASSEENTHISFRAME;
   }
}

char __fastcall__ p_enemy_get_texture(char o)
{
  char i = mobjForObj[o];
  return getTexture(&mobjs[i]);
}

void __fastcall__ P_DamageMobj(char damage);

void __fastcall__ p_enemy_damage(char o, char damage)
{
   if (getObjectType(o) < 5)
   {
     char i = mobjForObj[o];
     actor = &mobjs[i];
     info = &mobjinfo[actor->infoType];
     P_DamageMobj(damage);
   }
}

void __fastcall__ p_enemy_single_think(char mobjIndex)
{
  actor = &mobjs[mobjIndex];
  info = &mobjinfo[actor->infoType];
  state = &states[actor->stateIndex];
  distanceFromPlayer = P_ApproxDistance(playerx - actor->x, playery - actor->y);
#ifdef PRINT_ACTION
  printAction();
#endif
  actions[state->actionIndex]();
}  

void __fastcall__ p_enemy_think(void)
{
  char i = 0;
  while (i < thinkercap)
  {
    char mobjIndex = thinkers[i];
    p_enemy_single_think(mobjIndex);
    ++i;
  }
  if (mobjs[MAX_MOBJ-1].allocated)
  {
    p_enemy_single_think(MAX_MOBJ-1);
  }
}

char P_Random(void);

enum EDirType
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
};
// enums are 16 bit, so don't use
#define dirtype_t char

//
// P_NewChaseDir related LUT.
//
dirtype_t opposite[] =
{
  DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
  DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

dirtype_t diags[] =
{
    DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};


//
// ENEMY THINKING
//


//
// P_CheckSight
//
// hacked version of this for D20M
//

boolean __fastcall__ P_CheckSight(void)
{
  if (actor->sector == playerSector) return true;
  // this table will be cleared at the start of render
  // and filled in during render
  if (actor->flags & MF_WASSEENTHISFRAME) return true;
  return false;
}

void __fastcall__ S_StartSound(char sound)
{
   // just try to play it
   // will succeed or fail based on priorities
   // TODO: perhaps set a volume based on actor position?
   playSound(sound);
}

void __fastcall__ P_SetMobjState(char stateIndex)
{
  actor->stateIndex = stateIndex;
  state = &states[stateIndex];
}

void __fastcall__ P_DamageMobj(char damage)
{
	actor->health -= damage;
	if (actor->health <= 0)
	{
	    // kill actor
		actor->movecount = 2;
		P_SetMobjState(info->deathstate);
		++numKills;
	}
	else
	{
		actor->flags |= MF_JUSTATTACKED;
		// maybe flinch, depending on threshold
		if (damage > info->painchance)
		{
		  actor->movecount = 1;
		  P_SetMobjState(info->painstate);
		}
	}
}

void __fastcall__ P_RadiusAttack(char radius)
{
   // attempt to damage the player
    if (distanceFromPlayer < radius)
    {
      playSound(SOUND_OOF);
      damagePlayer(actor->health + (P_Random()&15));
    }
}

//
// P_CheckMeleeRange
//
boolean __fastcall__ P_CheckMeleeRange(void)
{
  if (distanceFromPlayer >= MELEERANGE)
    return false;
	
  if (! P_CheckSight() )
	  return false;
							
  return true;
}

//
// P_CheckMissileRange
//
boolean __fastcall__ P_CheckMissileRange(void)
{
    char dist;

    if (actor->reactiontime)
  		return false;	// do not attack yet
		
    if (! P_CheckSight() )
		  return false;
	
	  dist = distanceFromPlayer;
		
    if (!info->meleestate && dist >= 20)
		  dist -= 20; // no melee attack, so fire more

    if (dist > 50)
  		dist = 50;
		
    if ((P_Random()>>2) < dist)
	  	return false;

    return true;
}

#define POKE(addr,val) ((*(unsigned char *)(addr)) = val)
#define PEEK(addr) (*(unsigned char *)(addr))

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//

signed char __fastcall__ try_move(int trydx, int trydy)
{
  // check the edges we can cross first
  // if any of them teleport us, move
  
  signed char thatSector;
  char i, ni;
  signed char v1x, v1y, v2x, v2y;
  signed char ex, ey;
  int px, py;
  long dot;
  long height;
  int edgeLen;
  int edgeLen2;
  int distance;
  char edgeGlobalIndex;
  char vertGlobalIndex, vert2GlobalIndex;
  char curSector = actor->sector;
  char sectorToReturn = curSector;
  char secNumVerts = getNumVerts(curSector);
  int tx = actor->x + trydx;
  int ty = actor->y + trydy;
  
  // see which edge the new coordinate is behind
  for (i = 0; i < secNumVerts; ++i)
  {
    ni = getNextEdge(curSector, i);
    vertGlobalIndex = getVertexIndex(curSector, i);
    vert2GlobalIndex = getVertexIndex(curSector, ni);
    v1x = getVertexX(vertGlobalIndex);
    v1y = getVertexY(vertGlobalIndex);
    v2x = getVertexX(vert2GlobalIndex);
    v2y = getVertexY(vert2GlobalIndex);
    ex = v2x - v1x;
    ey = v2y - v1y;
    // check if moving towards edge
    // dot = trydx*ey - trydy*ex;
    fastMultiplySetup16x8e24(ey);
    dot = fastMultiply16x8e24(trydx);
    fastMultiplySetup16x8e24(ex);
    dot -= fastMultiply16x8e24(trydy);
    if (dot <= 0)
    {
      px = tx - (((short)v1x)<<8);
      py = ty - (((short)v1y)<<8);
      edgeGlobalIndex = getEdgeIndex(curSector, i);
      edgeLen = getEdgeLen(edgeGlobalIndex);
      //height = px * ey - py * ex;
      fastMultiplySetup16x8e24(ey);
      height = fastMultiply16x8e24(px);
      fastMultiplySetup16x8e24(ex);
      height -= fastMultiply16x8e24(py);

      fastMultiplySetup8x8(edgeLen);
      edgeLen2 = fastMultiply8x8(edgeLen);

      if (height < (edgeLen2<<1))
      {
			  // check we're within the extents of the edge
			  thatSector = getOtherSector(edgeGlobalIndex, curSector);
			  if (thatSector != -1 && !isDoorClosed(edgeGlobalIndex))
			  {
			    //distance = px * ex + py * ey;
          fastMultiplySetup16x8e24(ex);
          distance = fastMultiply16x8e24(px);
          fastMultiplySetup16x8e24(ey);
          distance += fastMultiply16x8e24(py);
          if (distance > edgeLen2 && distance < (edgeLen2*edgeLen - edgeLen2))
          {
            #if 0
            gotoxy(0,16);
            cprintf("%d %d %d %d %d. ", curSector, distance, edgeLen, dot, height);
            #endif
            if (height <= 0)
            {
              #if 0
              gotoxy(0,4);
              cprintf("%d. ", thatSector);
              #endif
              return thatSector;
            }
            return curSector;
          }
          else
          {
            // hit a wall
            sectorToReturn = -1;
          }
			  }
			  else
			  {
			    // hit a wall
			    sectorToReturn = -1;
			  }
      }
	  }
  }
  return sectorToReturn;
}

boolean __fastcall__ P_TryMove(fixed_t trydx, fixed_t trydy)
{
   // check the move is valid
   signed char nextSector = try_move(trydx, trydy);
   if (nextSector != -1)
   {
     char o = objForMobj[actor->mobjIndex];
     actor->x += trydx;
     actor->y += trydy;

     // also, copy the position to the object
     setObjectX(o, actor->x);
     setObjectY(o, actor->y);
     
     // and, update the sector!
     if (actor->sector != nextSector)
     {
        actor->sector = nextSector;
        setObjectSector(o, nextSector);
     }
     
     return true;
   }

   return false;
}

#define MIN_SPEED 32
#define FU_45 22
signed char xspeed[8] = {MIN_SPEED,FU_45,0,-FU_45,-MIN_SPEED,-FU_45,0,FU_45};
signed char yspeed[8] = {0,FU_45,MIN_SPEED,FU_45,0,-FU_45,-MIN_SPEED,-FU_45};

boolean __fastcall__ P_Move(void)
{
    fixed_t	trydx;
    fixed_t	trydy;
    
    // warning: 'catch', 'throw', and 'try'
    // are all C++ reserved words
		
    if (actor->movedir == DI_NODIR)
    {
    	return false;
    }

    if (distanceFromPlayer < MELEERANGE)
    {
      return true;
    }

    trydx = info->speed*xspeed[actor->movedir];
    trydy = info->speed*yspeed[actor->movedir];

    return P_TryMove(trydx, trydy);
}


//
// TryWalk
// Attempts to move actor on
// in its current (obj->movedir) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
boolean __fastcall__ P_TryWalk(void)
{	
    if (!P_Move())
    {
	   return false;
    }

    actor->movecount = 3 + (P_Random()&3); // was 15!
    return true;
}



#define CHASEDIST 256

void __fastcall__ P_NewChaseDir(void)
{
    int deltax, deltay;
    char d1, d2, olddir, newdir, turnaround;
    
    if (newChaseDirThisFrame != 0) return;
    newChaseDirThisFrame = 1;

    olddir = actor->movedir;
    newdir = DI_NODIR;
    turnaround = opposite[olddir];

    deltax = playerx - actor->x;
    deltay = playery - actor->y;

    if (deltax > CHASEDIST) d1 = DI_EAST;
    else if (deltax < -CHASEDIST) d1 = DI_WEST;
    else d1 = DI_NODIR;

    if (deltay < -CHASEDIST) d2 = DI_SOUTH;
    else if (deltay > CHASEDIST) d2 = DI_NORTH;
    else d2 = DI_NODIR;
	
    // try direct diagonal route
    if (d1 != DI_NODIR && d2 != DI_NODIR)
    {
		newdir = diags[((deltay < 0)<<1) + (deltax > 0)];
		if (newdir == turnaround)
		{
		  newdir = DI_NODIR;
		}
	}
    if (newdir == DI_NODIR)
    {
		if (d1 == turnaround) d1 = DI_NODIR;
		if (d2 == turnaround) d2 = DI_NODIR;
		if (d1 != DI_NODIR)
		{
		  newdir = d1;
		}
		else if (d2 != DI_NODIR)
		{
		  newdir = d2;
		}
		else
		{
		  newdir = P_Random() & 7;
		}
	}

	actor->movedir = newdir;
	if (!P_TryWalk())
	{
		actor->movedir = P_Random() & 7;
		actor->movecount = 3;
	}
}


//
// ACTION ROUTINES
//


//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void __fastcall__ A_Chase(void)
{
    if (actor->reactiontime) actor->reactiontime--;
				
    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
  		actor->flags &= ~MF_JUSTATTACKED;
	    P_NewChaseDir();
	  	return;
    }
    
    // check for melee attack
    if (info->meleestate != -1
		  && P_CheckMeleeRange())
    {
      actor->movecount = 0;
		  P_SetMobjState(info->meleestate);
		  return;
    }
    
    // check for missile attack
    if (info->shootstate != -1)
    {
      if (actor->movecount)
      {
        goto nomissile;
      }

      if (!P_CheckMissileRange())
			  goto nomissile;
		
		  P_SetMobjState(info->shootstate);
		  actor->flags |= MF_JUSTATTACKED;
		  return;
    }

    // ?
nomissile:

    // chase towards player
    if (--actor->movecount < 0
	  	|| !P_Move())
    {
  		P_NewChaseDir();
    }

    // make active sound
    if (info->activesound != -1
      && P_Random() < 3)
    {
      S_StartSound(info->activesound);
    }
}

//
// A_Shoot
//
void __fastcall__ A_Shoot(void)
{
  char	damage;
  char dist = distanceFromPlayer;
	
  S_StartSound(info->missilesound);
  if (dist > 28) dist = 28;
  if ((P_Random()&31) > dist)
  {
	  damage = ((P_Random()&3)+2)*3; // this was ((r%5)+1)*3
	  damagePlayer(damage);
	}
	actor->reactiontime = P_Random()&7;
	P_SetMobjState(info->chasestate);
}

//
// A_Missile
//
void __fastcall__ A_Missile(void)
{
	// launch a missile
	{
	  mobj_t *miss = &mobjs[MAX_MOBJ-1];
	  if (miss->allocated == false)
	  {
	    long dx = playerx - actor->x;
	    long dy = playery - actor->y;
	    int distance = sqrt(dx*dx + dy*dy)/64;
	    dx /= distance;
	    dy /= distance;

	    miss->allocated = true;
	    miss->x = actor->x;
	    miss->y = actor->y;
	    miss->momx = dx;
	    miss->momy = dy;
	    miss->sector = actor->sector;
	    miss->infoType = MOBJINFO_IMPSHOT;
	    miss->stateIndex = STATE_IMPSHOTFLY;
	    miss->mobjIndex = MAX_MOBJ - 1;
      miss->health = mobjinfo[actor->infoType].missiledamage;

#if 0
	    gotoxy(1,1);
	    cprintf("%ld %ld %d %d. \n", dx, dy, miss->momx, miss->momy);
#endif
	    objForMobj[MAX_MOBJ - 1] = MAX_OBJ - 1;
	    mobjForObj[MAX_OBJ - 1] = MAX_MOBJ - 1;
	    setObjectSector(MAX_OBJ - 1, miss->sector);
	    setObjectX(MAX_OBJ - 1, miss->x);
	    setObjectY(MAX_OBJ - 1, miss->y);
	    setObjectType(MAX_OBJ - 1, kOT_ImpShot);
	  }
	}
	actor->reactiontime = P_Random()&7;
	P_SetMobjState(info->chasestate);
}

//
// A_Melee
//
void __fastcall__ A_Melee(void)
{
  if (actor->movecount == 0)
  {
    char damage = ((P_Random()&7)+1)*3;
		
    if (info->meleesound != -1)
	    S_StartSound(info->meleesound);
    damagePlayer(damage);
		
    ++actor->movecount;
  }
  else
  {
    P_SetMobjState(info->chasestate);
  }
}

char cacodemonsDead = 0;

void __fastcall__ A_Fall(void)
{
   if (--actor->movecount == 0)
   {
     // make the object into a static corpse
     char o = objForMobj[actor->mobjIndex];
     setObjectType(o, info->deathObjectType);
     if (info->deathObjectType == kOT_CacodemonCorpse)
     {
       // count to 2
       ++cacodemonsDead;
       // open door when done
       if (cacodemonsDead == 2)
       {
         // edge 3 is the door to the final switch
         basicOpenDoor(3);
         playSound(SOUND_DOROPN);
       }
     }
     actor->allocated = false;
   }
}

void __fastcall__ A_Flinch(void)
{
  if (--actor->movecount == 0)
  {
    P_SetMobjState(info->chasestate);
  }
}

//
// A_Fly
//

void __fastcall__ A_Fly(void)
{
   boolean die = false;
   if (distanceFromPlayer < 3)
   {
     die = true;
     damagePlayer(actor->health + (P_Random()&15));
     playSound(SOUND_OOF);
   }
   else
   {
	   int trydx = ((int)actor->momx)<<2;
	   int trydy = ((int)actor->momy)<<2;
	   if (!P_TryMove(trydx, trydy))
	   {
	      die = true;
  	      // explode
	      P_RadiusAttack(4);
	   }
   }
   if (die)
   {
	  char o = objForMobj[actor->mobjIndex];
	  setObjectSector(o, -1);
	  actor->allocated = false;
   }
}
