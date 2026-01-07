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
  instructions_.push_back(inst);

  endAddress_ = inst->address + inst->size();
}

void BasicBlock::addSuccessor(std::shared_ptr<BasicBlock> bb)
{
  successors_.push_back(bb);
}

void BasicBlock::addPredecessor(std::shared_ptr<BasicBlock> bb)
{
  predecessors_.push_back(bb);
}

size_t BasicBlock::getSize()
{
  return endAddress_ - startAddress_;
}

BasicBlock::EndTYpe BasicBlock::endType()
{
  return EndTYpe::Invalid;
}
