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

#ifndef PCDATA_HPP
#define PCDATA_HPP

/*
 * Data which only PC's have.
 */
class PCData {
public:
  std::string pwd;
  std::string bamfin;
  std::string bamfout;
  std::string title;
private:
  sh_int perm_str;
  sh_int perm_int;
  sh_int perm_wis;
  sh_int perm_dex;
  sh_int perm_con;
  sh_int mod_str;
  sh_int mod_int;
  sh_int mod_wis;
  sh_int mod_dex;
  sh_int mod_con;
public:
  sh_int condition[3];
  sh_int pagelen;
  sh_int learned[MAX_SKILL];
  std::list<std::string> gag_list;

  PCData();
  void set_prime(sh_int prime);
  std::string trainable_list(void);
  void set_perm(std::string attr, int value);
  sh_int get_perm(std::string attr);
  void set_mod(std::string attr, int value);
  sh_int get_mod(std::string attr);
  sh_int get_curr(std::string attr);

};

#endif // PCDATA_HPP
