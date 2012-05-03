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

#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "baseobject.hpp"

/*
 * One object.
 */
class Object : public BaseObject {
public:
  std::list<Object *> contains;
  Object *in_obj;
  Character *carried_by;
  std::list<ExtraDescription *> extra_descr;
  std::list<Affect *> affected;
  ObjectPrototype *pIndexData;
  Room *in_room;
  sh_int item_type;
  sh_int extra_flags;
  sh_int wear_flags;
  sh_int wear_loc;
  sh_int weight;
  int cost;
  sh_int level;
  sh_int timer;
  int value[4];

  Object();
  std::string item_type_name ();
  int apply_ac (int iWear);
  int get_obj_number ();
  int get_obj_weight ();
  bool can_wear (sh_int part);
  bool is_obj_stat(sh_int stat);
  void obj_from_room ();
  void obj_to_room (Room * pRoomIndex);
  void obj_to_obj (Object * obj_to);
  void obj_from_obj ();
  void obj_to_char (Character * ch);
  void obj_from_char ();
  void extract_obj ();
  void fwrite_obj (Character * ch, std::ofstream & fp, int iNest);
  bool fread_obj (Character * ch, std::ifstream & fp);
  std::string format_obj_to_char (Character * ch, bool fShort);

};


#endif // OBJECT_HPP

