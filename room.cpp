/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.diku' as well the Merc      *
 *  license in 'license.merc'.  In particular, you may not remove either   *
 *  of these copyright notices.                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 MurkMUD++ - A Windows compatible, C++ compatible Merc 2.2 Mud.

 \author Jon A. Lambert
 \date 08/30/2006
 \version 1.4
 \remarks
  This source code copyright (C) 2005, 2006 by Jon A. Lambert
  All rights reserved.

  Use governed by the MurkMUD++ public license found in license.murk++
*/

#include "os.hpp"
#include "config.hpp"
#include "globals.hpp"
#include "room.hpp"
#include "world.hpp"

int Room::top_room = 0;

Room::Room() :
  area(NULL),
  vnum(0), room_flags(0), light(0), sector_type(0) {
  memset(exit, 0, sizeof exit);
  top_room++;
}

/*
 * True if room is private.
 */
bool Room::is_private() {
  int count = people.size();

  if (IS_SET (room_flags, ROOM_PRIVATE) && count >= 2)
    return true;

  if (IS_SET (room_flags, ROOM_SOLITARY) && count >= 1)
    return true;

  return false;
}

/*
 * True if room is dark.
 */
bool Room::is_dark ()
{
  if (light > 0)
    return false;

  if (IS_SET (room_flags, ROOM_DARK))
    return true;

  if (sector_type == SECT_INSIDE || sector_type == SECT_CITY)
    return false;

  if (g_world->is_dark())
    return true;

  return false;
}

