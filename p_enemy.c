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

#include <stdlib.h>

#include "playSound.h"

#define fixed_t int
#define boolean char
#define false 0
#define true 1
#define FRACUNIT 256

// player radius for movement checking
#define PLAYERRADIUS	(16*FRACUNIT) // for reference
#define MELEERANGE		(64*FRACUNIT)

#define sfx_pistol SOUND_DPPISTOL
#define sfx_claw SOUND_DPCLAW

#define MF_JUSTHIT 1
#define MF_JUSTATTACKED 2
#define MF_SHOOTABLE 4
#define MF_AMBUSH 8
#define MF_SOLID 16
#define MF_WASSEENTHISFRAME 32

#define MT_TROOPSHOT 6

typedef struct
{
   char texture;
   char animate;
   
   void (*action)(struct mobj_T *);
}
mobjState_t;

mobjState_t states[] =
{
};

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

// TODO: fill out
mobjInfo_t mobjinfo[1];

typedef struct mobj_T
{
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
   mobjInfo_t *info;
   struct mobj_T *target;
   
   void (*action)(struct mobj_T *);
}
mobj_t;

mobj_t *player;

char P_Random(void);

//
// P_ApproxDistance
// Gives an estimation of distance (not exact)
//

fixed_t
P_ApproxDistance
( fixed_t	dx,
  fixed_t	dy )
{
    dx = abs(dx);
    dy = abs(dy);
    if (dx < dy)
		return dx+dy-(dx>>1);
    return dx+dy-(dy>>1);
}


typedef enum
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
    
} dirtype_t;


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
// Enemies are allways spawned
// with targetplayer = -1
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// P_CheckSight & P_LookForPlayers
//
// hacked version of this for D20M
//

boolean P_CheckSight(mobj_t *actor, mobj_t *other)
{
  if (actor->sector == other->sector) return true;
  // this table will be cleared at the start of render
  // and filled in during render
  if (actor->flags & MF_WASSEENTHISFRAME) return true;
  return false;
}

boolean P_LookForPlayers(mobj_t *actor)
{
	if (P_CheckSight(actor, player))
	{
		actor->target = player;
		return true;
	}
	return false;
}

void S_StartSound(mobj_t *actor, char sound)
{
   // just try to play it
   // will succeed or fail based on priorities
   playSound(sound);
}

void P_SetMobjState(mobj_t *actor, char state)
{
}

void P_DamageMobj(mobj_t *actor, mobj_t *damager, mobj_t *owner, int damage)
{
	actor->health -= damage;
	if (actor->health <= 0)
	{
		// kill actor - FIX!
		P_SetMobjState(actor, actor->info->deathstate);
	}
	else
	{
		actor->flags |= MF_JUSTATTACKED;
	}
}

void P_RadiusAttack(mobj_t *actor, mobj_t *target, int dist)
{
}

//
// P_CheckMeleeRange
//
boolean P_CheckMeleeRange (mobj_t*	actor)
{
    mobj_t*	pl;
    fixed_t	dist;
	
    if (!actor->target)
	return false;
		
    pl = actor->target;
    dist = P_ApproxDistance (pl->x-actor->x, pl->y-actor->y);

    if (dist >= MELEERANGE) //-20*FRACUNIT+pl->info->radius)
	return false;
	
    if (! P_CheckSight (actor, actor->target) )
	return false;
							
    return true;		
}

//
// P_CheckMissileRange
//
boolean P_CheckMissileRange (mobj_t* actor)
{
    fixed_t	dist;
	
    if (! P_CheckSight (actor, actor->target) )
	return false;
	
    if ( actor->flags & MF_JUSTHIT )
    {
	// the target just hit the enemy,
	// so fight back!
	actor->flags &= ~MF_JUSTHIT;
	return true;
    }
	
    if (actor->reactiontime)
	return false;	// do not attack yet
		
    // OPTIMIZE: get this from a global checksight
    dist = P_ApproxDistance ( actor->x-actor->target->x,
			     actor->y-actor->target->y) - 64*FRACUNIT;

    if (!actor->info->meleestate)
	dist -= 128*FRACUNIT;	// no melee attack, so fire more

    dist >>= 16;
    
    if (dist > 200)
	dist = 200;
		
    if (P_Random () < dist)
	return false;
		
    return true;
}


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
#define FU_45 181
fixed_t	xspeed[8] = {FRACUNIT,FU_45,0,-FU_45,-FRACUNIT,-FU_45,0,FU_45};
fixed_t yspeed[8] = {0,FU_45,FRACUNIT,FU_45,0,-FU_45,-FRACUNIT,-FU_45};

