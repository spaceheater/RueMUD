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

#ifndef OBJPROTO_HPP
#define OBJPROTO_HPP

#include "baseobject.hpp"

/*
 * Prototype for an object.
 */
class ObjectPrototype : public BaseObject {
public:
  static int top_obj;
  std::list<ExtraDescription *> extra_descr;
  std::list<Affect *> affected;
  sh_int vnum;
  sh_int item_type;
  sh_int extra_flags;
  sh_int wear_flags;
  sh_int count;
  sh_int weight;
  int cost;                     /* Unused */
  int value[4];

  ObjectPrototype();
  int count_obj_list (std::list<Object *> & list);
  Object * get_obj_type ();
  Object * create_object (int lvl);

};

#endif // OBJPROTO_HPP

