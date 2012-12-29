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
#include "player.h"
#include "mapAsm.h"
#include "util.h"
#include "fastmath.h"
#include "enemy.h"

#pragma staticlocals(on)

#define fixed_t int
#define boolean char
#define false 0
#define true 1

#define MELEERANGE 4

#define MF_JUSTATTACKED 1
#define MF_THOUGHTTHISFRAME 2
#define MF_WASSEENTHISFRAME 4

#define MOBJINFO_POSSESSED 0
#define MOBJINFO_IMP 1
#define MOBJINFO_DEMON 2
#define MOBJINFO_CACODEMON 3
#define MOBJINFO_IMPSHOT 4

#define STATE_IMPSHOTFLY 18 // keep in sync with enemy.s

// this needs to be one more than the value in the editor
// so that the imps can throw projectiles
#define MAX_MOBJ 21

int missile_momx;
int missile_momy;

void  A_Chase(void);
void  A_Flinch(void);
void  A_Melee(void);
void  A_Shoot(void);
void  A_Missile(void);
void  A_Fall(void);
void  A_Fly(void);

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

// see enemy.s for the states

char state_texture[] =
{
  TEX_ANIMATE + 8,
  10,
  9,
  10,

  TEX_ANIMATE + 11,
  13,
  12,
  12,
  13,

  TEX_ANIMATE + 14,
  16,
  15,
  16,

  TEX_ANIMATE + 17,
  18,
  17,
  17,
  18,
  
  0
};

char state_actionIndex[] =
{
  ACTION_CHASE,
  ACTION_FLINCH,
  ACTION_SHOOT,
  ACTION_FALL,

  ACTION_CHASE,
  ACTION_FLINCH,
  ACTION_MELEE,
  ACTION_MISSILE,
  ACTION_FALL,
  
  ACTION_CHASE,
  ACTION_FLINCH,
  ACTION_MELEE,
  ACTION_FALL,

  ACTION_CHASE,
  ACTION_FLINCH,
  ACTION_MELEE,
  ACTION_MISSILE,
  ACTION_FALL,
  
  ACTION_FLY
};


char actorIndex;
char objIndex;
char actionIndex;
char distanceFromPlayer;

char newChaseDirThisFrame = 0;

//#define PRINT_ACTION

#ifdef PRINT_ACTION
void __fastcall__ printAction(void)
{
   gotoxy(0,10);
   switch (actionIndex)
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

char numMobj = 0;
// if you change this change the values in mapAsm.s
#define MAX_OBJ 48

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
    setMobjIndex(i);
    setMobjAllocated(0);
  }
  numEnemies = 0;
  numKills = 0;
}

//char numAllocated = 0;
char firstTime = 1;

