#include "Parser.h"

Parser::Parser(BinaryModule& bin)
:bin_(bin)
{
}

bool Parser::parseFunctions()
{
  // collect entrances
  collectEntrance();
  if(entrances_.empty())
    return true;

  while (true)
  {
    RVA_t rva = entrances_.top();
    entrances_.pop();
    parseFunction(rva);
  }

  return true;
}

bool Parser::parseFunction(RVA_t rva)
{
  

  return true;
}

void Parser::collectEntrance()
{
  std::vector<uint32_t> entrances;

  if (bin_.entryPoint() != 0)
  {
    entrances_.push(bin_.entryPoint());
  }

  for (auto& item : bin_.exportFunctions())
  {
    entrances_.push(item.rva);
  }
}
