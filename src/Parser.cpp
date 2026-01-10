#include "Parser.h"
#include <unordered_map>
#include <format>
#include <iostream>

Parser::Parser(BinaryModule& bin)
:bin_(bin)
{
}

bool Parser::parseFunctions()
{
  // collect entrances
  std::set<RVA_t> entrances = collectModuleEntrance();

  for (RVA_t entry: entrances)
  {
    auto func = std::make_shared<Function>(bin_, entry);
    if (func->parse()) {
      functions_.push_back(func);
      //bin_.addFunction(func);  // ×¢²áµ½Ä£¿é
    }
  }

  //buildBlocks(entrances);
  return true;
}



std::set<RVA_t> Parser::collectModuleEntrance()
{
  std::set<RVA_t> entrance;

  if (bin_.entryPoint() != 0)
  {
    entrance.insert(bin_.entryPoint());
  }

  for (auto& item : bin_.exportFunctions())
  {
    entrance.insert(item.rva);
  }

  return entrance;
}