boolean P_TryMove(mobj_t *actor, fixed_t tryx, fixed_t tryy);

boolean P_Move (mobj_t*	actor)
{
    fixed_t	tryx;
    fixed_t	tryy;
    
    // warning: 'catch', 'throw', and 'try'
    // are all C++ reserved words
		
    if (actor->movedir == DI_NODIR)
	return false;
		
    tryx = actor->x + actor->info->speed*xspeed[actor->movedir];
    tryy = actor->y + actor->info->speed*yspeed[actor->movedir];

	// need to replace this function with one of my own devising
	// should just check the lines of this sector
    return P_TryMove (actor, tryx, tryy);
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
boolean P_TryWalk (mobj_t* actor)
{	
    if (!P_Move (actor))
    {
	   return false;
    }

    actor->movecount = P_Random()&15;
    return true;
}




void P_NewChaseDir (mobj_t*	actor)
{
    fixed_t	deltax;
    fixed_t	deltay;
    
    dirtype_t	d[3];
    
    int		tdir;
    dirtype_t	olddir;
    
    dirtype_t	turnaround;

    olddir = actor->movedir;
    turnaround=opposite[olddir];

    deltax = actor->target->x - actor->x;
    deltay = actor->target->y - actor->y;

    if (deltax>10*FRACUNIT)
	d[1]= DI_EAST;
    else if (deltax<-10*FRACUNIT)
	d[1]= DI_WEST;
    else
	d[1]=DI_NODIR;

    if (deltay<-10*FRACUNIT)
	d[2]= DI_SOUTH;
    else if (deltay>10*FRACUNIT)
	d[2]= DI_NORTH;
    else
	d[2]=DI_NODIR;

    // try direct route
    if (d[1] != DI_NODIR
	&& d[2] != DI_NODIR)
    {
	actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
	if (actor->movedir != turnaround && P_TryWalk(actor))
	    return;
    }

    // try other directions
    if (P_Random() > 200
	||  abs(deltay)>abs(deltax))
    {
	tdir=d[1];
	d[1]=d[2];
	d[2]=tdir;
    }

    if (d[1]==turnaround)
	d[1]=DI_NODIR;
    if (d[2]==turnaround)
	d[2]=DI_NODIR;
	
    if (d[1]!=DI_NODIR)
    {
	actor->movedir = d[1];
	if (P_TryWalk(actor))
	{
	    // either moved forward or attacked
	    return;
	}
    }

    if (d[2]!=DI_NODIR)
    {
	actor->movedir =d[2];

	if (P_TryWalk(actor))
	    return;
    }

    // there is no direct path to the player,
    // so pick another direction.
    if (olddir!=DI_NODIR)
    {
	actor->movedir =olddir;

	if (P_TryWalk(actor))
	    return;
    }

    // randomly determine direction of search
    if (P_Random()&1) 	
    {
	for ( tdir=DI_EAST;
	      tdir<=DI_SOUTHEAST;
	      tdir++ )
	{
	    if (tdir!=turnaround)
	    {
		actor->movedir =tdir;
		
		if ( P_TryWalk(actor) )
		    return;
	    }
	}
    }
    else
    {
	for ( tdir=DI_SOUTHEAST;
	      tdir != (DI_EAST-1);
	      tdir-- )
	{
	    if (tdir!=turnaround)
	    {
		actor->movedir =tdir;
		
		if ( P_TryWalk(actor) )
		    return;
	    }
	}
    }

    if (turnaround !=  DI_NODIR)
    {
	actor->movedir =turnaround;
	if ( P_TryWalk(actor) )
	    return;
    }

    actor->movedir = DI_NODIR;	// can not move
}


//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look (mobj_t* actor)
{
    if ((actor->flags & MF_WASSEENTHISFRAME)
		|| actor->sector == player->sector)
	{
		actor->target = player;
	    S_StartSound (actor, actor->info->seesound);
	    // go into chase state
	    P_SetMobjState (actor, actor->info->chasestate);
	}
}


//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase (mobj_t*	actor)
{
    if (actor->reactiontime)
	actor->reactiontime--;
				
    if (!actor->target
	|| !(actor->target->flags&MF_SHOOTABLE))
    {
	// look for a new target
	if (P_LookForPlayers(actor))
	    return; 	// got a new target
	
	P_SetMobjState (actor, actor->info->spawnstate);
	return;
    }
    
    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
		actor->flags &= ~MF_JUSTATTACKED;
	    P_NewChaseDir (actor);
		return;
    }
    
    // check for melee attack
    if (actor->info->meleestate
	&& P_CheckMeleeRange (actor))
    {
	if (actor->info->meleesound)
	    S_StartSound (actor, actor->info->meleesound);

	P_SetMobjState (actor, actor->info->meleestate);
	return;
    }
    
    // check for missile attack
    if (actor->info->shootstate)
    {
	if (actor->movecount)
	{
	    goto nomissile;
	}
	
	if (!P_CheckMissileRange (actor))
	    goto nomissile;
	
	P_SetMobjState (actor, actor->info->shootstate);
	actor->flags |= MF_JUSTATTACKED;
	return;
    }

    // ?
  nomissile:
    
    // chase towards player
    if (--actor->movecount < 0
		|| !P_Move (actor))
    {
		P_NewChaseDir (actor);
    }
    
    // make active sound
    if (actor->info->activesound
		&& P_Random () < 3)
    {
		S_StartSound (actor, actor->info->activesound);
    }
}


