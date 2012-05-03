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
#include "utils.hpp"
#include "io.hpp"
#include "pcdata.hpp"

PCData::PCData() :
  perm_str(13), perm_int(13), perm_wis(13), perm_dex(13), perm_con(13),
  mod_str(0), mod_int(0), mod_wis(0), mod_dex(0), mod_con(0), pagelen(20) {
  memset(condition, 0, sizeof condition);
  memset(learned, 0, sizeof learned);
}

void PCData::set_prime(sh_int prime)
{
  switch (prime) {
  case APPLY_STR:
    perm_str = 16;
    break;
  case APPLY_INT:
    perm_int = 16;
    break;
  case APPLY_WIS:
    perm_wis = 16;
    break;
  case APPLY_DEX:
    perm_dex = 16;
    break;
  case APPLY_CON:
    perm_con = 16;
    break;
  }
}

std::string PCData::trainable_list(void)
{
  std::string str;

  if (perm_str < 18)
    str.append(" str");
  if (perm_int < 18)
    str.append(" int");
  if (perm_wis < 18)
    str.append(" wis");
  if (perm_dex < 18)
    str.append(" dex");
  if (perm_con < 18)
    str.append(" con");
  return str;
}

void PCData::set_perm(std::string attr, int value)
{
  if (!str_cmp (attr, "str")) {
    perm_str = value;
  } else if (!str_cmp (attr, "int")) {
    perm_int = value;
  } else if (!str_cmp (attr, "wis")) {
    perm_wis = value;
  } else if (!str_cmp (attr, "dex")) {
    perm_dex = value;
  } else if (!str_cmp (attr, "con")) {
    perm_con = value;
  } else {
    bug_printf("Invalid argument to PCData::set_perm: %s", attr.c_str());
  }
  return;
}

sh_int PCData::get_perm(std::string attr)
{
  if (!str_cmp (attr, "str")) {
    return perm_str;
  } else if (!str_cmp (attr, "int")) {
    return perm_int;
  } else if (!str_cmp (attr, "wis")) {
    return perm_wis;
  } else if (!str_cmp (attr, "dex")) {
    return perm_dex;
  } else if (!str_cmp (attr, "con")) {
    return perm_con;
  } else {
    bug_printf("Invalid argument to PCData::get_perm: %s", attr.c_str());
    return 0;
  }
}

void PCData::set_mod(std::string attr, int value)
{
  if (!str_cmp (attr, "str")) {
    mod_str = value;
  } else if (!str_cmp (attr, "int")) {
    mod_int = value;
  } else if (!str_cmp (attr, "wis")) {
    mod_wis = value;
  } else if (!str_cmp (attr, "dex")) {
    mod_dex = value;
  } else if (!str_cmp (attr, "con")) {
    mod_con = value;
  } else {
    bug_printf("Invalid argument to PCData::set_mod: %s", attr.c_str());
  }
  return;
}

sh_int PCData::get_mod(std::string attr)
{
  if (!str_cmp (attr, "str")) {
    return mod_str;
  } else if (!str_cmp (attr, "int")) {
    return mod_int;
  } else if (!str_cmp (attr, "wis")) {
    return mod_wis;
  } else if (!str_cmp (attr, "dex")) {
    return mod_dex;
  } else if (!str_cmp (attr, "con")) {
    return mod_con;
  } else {
    bug_printf("Invalid argument to PCData::get_mod: %s", attr.c_str());
    return 0;
  }
}

sh_int PCData::get_curr(std::string attr)
{
  if (!str_cmp (attr, "str")) {
    return perm_str + mod_str;
  } else if (!str_cmp (attr, "int")) {
    return perm_int + mod_int;
  } else if (!str_cmp (attr, "wis")) {
    return perm_wis + mod_wis;
  } else if (!str_cmp (attr, "dex")) {
    return perm_dex + mod_dex;
  } else if (!str_cmp (attr, "con")) {
    return perm_con + mod_con;
  } else {
    bug_printf("Invalid argument to PCData::get_perm: %s", attr.c_str());
    return 0;
  }
}
