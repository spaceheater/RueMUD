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

#ifndef DESCRIPTOR_HPP
#define DESCRIPTOR_HPP


/*
 * Descriptor (channel) structure.
 */
class Descriptor {
public:
  Character *character;
  Character *original;
  std::string host;
  SOCKET descriptor;
  int connected;
  bool fcommand;
  char inbuf[4 * MAX_INPUT_LENGTH];
  std::string incomm;
  std::string inlast;
  int repeat;
  char *showstr_head;
  char *showstr_point;
  std::string outbuf;

  Descriptor(SOCKET desc);
  void show_string (const std::string & input);
  void nanny (std::string argument);
  void read_from_buffer ();
  bool check_reconnect (const std::string & name, bool fConn);
  bool check_playing (const std::string & name);
  void close_socket ();
  bool read_from_descriptor ();
  bool load_char_obj (const std::string & name);
  void write_to_buffer (const std::string & txt);
  bool process_output (bool fPrompt);
  void assign_hostname ();
};

#endif // DESCRIPTOR_HPP




