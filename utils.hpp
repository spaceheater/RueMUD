/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
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

#ifndef UTILS_HPP
#define UTILS_HPP

int number_range (int from, int to);
int number_fuzzy (int number);
int dice (int number, int size);
int number_percent (void);
int number_door (void);
int interpolate (int level, int value_00, int value_32);
bool str_cmp(const std::string & s1, const std::string & s2);
bool str_prefix(const std::string & s1, const std::string & s2);
bool str_infix (const std::string & astr, const std::string & bstr);
bool str_suffix(const std::string& s1, const std::string& s2);
std::string capitalize (const std::string & str);
void global_replace (std::string & str, const std::string & s1, const std::string & s2);
bool is_number (const std::string & arg);
int number_argument (const std::string & argument, std::string & arg);
std::string one_argument (const std::string & argument, std::string & arg_first);
bool is_name (const std::string & str, std::string namelist);
void smash_tilde (std::string & str);
std::string fread_string (std::ifstream & fp);
char fread_letter (std::ifstream & fp);
int fread_number (std::ifstream & fp);
void fread_to_eol (std::ifstream & fp);
std::string fread_word (std::ifstream & fp);

#endif // UTILS_HPP
