#include "Parser.h"

Parser::Parser(BinaryModule& bin)
:bin_(bin)
{
}

bool Parser::parseFunctions()
{
  // collect entrances
  collectEntrance();
  if(workList_.empty())
    return true;

  while (!workList_.empty())
  {
    RVA_t rva = workList_.top();
    workList_.pop();
    parseFunction(rva);
  }

  return true;
}

bool Parser::parseFunction(RVA_t rva)
{
  RVA_t currentRva = rva;
  while (auto inst = bin_.disassembleOne(currentRva))
  {
    
    currentRva += inst->size();
  }

  return true;
}

void Parser::collectEntrance()
{
  std::vector<uint32_t> entrances;

  if (bin_.entryPoint() != 0)
  {
    workList_.push(bin_.entryPoint());
  }

  for (auto& item : bin_.exportFunctions())
  {
    workList_.push(item.rva);
  }
}
