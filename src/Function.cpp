#include "Function.h"
#include <iostream>
#include <format>
#include <stack>

Function::Function(BinaryModule& bin, RVA_t rva)
:bin_(bin), rva_(rva)
{

}

Function::~Function()
{

}

bool Function::parse()
{
  std::set<RVA_t> leaders;
  std::set<RVA_t> visited;
  std::stack<RVA_t> workLists;

  leaders.insert(rva());
  workLists.push(rva());

  auto instAnalyzer = bin_.instructionAnalyzer();

  while (!workLists.empty())
  {
    RVA_t rva = workLists.top();
    workLists.pop();

    if (visited.count(rva))
      continue;
    visited.insert(rva);

    std::shared_ptr<BasicBlock> block = getOrCreateBlock(rva);

    RVA_t currentRva = rva;
    while (auto inst = bin_.disassembleOne(currentRva))
    {
      auto inst_ptr = std::make_shared<Instruction>(std::move(*inst));
      block->addInstruction(inst_ptr);

      std::cout
        << std::format("{:08X} {}\t{}", inst_ptr->address + bin_.imageBase(), inst_ptr->mnemonic, inst_ptr->operands)
        << std::endl;

      
      if (instAnalyzer->isConditionalJump(*inst_ptr))
      {
        block->setEndType(BasicBlock::EndType::ConditionalJump);
        auto target = instAnalyzer->getJumpTarget(*inst_ptr);
        auto fall = inst_ptr->address + inst_ptr->size();
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

      if (instAnalyzer->isUnconditionalJump(*inst_ptr))
      {
        if (instAnalyzer->isIndirectJump(*inst_ptr))
        {
          block->setEndType(BasicBlock::EndType::IndirectJump);
        }
        else
        {
          block->setEndType(BasicBlock::EndType::UnconditionalJump);
        }

        auto target = instAnalyzer->getJumpTarget(*inst_ptr);

        if (target && leaders.insert(target.value()).second)
        {
          workLists.push(target.value());
        }

        break;
      }

      if (instAnalyzer->isReturn(*inst_ptr))
      {
        block->setEndType(BasicBlock::EndType::Return);
        break;
      }
      currentRva += inst_ptr->size();

      // 是否到到已有Block
      if (leaders.count(currentRva))
      {
        block->setEndType(BasicBlock::EndType::FallThrough);
        break;
      }

      // 检查是否进入了已有块的中间（需要分割）
      if (auto existing = findBlockContaining(currentRva)) {
        auto newBlock = existing->splitAt(currentRva);
        blocks_[currentRva] = newBlock;
        leaders.insert(currentRva);
        block->setEndType(BasicBlock::EndType::FallThrough);
        break;
      }
    }

    std::cout << "---------------------------------------------------" << std::endl;
  }

  entryBlock_ = blocks_[rva()];

  buildCFGEdges();

  return !blocks_.empty();
}

std::shared_ptr<BasicBlock> Function::getOrCreateBlock(RVA_t addr)
{
  // 已存在
  if (auto iter = blocks_.find(addr); iter != blocks_.end())
    return iter->second;

  // 检查是否在已有块中间
  if (auto existing = findBlockContaining(addr)) {
    auto newBlock = existing->splitAt(addr);
    blocks_[addr] = newBlock;
    return newBlock;
  }

  // 创建新块
  auto block = std::make_shared<BasicBlock>(addr);
  blocks_[addr] = block;
  return block;
}

std::shared_ptr<BasicBlock> Function::findBlockContaining(RVA_t rva)
{
  auto iter = blocks_.upper_bound(rva);
  if (iter == blocks_.begin())
    return nullptr;

  --iter;
  if (rva >= iter->second->startAddress() && rva < iter->second->endAddress())
    return iter->second;

  return nullptr;
}

void Function::buildCFGEdges()
{
  
}

