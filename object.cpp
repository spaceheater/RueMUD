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
#include "object.hpp"
#include "room.hpp"
#include "affect.hpp"
#include "extra.hpp"
#include "objproto.hpp"
#include "io.hpp"
#include "utils.hpp"

Object::Object() :
  in_obj(NULL), carried_by(NULL), pIndexData(NULL),
  in_room(NULL), item_type(0), extra_flags(0), wear_flags(0), wear_loc(0),
  weight(0), cost(0), level(0), timer(0) {
  memset(value, 0, sizeof value);
}

/*
 * Return ascii name of an item type.
 */
std::string Object::item_type_name ()
{
  switch (item_type) {
  case ITEM_LIGHT:
    return "light";
  case ITEM_SCROLL:
    return "scroll";
  case ITEM_WAND:
    return "wand";
  case ITEM_STAFF:
    return "staff";
  case ITEM_WEAPON:
    return "weapon";
  case ITEM_TREASURE:
    return "treasure";
  case ITEM_ARMOR:
    return "armor";
  case ITEM_POTION:
    return "potion";
  case ITEM_FURNITURE:
    return "furniture";
  case ITEM_TRASH:
    return "trash";
  case ITEM_CONTAINER:
    return "container";
  case ITEM_DRINK_CON:
    return "drink container";
  case ITEM_KEY:
    return "key";
  case ITEM_FOOD:
    return "food";
  case ITEM_MONEY:
    return "money";
  case ITEM_BOAT:
    return "boat";
  case ITEM_CORPSE_NPC:
    return "npc corpse";
  case ITEM_CORPSE_PC:
    return "pc corpse";
  case ITEM_FOUNTAIN:
    return "fountain";
  case ITEM_PILL:
    return "pill";
  }

  bug_printf ("Item_type_name: unknown type %d.", item_type);
  return "(unknown)";
}

bool Object::can_wear (sh_int part) {
  return wear_flags & part;
}

bool Object::is_obj_stat(sh_int stat) {
  return extra_flags & stat;
}

