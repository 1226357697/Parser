#include "Parser.h"

Parser::Parser(BinaryModule& bin)
:bin_(bin)
{
}

bool Parser::parseFunctions()
{
  // collect entrances
  std::set<uint32_t> leaders = searchLeaders();
  std::stack<uint32_t> workLists(std::deque<uint32_t>(leaders.begin(), leaders.end()));
  auto instAnalyzer = bin_.instructionAnalyzer();

  while (!workLists.empty())
  {
    RVA_t rva = workLists.top();
    workLists.pop();

    RVA_t currentRva = rva;
    while (auto inst = bin_.disassembleOne(currentRva))
    {
      if (instAnalyzer->isConditionalJump(*inst))
      {

      }
      currentRva += inst->size();
    }
  }

  return true;
}



std::set<uint32_t> Parser::searchLeaders()
{
  std::set<uint32_t> leaders;

  if (bin_.entryPoint() != 0)
  {
    leaders.insert(bin_.entryPoint());
  }

  for (auto& item : bin_.exportFunctions())
  {
    leaders.insert(item.rva);
  }

  return leaders;
}