char R_PointToAngle(int x, int y)
{
    if (x>= 0)
    {
		if (y>= 0)
		{
			if (x>2*y)
			{
				return DI_EAST;
			}
			else if (y > 2*x)
			{
				return DI_NORTH;
			}
			else
			{
				return DI_NORTHEAST;
			}
		}
		else
		{
			// y<0
			y = -y;

			if (x>2*y)
			{
				return DI_EAST;
			}
			else if (y > 2*x)
			{
				return DI_SOUTH;
			}
			else
			{
				return DI_SOUTHEAST;
			}
		}
    }
    else
    {
		// x<0
		x = -x;

		if (y>= 0)
		{
			if (x>2*y)
			{
				return DI_WEST;
			}
			else if (y > 2*x)
			{
				return DI_NORTH;
			}
			else
			{
				return DI_NORTHEAST;
			}
		}
		else
		{
			// y<0
			y = -y;

			if (x>2*y)
			{
				return DI_WEST;
			}
			else if (y > 2*x)
			{
				return DI_SOUTH;
			}
			else
			{
				return DI_SOUTHWEST;
			}
		}
    }
    return 0;
}

//
// A_FaceTarget
//
void A_FaceTarget (mobj_t* actor)
{	
    if (!actor->target)
	return;
    
    actor->flags &= ~MF_AMBUSH;
	
    actor->movedir = R_PointToAngle (actor->x - actor->target->x,
				    actor->y - actor->target->y);
}


//
// A_PosAttack
//
// Possessed soldier
//
void A_PosAttack (mobj_t* actor)
{
    int		damage;
    fixed_t dist;
	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);

    S_StartSound (actor, sfx_pistol);
    dist = P_ApproxDistance(actor->x - actor->target->x, actor->y - actor->target->y);
    if (dist > 220) dist == 220;
    if (P_Random() > dist)
    {
	    damage = ((P_Random()&3)+2)*3; // this was ((r%5)+1)*3
	    P_DamageMobj (actor->target, actor, actor, damage);
	}
}


//
// A_TroopAttack
//
// Imp
//
void A_TroopAttack (mobj_t* actor)
{
    int		damage;
	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
		damage = ((P_Random()&7)+1)*3;
		P_DamageMobj (actor->target, actor, actor, damage);
		return;
    }
    
    // launch a missile
    //P_SpawnMissile (actor, actor->target, MT_TROOPSHOT);
}


void A_Fall (mobj_t *actor)
{
    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;

    // So change this if corpse objects
    // are meant to be obstacles.
}


//
// A_Explode
//
void A_Explode (mobj_t* thingy)
{
    P_RadiusAttack ( thingy, thingy->target, 128 );
}
