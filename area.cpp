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
#include "utils.hpp"
#include "io.hpp"

#include "area.hpp"
#include "reset.hpp"
#include "objproto.hpp"
#include "mobproto.hpp"
#include "exit.hpp"
#include "object.hpp"
#include "room.hpp"

int Area::top_area = 0;

Area::Area() :
  age(15), nplayer(0) {
  top_area++;
}

/*
 * Reset one area.
 */
void Area::reset_area (void)
{
  Reset *pReset;
  Character *mob;
  bool last;
  int level;

  mob = NULL;
  last = true;
  level = 0;
  std::list<Reset*>::iterator rst;
  for (rst = reset_list.begin(); rst != reset_list.end(); rst++) {
    pReset = *rst;
    Room *pRoomIndex;
    MobPrototype *pMobIndex;
    ObjectPrototype *pObjIndex;
    ObjectPrototype *pObjToIndex;
    Exit *pexit;
    Object *obj;
    Object *obj_to;

    switch (pReset->command) {
    default:
      bug_printf ("Reset_area: bad command %c.", pReset->command);
      break;

    case 'M':
      if ((pMobIndex = get_mob_index (pReset->arg1)) == NULL) {
        bug_printf ("Reset_area: 'M': bad vnum %d.", pReset->arg1);
        continue;
      }

      if ((pRoomIndex = get_room_index (pReset->arg3)) == NULL) {
        bug_printf ("Reset_area: 'R': bad vnum %d.", pReset->arg3);
        continue;
      }

      level = URANGE (0, pMobIndex->level - 2, LEVEL_HERO);
      if (pMobIndex->count >= pReset->arg2) {
        last = false;
        break;
      }

      mob = pMobIndex->create_mobile ();

      /*
       * Check for pet shop.
       */
      {
        Room *pRoomIndexPrev;
        pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
        if (pRoomIndexPrev != NULL
          && IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
          SET_BIT (mob->actflags, ACT_PET);
      }

      if (pRoomIndex->is_dark())
        SET_BIT (mob->affected_by, AFF_INFRARED);

      mob->char_to_room(pRoomIndex);
      level = URANGE (0, mob->level - 2, LEVEL_HERO);
      last = true;
      break;

    case 'O':
      if ((pObjIndex = get_obj_index (pReset->arg1)) == NULL) {
        bug_printf ("Reset_area: 'O': bad vnum %d.", pReset->arg1);
        continue;
      }

      if ((pRoomIndex = get_room_index (pReset->arg3)) == NULL) {
        bug_printf ("Reset_area: 'R': bad vnum %d.", pReset->arg3);
        continue;
      }

      if (nplayer > 0
        || pObjIndex->count_obj_list (pRoomIndex->contents) > 0) {
        last = false;
        break;
      }

      obj = pObjIndex->create_object(number_fuzzy (level));
      obj->cost = 0;
      obj->obj_to_room (pRoomIndex);
      last = true;
      break;

    case 'P':
      if ((pObjIndex = get_obj_index (pReset->arg1)) == NULL) {
        bug_printf ("Reset_area: 'P': bad vnum %d.", pReset->arg1);
        continue;
      }

      if ((pObjToIndex = get_obj_index (pReset->arg3)) == NULL) {
        bug_printf ("Reset_area: 'P': bad vnum %d.", pReset->arg3);
        continue;
      }

      if (nplayer > 0
        || (obj_to = pObjToIndex->get_obj_type()) == NULL
        || pObjIndex->count_obj_list (obj_to->contains) > 0) {
        last = false;
        break;
      }

      obj = pObjIndex->create_object (number_fuzzy (obj_to->level));
      obj->obj_to_obj (obj_to);
      last = true;
      break;

    case 'G':
    case 'E':
      if ((pObjIndex = get_obj_index (pReset->arg1)) == NULL) {
        bug_printf ("Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1);
        continue;
      }

      if (!last)
        break;

      if (mob == NULL) {
        bug_printf ("Reset_area: 'E' or 'G': null mob for vnum %d.", pReset->arg1);
        last = false;
        break;
      }

      if (mob->pIndexData->pShop != NULL) {
        int olevel;

        switch (pObjIndex->item_type) {
        default:
          olevel = 0;
          break;
        case ITEM_PILL:
          olevel = number_range (0, 10);
          break;
        case ITEM_POTION:
          olevel = number_range (0, 10);
          break;
        case ITEM_SCROLL:
          olevel = number_range (5, 15);
          break;
        case ITEM_WAND:
          olevel = number_range (10, 20);
          break;
        case ITEM_STAFF:
          olevel = number_range (15, 25);
          break;
        case ITEM_ARMOR:
          olevel = number_range (5, 15);
          break;
        case ITEM_WEAPON:
          olevel = number_range (5, 15);
          break;
        }

        obj = pObjIndex->create_object (olevel);
        SET_BIT (obj->extra_flags, ITEM_INVENTORY);
      } else {
        obj = pObjIndex->create_object (number_fuzzy (level));
      }
      obj->obj_to_char (mob);
      if (pReset->command == 'E')
        mob->equip_char (obj, pReset->arg3);
      last = true;
      break;

    case 'D':
      if ((pRoomIndex = get_room_index (pReset->arg1)) == NULL) {
        bug_printf ("Reset_area: 'D': bad vnum %d.", pReset->arg1);
        continue;
      }

      if ((pexit = pRoomIndex->exit[pReset->arg2]) == NULL)
        break;

      switch (pReset->arg3) {
      case 0:
        REMOVE_BIT (pexit->exit_info, EX_CLOSED);
        REMOVE_BIT (pexit->exit_info, EX_LOCKED);
        break;

      case 1:
        SET_BIT (pexit->exit_info, EX_CLOSED);
        REMOVE_BIT (pexit->exit_info, EX_LOCKED);
        break;

      case 2:
        SET_BIT (pexit->exit_info, EX_CLOSED);
        SET_BIT (pexit->exit_info, EX_LOCKED);
        break;
      }

      last = true;
      break;

    case 'R':
      if ((pRoomIndex = get_room_index (pReset->arg1)) == NULL) {
        bug_printf ("Reset_area: 'R': bad vnum %d.", pReset->arg1);
        continue;
      }

      {
        int d0;
        int d1;

        for (d0 = 0; d0 < pReset->arg2 - 1; d0++) {
          d1 = number_range (d0, pReset->arg2 - 1);
          pexit = pRoomIndex->exit[d0];
          pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
          pRoomIndex->exit[d1] = pexit;
        }
      }
      break;
    }
  }

  return;
}



