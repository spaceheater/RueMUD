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

#include "os.hpp"

#include "symbols.hpp"

bool SymbolTable::add(std::string symbol, int value)
{
  if (kv.find(symbol) != kv.end() || vk.find(value) != vk.end())
    return false;
  kv.insert (std::map<std::string, int>::value_type (symbol, value));
  vk.insert (std::map<int, std::string>::value_type (value, symbol));
  return true;
}

std::string SymbolTable::lookup(int value)
{
  std::map<int, std::string>::iterator p_val;

  p_val = vk.find(value);
  if (p_val != vk.end())
    return (*p_val).second;
  return "";
}

int SymbolTable::lookup(std::string symbol)
{
  std::map<std::string, int>::iterator p_val;

  p_val = kv.find(symbol);
  if (p_val != kv.end())
    return (*p_val).second;
  return -1;
}