/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int Object::get_obj_number ()
{
  int number = 0;

  if (item_type == ITEM_CONTAINER) {
    for (ObjIter o = contains.begin(); o != contains.end(); o++)
      number += (*o)->get_obj_number();
  } else
    number = 1;

  return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int Object::get_obj_weight ()
{
  int wt = weight;

  for (ObjIter o = contains.begin(); o != contains.end(); o++)
    wt += (*o)->get_obj_weight ();

  return wt;
}

/*
 * Move an obj out of a room.
 */
void Object::obj_from_room ()
{
  Room *in_rm = in_room;

  if (in_rm == NULL) {
    bug_printf ("obj_from_room: NULL.");
    return;
  }

  in_rm->contents.erase(find(in_rm->contents.begin(),in_rm->contents.end(),this));
  in_room = NULL;
  return;
}

/*
 * Move an obj into a room.
 */
void Object::obj_to_room (Room * pRoomIndex)
{
  pRoomIndex->contents.push_back(this);
  in_room = pRoomIndex;
  carried_by = NULL;
  in_obj = NULL;
  return;
}

/*
 * Move an object into an object.
 */
void Object::obj_to_obj (Object * obj_to)
{
  obj_to->contains.push_back(this);
  in_obj = obj_to;
  in_room = NULL;
  carried_by = NULL;

  for (; obj_to != NULL; obj_to = obj_to->in_obj) {
    if (obj_to->carried_by != NULL) {
      obj_to->carried_by->carry_number += get_obj_number();
      obj_to->carried_by->carry_weight += get_obj_weight();
    }
  }

  return;
}

/*
 * Move an object out of an object.
 */
void Object::obj_from_obj ()
{
  Object *obj_from = in_obj;

  if (obj_from == NULL) {
    bug_printf ("Obj_from_obj: null obj_from.");
    return;
  }

  obj_from->contains.erase(find(obj_from->contains.begin(), obj_from->contains.end(), this));
  in_obj = NULL;

  for (; obj_from != NULL; obj_from = obj_from->in_obj) {
    if (obj_from->carried_by != NULL) {
      obj_from->carried_by->carry_number -= get_obj_number();
      obj_from->carried_by->carry_weight -= get_obj_weight();
    }
  }

  return;
}

/*
 * Give an obj to a char.
 */
void Object::obj_to_char (Character * ch)
{
  ch->carrying.push_back(this);
  carried_by = ch;
  in_room = NULL;
  in_obj = NULL;
  ch->carry_number += get_obj_number();
  ch->carry_weight += get_obj_weight();
}

/*
 * Find the ac value of an obj, including position effect.
 */
int Object::apply_ac (int iWear)
{
  if (item_type != ITEM_ARMOR)
    return 0;

  switch (iWear) {
  case WEAR_BODY:
    return 3 * value[0];
  case WEAR_HEAD:
    return 2 * value[0];
  case WEAR_LEGS:
    return 2 * value[0];
  case WEAR_FEET:
    return value[0];
  case WEAR_HANDS:
    return value[0];
  case WEAR_ARMS:
    return value[0];
  case WEAR_SHIELD:
    return value[0];
  case WEAR_FINGER_L:
    return value[0];
  case WEAR_FINGER_R:
    return value[0];
  case WEAR_NECK_1:
    return value[0];
  case WEAR_NECK_2:
    return value[0];
  case WEAR_ABOUT:
    return 2 * value[0];
  case WEAR_WAIST:
    return value[0];
  case WEAR_WRIST_L:
    return value[0];
  case WEAR_WRIST_R:
    return value[0];
  case WEAR_HOLD:
    return value[0];
  }

  return 0;
}

/*
 * Take an obj from its character.
 */
void Object::obj_from_char ()
{
  Character *ch = carried_by;

  if (ch == NULL) {
    bug_printf ("Obj_from_char: null ch.");
    return;
  }

  if (wear_loc != WEAR_NONE)
    ch->unequip_char(this);

  ch->carrying.erase(find(ch->carrying.begin(),ch->carrying.end(), this));

  carried_by = NULL;
  ch->carry_number -= get_obj_number();
  ch->carry_weight -= get_obj_weight();
  return;
}

/*
 * Extract an obj from the world.
 */
void Object::extract_obj ()
{
  Object *obj_content;

  if (in_room != NULL)
    obj_from_room ();
  else if (carried_by != NULL)
    obj_from_char ();
  else if (in_obj != NULL)
    obj_from_obj ();

  ObjIter o, next;
  for (o = contains.begin(); o != contains.end(); o = next) {
    obj_content = *o;
    next = ++o;
    obj_content->extract_obj();
  }

  deepobnext = object_list.erase(find(object_list.begin(), object_list.end(), this));

  AffIter af;
  for (af = affected.begin(); af != affected.end(); af++) {
    delete *af;
  }
  affected.clear();

  std::list<ExtraDescription *>::iterator ed;
  for (ed = extra_descr.begin(); ed != extra_descr.end(); ed++) {
    delete *ed;
  }
  extra_descr.clear();

  --pIndexData->count;
  delete this;
  return;
}

/*
 * Write an object and its contents.
 */
void Object::fwrite_obj (Character * ch, std::ofstream & fp, int iNest)
{

  /*
   * Castrate storage characters.
   */
  if (ch->level < level || item_type == ITEM_KEY || item_type == ITEM_POTION)
    return;

  fp << "#OBJECT\n";
  fp << "Nest         " << iNest << "\n";
  fp << "Name         " << name << "~\n";
  fp << "ShortDescr   " << short_descr << "~\n";
  fp << "Description  " << description << "~\n";
  fp << "Vnum         " << pIndexData->vnum << "\n";
  fp << "ExtraFlags   " << extra_flags << "\n";
  fp << "WearFlags    " << wear_flags << "\n";
  fp << "WearLoc      " << wear_loc << "\n";
  fp << "ItemType     " << item_type << "\n";
  fp << "Weight       " << weight << "\n";
  fp << "Level        " << level << "\n";
  fp << "Timer        " << timer << "\n";
  fp << "Cost         " << cost << "\n";
  fp << "Values       " << value[0] << " " << value[1] << " " <<
    value[2] << " " << value[3] << "\n";

  switch (item_type) {
  case ITEM_POTION:
  case ITEM_SCROLL:
    if (value[1] > 0) {
      fp << "Spell 1      '" << skill_table[value[1]].name << "'\n";
    }

    if (value[2] > 0) {
      fp << "Spell 2      '" << skill_table[value[2]].name << "'\n";
    }

    if (value[3] > 0) {
      fp << "Spell 3      '" << skill_table[value[3]].name << "'\n";
    }

    break;

  case ITEM_PILL:
  case ITEM_STAFF:
  case ITEM_WAND:
    if (value[3] > 0) {
      fp << "Spell 3      '" << skill_table[value[3]].name << "'\n";
    }

    break;
  }

  AffIter af;
  for (af = affected.begin(); af != affected.end(); af++) {
    fp << "Affect       " << (*af)->type << " " << (*af)->duration << " " <<
      (*af)->modifier << " " << (*af)->location << " " << (*af)->bitvector << "\n";
  }

  std::list<ExtraDescription *>::iterator ed;
  for (ed = extra_descr.begin(); ed != extra_descr.end(); ed++) {
    fp << "ExtraDescr   " << (*ed)->keyword << "~ " <<
      (*ed)->description << "~\n";
  }

  fp << "End\n\n";

  std::list<Object*>::reverse_iterator o;
  for (o = contains.rbegin(); o != contains.rend(); o++)
    (*o)->fwrite_obj (ch, fp, iNest + 1);

  return;
}

bool Object::fread_obj (Character * ch, std::ifstream & fp)
{
  std::string word;
  int iNest = 0;
  bool fMatch;
  bool fNest = false;
  bool fVnum = true;

  for (;;) {
    word = fp.eof() ? std::string("End") : fread_word (fp);
    fMatch = false;

    switch (toupper (word[0])) {
    case '*':
      fMatch = true;
      fread_to_eol (fp);
      break;

    case 'A':
      if (!str_cmp (word, "Affect")) {
        Affect *paf;

        paf = new Affect();

        paf->type = fread_number (fp);
        paf->duration = fread_number (fp);
        paf->modifier = fread_number (fp);
        paf->location = fread_number (fp);
        paf->bitvector = fread_number (fp);
        affected.push_back(paf);
        fMatch = true;
        break;
      }
      break;

    case 'C':
      KEY ("Cost", cost, fread_number (fp));
      break;

    case 'D':
      KEY ("Description", description, fread_string (fp));
      break;

    case 'E':
      KEY ("ExtraFlags", extra_flags, fread_number (fp));

      if (!str_cmp (word, "ExtraDescr")) {
        ExtraDescription *ed;

        ed = new ExtraDescription();

        ed->keyword = fread_string (fp);
        ed->description = fread_string (fp);
        extra_descr.push_back(ed);
        fMatch = true;
      }

      if (!str_cmp (word, "End")) {
        if (!fNest || !fVnum) {
          bug_printf ("Fread_obj: incomplete object.");
          return false;
        } else {
          object_list.push_back(this);
          pIndexData->count++;
          if (iNest == 0 || rgObjNest[iNest] == NULL)
            obj_to_char (ch);
          else
            obj_to_obj (rgObjNest[iNest - 1]);
          return true;
        }
      }
      break;

    case 'I':
      KEY ("ItemType", item_type, fread_number (fp));
      break;

    case 'L':
      KEY ("Level", level, fread_number (fp));
      break;

    case 'N':
      KEY ("Name", name, fread_string (fp));

      if (!str_cmp (word, "Nest")) {
        iNest = fread_number (fp);
        if (iNest < 0 || iNest >= MAX_NEST) {
          bug_printf ("Fread_obj: bad nest %d.", iNest);
        } else {
          rgObjNest[iNest] = this;
          fNest = true;
        }
        fMatch = true;
      }
      break;

    case 'S':
      KEY ("ShortDescr", short_descr, fread_string (fp));

      if (!str_cmp (word, "Spell")) {
        int iValue;
        int sn;

        iValue = fread_number (fp);
        sn = skill_lookup (fread_word (fp));
        if (iValue < 0 || iValue > 3) {
          bug_printf ("Fread_obj: bad iValue %d.", iValue);
        } else if (sn < 0) {
          bug_printf ("Fread_obj: unknown skill.");
        } else {
          value[iValue] = sn;
        }
        fMatch = true;
        break;
      }

      break;

    case 'T':
      KEY ("Timer", timer, fread_number (fp));
      break;

    case 'V':
      if (!str_cmp (word, "Values")) {
        value[0] = fread_number (fp);
        value[1] = fread_number (fp);
        value[2] = fread_number (fp);
        value[3] = fread_number (fp);
        fMatch = true;
        break;
      }

      if (!str_cmp (word, "Vnum")) {
        int vnum;

        vnum = fread_number (fp);
        if ((pIndexData = get_obj_index (vnum)) == NULL)
          bug_printf ("Fread_obj: bad vnum %d.", vnum);
        else
          fVnum = true;
        fMatch = true;
        break;
      }
      break;

    case 'W':
      KEY ("WearFlags", wear_flags, fread_number (fp));
      KEY ("WearLoc", wear_loc, fread_number (fp));
      KEY ("Weight", weight, fread_number (fp));
      break;

    }

    if (!fMatch) {
      bug_printf ("Fread_obj: no match.");
      fread_to_eol (fp);
    }
  }
  return false;
}

std::string Object::format_obj_to_char (Character * ch, bool fShort)
{
  std::string buf;

  if (is_obj_stat(ITEM_INVIS))
    buf.append("(Invis) ");
  if (ch->is_affected (AFF_DETECT_EVIL) && is_obj_stat(ITEM_EVIL))
    buf.append("(Red Aura) ");
  if (ch->is_affected (AFF_DETECT_MAGIC) && is_obj_stat(ITEM_MAGIC))
    buf.append("(Magical) ");
  if (is_obj_stat(ITEM_GLOW))
    buf.append("(Glowing) ");
  if (is_obj_stat(ITEM_HUM))
    buf.append("(Humming) ");

  if (fShort) {
    if (!short_descr.empty())
      buf.append(short_descr);
  } else {
    if (!description.empty())
      buf.append(description);
  }

  return buf;
}


