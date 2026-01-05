#include "Parser.h"
#include <format>
#include <iostream>

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
      std::cout 
      << std::format("{:08X} {}\t{}", inst->address + bin_.imageBase(), inst->mnemonic, inst->operands)
      << std::endl;;

      if (instAnalyzer->isConditionalJump(*inst))
      {
        auto target = instAnalyzer->getJumpTarget(*inst);
        auto fall = inst->address + inst->size();
        if (target && leaders.insert(target.value()).second)
        {
          workLists.push(target.value());
        }

        if (leaders.insert(fall).second)
        {
          workLists.push(fall);
        }

        break;
      }

      if (instAnalyzer->isUnconditionalJump(*inst))
      {
        auto target = instAnalyzer->getJumpTarget(*inst);

        if (target && leaders.insert(target.value()).second)
        {
          workLists.push(target.value());
        }

        break;
      }

      if (instAnalyzer->isReturn(*inst))
      {
        break;
      }
      currentRva += inst->size();
    }

    std::cout << "---------------------------------------------------" <<std::endl;
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
