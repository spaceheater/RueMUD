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

#ifndef BASEOBJECT_HPP
#define BASEOBJECT_HPP

/*
 * Prototype for an object.
 */
class BaseObject {
public:
  std::string name;
  std::string short_descr;
  std::string description;
  int id;

//  BaseObject();
};

#endif // BASEOBJECT_HPP