char __fastcall__ allocMobj(char o)
{
  char i;
//  ++numAllocated;
//  if (numAllocated > 2)
//  {
//    return -1;
//  }
  for (i = 0; i < MAX_MOBJ; ++i)
  {
    if (!mobjAllocated(i))
    {
      char ot;
      ++numEnemies;
      setObjForMobj(o, i);

      // temp to test
      // setObjectType(o, kOT_Demon);

      setMobjIndex(i);
      setMobjAllocated(1);
      setMobjMovedir(0);
      setMobjFlags(0);
      setMobjReactiontime(2);
      setMobjMovecount(0);
      ot = getObjectType(o);
      setMobjInfoType(ot);
      setMobjCurrentType(ot);
      setMobjHealth(getMobjSpawnHealth());
      setMobjStateIndex(getMobjChaseState());
      
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
     setMobjIndex(i);
     removeMobjFlags(MF_WASSEENTHISFRAME|MF_THOUGHTTHISFRAME);
     // think dying dudes and projectiles
     if (mobjAllocated(i))
     {
       if (mobjHealth() <= 0 || i == MAX_MOBJ-1)
       {
         p_enemy_add_thinker(objForMobj(i));
       }
     }
   }
   
   newChaseDirThisFrame = 0;
}

void __fastcall__ p_enemy_add_thinker(char o)
{
  char t = getObjectType(o);
  if (t < 5)
  {
    char i = mobjForObj(o);
    setMobjIndex(i);
    if (!testMobjFlags(MF_THOUGHTTHISFRAME))
    {
      addMobjFlags(MF_THOUGHTTHISFRAME);
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
    char i = mobjForObj(o);
    setMobjIndex(i);
    addMobjFlags(MF_WASSEENTHISFRAME);
  }
}

char __fastcall__ p_enemy_get_texture(char o)
{
  char i = mobjForObj(o);
  char si;
  setMobjIndex(i);
  si = mobjStateIndex();
  return state_texture[si];
}

void __fastcall__ P_DamageMobj(char damage);

void __fastcall__ p_enemy_damage(char o, char damage)
{
  char ot = getObjectType(o);
  if (ot < 5)
  {
    actorIndex = mobjForObj(o);
    setMobjIndex(actorIndex);
    setMobjCurrentType(ot);
    P_DamageMobj(damage);
  }
}

void __fastcall__ p_enemy_single_think(char mobjIndex)
{
  char ot, si;
  actorIndex = mobjIndex;
  objIndex = objForMobj(actorIndex);
  setMobjIndex(mobjIndex);
  ot = mobjInfoType();
  setMobjCurrentType(ot);
  si = mobjStateIndex();
  actionIndex = state_actionIndex[si];
  distanceFromPlayer = P_ApproxDistance(playerx - getObjectX(objIndex), playery - getObjectY(objIndex));
#ifdef PRINT_ACTION
  printAction();
#endif
//  print3DigitNumToScreen(mobjIndex, 0x1000 + 22*4);
//  print3DigitNumToScreen(objIndex, 0x1000 + 22*5);
//  print3DigitNumToScreen(getObjectSector(objIndex), 0x1000 + 22*6);
//  print3DigitNumToScreen(mobjStateIndex(), 0x1000 + 22*7);
//  print3DigitNumToScreen(actionIndex, 0x1000 + 22*8);
  actions[actionIndex]();
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
  if (mobjAllocated(MAX_MOBJ-1))
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

boolean P_CheckSight(void)
{
  if (getObjectSector(objIndex) == playerSector) return true;
  // this table will be cleared at the start of render
  // and filled in during render
  if (testMobjFlags(MF_WASSEENTHISFRAME)) return true;
  return false;
}

   // just try to play it
   // will succeed or fail based on priorities
   // TODO: perhaps set a volume based on actor position?
#define S_StartSound playSound
#define P_SetMobjState setMobjStateIndex


void __fastcall__ P_DamageMobj(char damage)
{
	setMobjHealth(mobjHealth() - damage);
	if (mobjHealth() <= 0)
	{
	    // kill actor
		setMobjMovecount(2);
    playSound(SOUND_POPAIN);
		P_SetMobjState(getMobjDeathState());
		++numKills;
	}
	else
	{
		addMobjFlags(MF_JUSTATTACKED);
		// maybe flinch, depending on threshold
		if (damage > getMobjPainChance())
		{
		  setMobjMovecount(1);
		  P_SetMobjState(getMobjPainState());
		}
	}
}

void __fastcall__ P_RadiusAttack(char radius)
{
  // attempt to damage the player
  if (distanceFromPlayer < radius)
  {
    playSound(SOUND_OOF);
    damagePlayer(mobjHealth() + (P_Random()&15));
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

  if (mobjReactiontime())
    return false;	// do not attack yet
		
  if (! P_CheckSight() )
	  return false;
	
  dist = distanceFromPlayer;

#if 0
  if (!info->meleestate && dist >= 20)
	  dist -= 20; // no melee attack, so fire more
#endif

  if (dist > 50)
    dist = 50;
		
  if ((P_Random()>>2) < dist)
	  return false;

  return true;
}

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
  char curSector = getObjectSector(objIndex);
  char sectorToReturn = curSector;
  char secNumVerts = getNumVerts(curSector);
  int tx = getObjectX(objIndex) + trydx;
  int ty = getObjectY(objIndex) + trydy;
  
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
     setObjectX(objIndex, getObjectX(objIndex) + trydx);
     setObjectY(objIndex, getObjectY(objIndex) + trydy);

     // and, update the sector!
     if (getObjectSector(objIndex) != nextSector)
     {
       setObjectSector(objIndex, nextSector);
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
    char moveDir = mobjMovedir();
    char speed = getMobjSpeed();
    
    // warning: 'catch', 'throw', and 'try'
    // are all C++ reserved words
		
    if (moveDir == DI_NODIR)
    {
    	return false;
    }

    if (distanceFromPlayer < MELEERANGE)
    {
      return true;
    }

    trydx = speed*xspeed[moveDir];
    trydy = speed*yspeed[moveDir];

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

  setMobjMovecount(3 + (P_Random()&3)); // was 15!
  return true;
}



void __fastcall__ P_NewChaseDir(void)
{
  signed char deltax, deltay;
  char d1, d2, olddir, newdir, turnaround;
    
  if (newChaseDirThisFrame != 0) return;
  newChaseDirThisFrame = 1;

  olddir = mobjMovedir();
  newdir = DI_NODIR;
  turnaround = opposite[olddir];

  deltax = (playerx - getObjectX(objIndex))>>8;
  deltay = (playery - getObjectY(objIndex))>>8;

  {
    if (deltax > 0) d1 = DI_EAST;
    else if (deltax < -1) d1 = DI_WEST;
    else d1 = DI_NODIR;
  }

  {
    if (deltay < -1) d2 = DI_SOUTH;
    else if (deltay > 0) d2 = DI_NORTH;
    else d2 = DI_NODIR;
  }
	
  // try direct diagonal route
  if (d1 != DI_NODIR && d2 != DI_NODIR)
  {
		newdir = diags[((deltay < 0)<<1) + (deltax >= 0)];
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

	setMobjMovedir(newdir);
	if (!P_TryWalk())
	{
		setMobjMovedir(P_Random() & 7);
		setMobjMovecount(3);
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
void A_Chase(void)
{
  if (mobjReactiontime()) decMobjReactiontime();

  // do not attack twice in a row
  if (testMobjFlags(MF_JUSTATTACKED))
  {
  	removeMobjFlags(MF_JUSTATTACKED);
	  P_NewChaseDir();
	  return;
  }
    
  // check for melee attack
  if (getMobjMeleeState() != 0xff
		&& P_CheckMeleeRange())
  {
    setMobjMovecount(0);
		P_SetMobjState(getMobjMeleeState());
		return;
  }
    
  // check for missile attack
  if (getMobjShootState() != 0xff)
  {
    if (mobjMovecount())
    {
      goto nomissile;
    }

    if (!P_CheckMissileRange())
			goto nomissile;
		
		P_SetMobjState(getMobjShootState());
    addMobjFlags(MF_JUSTATTACKED);
		return;
  }

    // ?
nomissile:

  // chase towards player
  if (decMobjMovecount() < 0
	  || !P_Move())
  {
  	P_NewChaseDir();
  }

  // make active sound
  if (P_Random() < 3)
  {
    S_StartSound(SOUND_GURGLE);
  }
}

//
// A_Shoot
//
void A_Shoot(void)
{
  char	damage;
  char dist = distanceFromPlayer;
	
  S_StartSound(SOUND_PISTOL);
  if (dist > 28) dist = 28;
  if ((P_Random()&31) > dist)
  {
    damage = (P_Random()&3)+2;
	  damage += (damage<<1); // this was ((r%5)+1)*3
	  damagePlayer(damage);
	}
	setMobjReactiontime(P_Random()&7);
	P_SetMobjState(getMobjChaseState());
}

//
// A_Missile
//
void A_Missile(void)
{
	// launch a missile
	{
    if (mobjAllocated(MAX_MOBJ-1) == false)
	  {
      char ot = mobjInfoType();
      char missileDamage = (ot == MOBJINFO_IMP ? 10 : 30);
      char enemyIndex;

      // about 256 bytes!
      long dx = (playerx - getObjectX(objIndex))/16;
	    long dy = (playery - getObjectY(objIndex))/16;
	    unsigned int distance = sqrt24(dx*dx + dy*dy)/64;
      if (distance == 0) distance = 1;
	    missile_momx = dx/distance;
	    missile_momy = dy/distance;
      missile_momx <<= 2;
      missile_momy <<= 2;

	    //miss->allocated = true;
	    setObjectX(MAX_OBJ-1, getObjectX(objIndex));
	    setObjectY(MAX_OBJ-1, getObjectY(objIndex));
	    setObjectSector(MAX_OBJ-1, getObjectSector(objIndex));
      enemyIndex = actorIndex;
      setMobjIndex(MAX_MOBJ-1);
        setMobjAllocated(1);
	      setMobjInfoType(MOBJINFO_IMPSHOT);
	      setMobjStateIndex(STATE_IMPSHOTFLY);
        setMobjHealth(missileDamage);
        setMobjMovecount(32);
      setMobjIndex(enemyIndex);

#if 0
	    gotoxy(1,1);
	    cprintf("%ld %ld %d %d. \n", dx, dy, miss->momx, miss->momy);
#endif
      setObjForMobj(MAX_OBJ - 1, MAX_MOBJ - 1);
	    setObjectType(MAX_OBJ - 1, kOT_ImpShot);
	  }
	}
	setMobjReactiontime(P_Random()&7);
	P_SetMobjState(getMobjChaseState());
}

//
// A_Melee
//
void A_Melee(void)
{
  if (mobjMovecount() == 0)
  {
    char damage = ((P_Random()&7)+1)*3;
		
	  S_StartSound(SOUND_CLAW);
    damagePlayer(damage);
		
    incMobjMovecount();
  }
  else
  {
    P_SetMobjState(getMobjChaseState());
  }
}

char cacodemonsDead = 0;

void A_Fall(void)
{
   if (decMobjMovecount() == 0)
   {
     // make the object into a static corpse
     char o = objForMobj(actorIndex);
     char ot = mobjInfoType();
     setObjectType(o, kOT_PossessedCorpseWithAmmo + ot);
     if (ot == MOBJINFO_CACODEMON)
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
     setMobjAllocated(0);
   }
}

void A_Flinch(void)
{
  if (decMobjMovecount() <= 0)
  {
    P_SetMobjState(getMobjChaseState());
  }
}

//
// A_Fly
//

void A_Fly(void)
{
  boolean die = false;
  if (distanceFromPlayer < 3)
  {
    die = true;
    damagePlayer(mobjHealth() + (P_Random()&15));
    playSound(SOUND_OOF);
  }
  else if (!P_TryMove(missile_momx, missile_momy))
  {
    die = true;
      // explode
    P_RadiusAttack(4);
  }
  else if (decMobjMovecount() < 0)
  {
    die = true;
  }
  if (die)
  {
    char o = objForMobj(actorIndex);
    setObjectSector(o, -1);
    setMobjAllocated(0);
  }
}
