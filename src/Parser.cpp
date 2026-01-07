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

  buildBlocks(leaders);
  return true;
}



void Parser::buildBlocks(std::set<uint32_t>& leaders)
{
  std::unordered_map<uint32_t, std::shared_ptr<BasicBlock>> blocks_map(leaders.size());

  // fill the blocks_map
  for (uint32_t rva : leaders)
  {
    blocks_map[rva] = std::make_shared<BasicBlock>(rva);
  }

  auto instAnalyzer = bin_.instructionAnalyzer();

  for (uint32_t rva : leaders)
  {
    uint32_t blockRva = rva;
    uint32_t currentRva = rva;

    auto iter = blocks_map.find(blockRva);
    assert(iter != blocks_map.end());
    std::shared_ptr<BasicBlock> block = iter->second;


    while (auto inst = bin_.disassembleOne(currentRva))
    {
      auto inst_ptr = std::make_shared<Instruction>(std::move(*inst));
      block->addInstruction(inst_ptr);


      if (instAnalyzer->isConditionalJump(*inst))
      {
        block->setEndType(BasicBlock::EndTYpe::ConditionalJump);
        auto target = instAnalyzer->getJumpTarget(*inst);
        auto fall = inst->address + inst->size();
        
        if (target)
        {
          auto iter = blocks_map.find(target.value());
          assert(iter != blocks_map.end());
          block->addSuccessor(iter->second);
          iter->second->addPredecessor(block);
        }

        {
          auto iter = blocks_map.find(fall);
          assert(iter != blocks_map.end());
          block->addSuccessor(iter->second);
          iter->second->addPredecessor(block);
        }
        break;
      }

      if (instAnalyzer->isUnconditionalJump(*inst))
      {
        if (instAnalyzer->isIndirectJump(*inst))
        {
          block->setEndType(BasicBlock::EndTYpe::IndirectJump);
        }
        else
        {
          block->setEndType(BasicBlock::EndTYpe::UnconditionalJump);
        }

        auto target = instAnalyzer->getJumpTarget(*inst);

        if (target)
        {
          auto iter = blocks_map.find(target.value());
          assert(iter != blocks_map.end());
          block->addSuccessor(iter->second);
          iter->second->addPredecessor(block);
        }

        break;
      }

      if (instAnalyzer->isReturn(*inst))
      {
        break;
      }

      currentRva += inst_ptr->size();
      if(blocks_map.find(currentRva) != blocks_map.end())
        break;
    }

  }
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
