/*
 MurkMUD++ - A Windows compatible, C++ compatible Merc 2.2 Mud.

 \author Jon A. Lambert
 \date 01/02/2007
 \version 1.5
 \remarks
  This source code copyright (C) 2005, 2006, 2007 by Jon A. Lambert
  All rights reserved.

  Use governed by the MurkMUD++ public license found in license.murk++
*/

#ifndef SYMBOLS_HPP
#define SYMBOLS_HPP
#include <string>
#include <map>

class SymbolTable {
private:
  std::map<std::string, int> kv;
  std::map<int, std::string> vk;
public:
  bool add(std::string symbol, int value);
  int lookup(std::string symbol);
  std::string lookup(int value);
};

#endif // SYMBOLS_HPP
