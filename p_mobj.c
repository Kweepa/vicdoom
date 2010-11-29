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
//	Moving object handling. Spawn functions.
//
//-----------------------------------------------------------------------------

//
// P_SetMobjState
// Returns true if the mobj is still present.
//

boolean
P_SetMobjState
( mobj_t*	mobj,
  statenum_t	state )
{
    state_t*	st;

    do
    {
	if (state == S_NULL)
	{
	    mobj->state = (state_t *) S_NULL;
	    P_RemoveMobj (mobj);
	    return false;
	}

	st = &states[state];
	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;

	// Modified handling.
	// Call action functions when the state is set
	if (st->action)
	    st->action(mobj);	
	
	state = st->nextstate;
    } while (!mobj->tics);
				
    return true;
}


//
// P_ExplodeMissile  
//
void P_ExplodeMissile (mobj_t* mo)
{
    mo->momx = mo->momy = 0;

    P_SetMobjState (mo, mo->info->deathstate);

    mo->tics -= P_Random()&3;

    if (mo->tics < 1)
	mo->tics = 1;

    mo->flags &= ~MF_MISSILE;

    if (mo->info->deathsound)
	S_StartSound (mo, mo->info->deathsound);
}


//
// P_XYMovement  
//

boolean P_TryMove (mobj_t* mo, fixed_t tryx, fixed_t tryy)
{
  char curSector = mo->sector;
  sector *sec = &sectors[curSector];
  char i, ni;
  edge *curEdge;
  vertex *v1;
  vertex *v2;
  long ex;
  long ey;
  long px, py;
  long height;
  long edgeLen;
  long dist;
  
  // see which edge the new coordinate is behind
  for (i = 0; i < sec->numverts; ++i)
  {
     ni = (i + 1);
     if (ni == sec->numverts) ni = 0;
     curEdge = &edges[sec->edges[i]];
     v1 = &verts[sec->verts[i]];
     v2 = &verts[sec->verts[ni]];
     ex = ((long)v2->x) - v1->x;
     ey = ((long)v2->y) - v1->y;
     px = tryx - 256*v1->x;
     py = tryy - 256*v1->y;
     // need to precalc 65536/edge.len
     edgeLen = curEdge->len;
     height = (px * ey - py * ex) / edgeLen;
     if (height < INNERCOLLISIONRADIUS)
     {
        // check we're within the extents of the edge
        dist = (px * ex + py * ey)/edgeLen;
		if (dist > 0 && dist < 256*edgeLen)
		{
          if (height < 0)
          {
			if (curEdge->sector != -1)
			{
                 mo->sector = curEdge->sector;
				 mo->x = tryx;
				 mo->y = tryy;
				 return true;
              }
              else
              {
				return false;
			  }
           }
       }
		else if (curEdge->sector == -1
			&& dist > -INNERCOLLISIONRADIUS
			&& dist < 256*edgeLen + INNERCOLLISIONRADIUS)
       {
		return false;
		}
		}
	}
	mo->x = tryx;
	mo->y = tryy;
	return true;
}

//
// P_XYMovement  
//

void P_XYMovement (mobj_t* mo) 
{ 	
    if (!mo->momx && !mo->momy)
    {
	return;
    }
	
	if (!P_TryMove (mo, mo->x + mo->momx, mo->y + mo->momy))
	{
		if (mo->flags & MF_MISSILE)
	    {
		P_ExplodeMissile (mo);
	    }
	    else
		mo->momx = mo->momy = 0;
	}
    
    if (mo->flags & (MF_MISSILE | MF_SKULLFLY) )
	return; 	// no friction for missiles ever
}


//
// P_MobjThinker
//
void P_MobjThinker (mobj_t* mobj)
{
    // momentum movement
    if (mobj->momx
	|| mobj->momy
	|| (mobj->flags&MF_SKULLFLY) )
    {
	P_XYMovement (mobj);

	// FIXME: decent NOP/NULL/Nil function pointer please.
	if (mobj->thinker.function.acv == (actionf_v) (-1))
	    return;		// mobj was removed
    }
    
    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
	mobj->tics--;
		
	// you can cycle through multiple states in a tic
	if (!mobj->tics)
	    if (!P_SetMobjState (mobj, mobj->state->nextstate) )
		return;		// freed itself
    }
}


//
// P_SpawnMobj
//
mobj_t*
P_SpawnMobj
( mobj_t *owner,
  mobjtype_t	type )
{
    mobj_t*	mobj;
    state_t*	st;
    mobjinfo_t*	info;
	
    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));
    info = &mobjinfo[type];
	
    mobj->type = type;
    mobj->info = info;
    mobj->x = owner->x;
    mobj->y = owner->y;
    mobj->flags = info->flags;
    mobj->health = info->spawnhealth;
	mobj->reactiontime = info->reactiontime;
    
    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // set subsector and/or block links
    mobj->sector = owner->sector;

    P_AddThinker (mobj);

    return mobj;
}


//
// P_RemoveMobj
//

void P_RemoveMobj (mobj_t* mobj)
{
    // free block
    P_RemoveThinker ((thinker_t*)mobj);
}






//
// GAME SPAWN FUNCTIONS
//



//
// P_SpawnMissile
//
mobj_t*
P_SpawnMissile
( mobj_t*	source,
  mobj_t*	dest,
  mobjtype_t	type )
{
    mobj_t*	th;
    fixed_t dx;
    fixed_t dy;
    fixed_t dist;

    th = P_SpawnMobj (source, type);
    
    if (th->info->seesound)
	S_StartSound (th, th->info->seesound);

    th->target = source;	// where it came from
    
    dx = dest->x - source->x;
    dy = dest->y - source->y;
    
    dist = sqrt(dx*dx + dy*dy);
    
    th->momx = th->info->speed*dx/dist;
    th->momy = th->info->speed*dy/dist;
	
    return th;
}
