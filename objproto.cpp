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

#include "objproto.hpp"
#include "object.hpp"

int ObjectPrototype::top_obj = 0;

ObjectPrototype::ObjectPrototype() :
  vnum(0), item_type(0),
  extra_flags(0), wear_flags(0), count(0), weight(0), cost(0) {
  memset(value, 0, sizeof value);
  top_obj++;
}

/*
 * Count occurrences of an obj in a list.
 */
int ObjectPrototype::count_obj_list (std::list<Object *> & list)
{
  int nMatch = 0;

  for (ObjIter o = list.begin(); o != list.end(); o++) {
    if ((*o)->pIndexData == this)
      nMatch++;
  }
  return nMatch;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
Object * ObjectPrototype::get_obj_type ()
{
  for (ObjIter obj = object_list.begin(); obj != object_list.end(); obj++) {
    if ((*obj)->pIndexData == this)
      return *obj;
  }
  return NULL;
}

/*
 * Create an instance of an object.
 */
Object * ObjectPrototype::create_object (int lvl)
{
  Object *obj;

  if (this == NULL) {
    fatal_printf ("Create_object: NULL this.");
  }

  obj = new Object();

  obj->pIndexData = this;
  obj->in_room = NULL;
  obj->level = lvl;
  obj->wear_loc = -1;

  obj->name = name;
  obj->short_descr = short_descr;
  obj->description = description;
  obj->item_type = item_type;
  obj->extra_flags = extra_flags;
  obj->wear_flags = wear_flags;
  obj->value[0] = value[0];
  obj->value[1] = value[1];
  obj->value[2] = value[2];
  obj->value[3] = value[3];
  obj->weight = weight;
  obj->cost = number_fuzzy (10)
    * number_fuzzy (lvl) * number_fuzzy (lvl);

  /*
   * Mess with object properties.
   */
  switch (obj->item_type) {
  default:
    bug_printf ("Read_object: vnum %d bad type.", vnum);
    break;

  case ITEM_LIGHT:
  case ITEM_TREASURE:
  case ITEM_FURNITURE:
  case ITEM_TRASH:
  case ITEM_CONTAINER:
  case ITEM_DRINK_CON:
  case ITEM_KEY:
  case ITEM_FOOD:
  case ITEM_BOAT:
  case ITEM_CORPSE_NPC:
  case ITEM_CORPSE_PC:
  case ITEM_FOUNTAIN:
    break;

  case ITEM_SCROLL:
    obj->value[0] = number_fuzzy (obj->value[0]);
    break;

  case ITEM_WAND:
  case ITEM_STAFF:
    obj->value[0] = number_fuzzy (obj->value[0]);
    obj->value[1] = number_fuzzy (obj->value[1]);
    obj->value[2] = obj->value[1];
    break;

  case ITEM_WEAPON:
    obj->value[1] = number_fuzzy (number_fuzzy (1 * lvl / 4 + 2));
    obj->value[2] = number_fuzzy (number_fuzzy (3 * lvl / 4 + 6));
    break;

  case ITEM_ARMOR:
    obj->value[0] = number_fuzzy (lvl / 4 + 2);
    break;

  case ITEM_POTION:
  case ITEM_PILL:
    obj->value[0] = number_fuzzy (number_fuzzy (obj->value[0]));
    break;

  case ITEM_MONEY:
    obj->value[0] = obj->cost;
    break;
  }

  object_list.push_back(obj);
  count++;

  return obj;
}

