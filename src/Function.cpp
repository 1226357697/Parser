#include "Function.h"
#include <iostream>
#include <format>
#include <stack>

Function::Function(BinaryModule& bin, RVA_t rva, std::shared_ptr<BasicBlock> block)
:bin_(bin), rva_(rva), entryBlock_(block)
{

}

Function::~Function()
{

}

void Function::addBlock(std::shared_ptr<BasicBlock> block)
{
  blocks_.insert({(RVA_t)block->startAddress(), block });
}

size_t Function::end() const
{
  auto it = blocks_.rbegin();
  if (it != blocks_.rend()) {
    RVA_t maxRva = it->first;
    std::shared_ptr<BasicBlock> block = it->second;
    auto lastInst = block->instructions().back();
    return lastInst->address + lastInst->size();
  }

  assert(false);
  return 0;
}


