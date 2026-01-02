#include "BasicBlock.h"

BasicBlock::BasicBlock(RVA_t rva)
:startAddress_(rva)
{
}

BasicBlock::~BasicBlock()
{
}

void BasicBlock::addInstruction(std::shared_ptr<Instruction> inst)
{
}

size_t BasicBlock::getSize()
{
  return endAddress_;
}
