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
#include "io.hpp"
#include "utils.hpp"

#include "mobproto.hpp"

int MobPrototype::top_mob = 0;

MobPrototype::MobPrototype() :
  spec_fun(NULL), pShop(NULL), vnum(0), count(0), killed(0),
  sex(0), level(0), actflags(0), affected_by(0), alignment(0),
  mobprogs(NULL), progtypes(0) {
  top_mob++;
}

/*
 * Create an instance of a mobile.
 */
Character * MobPrototype::create_mobile ()
{
  Character *mob;

  if (this == NULL) {
    fatal_printf ("Create_mobile: NULL this.");
  }

  mob = new Character();

  mob->pIndexData = this;

  mob->name = name;
  mob->short_descr = short_descr;
  mob->long_descr = long_descr;
  mob->description = description;
  mob->spec_fun = spec_fun;
  mob->prompt = "<%h %m %v>";

  mob->level = number_fuzzy (level);
  mob->actflags = actflags;
  mob->affected_by = affected_by;
  mob->alignment = alignment;
  mob->sex = sex;

  mob->armor = interpolate (mob->level, 100, -100);

  mob->max_hit = mob->level * 8 + number_range (mob->level * mob->level / 4,
    mob->level * mob->level);
  mob->hit = mob->max_hit;

  /*
   * Insert in list.
   */
  char_list.push_back(mob);
  count++;
  return mob;
}

