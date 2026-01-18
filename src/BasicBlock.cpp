#include "BasicBlock.h"
#include <assert.h>

BasicBlock::BasicBlock(RVA_t rva)
:startAddress_(rva)
, endAddress_(rva)         
, tag_(Tag::kNone)          
, endType_(EndType::kInvalid)
, flags_(Flags::kNone)
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

std::vector<std::shared_ptr<BasicBlock>> BasicBlock::getSuccessors() const
{
  std::vector<std::shared_ptr<BasicBlock>> result;
  for (const auto& weak : successors_) {
    if (auto ptr = weak.lock()) {
      result.push_back(ptr);
    }
  }
  return result;
}

const std::vector<std::shared_ptr<BasicBlock>>& BasicBlock::getPredecessors() const
{
  return predecessors_;
}

std::shared_ptr<BasicBlock> BasicBlock::splitAt(RVA_t rva)
{
  assert(rva >= startAddress() && rva < endAddress());
  if(rva < startAddress() || rva > endAddress())
    return nullptr;

  auto splitIt = std::find_if(instructions_.begin(), instructions_.end(),
    [rva](auto& inst) { return inst->address >= rva; });

  auto newBlock = std::make_shared<BasicBlock>(rva);
  for (auto iter = splitIt; iter != instructions_.end(); ++iter) 
  {
    newBlock->addInstruction(*iter);
  }
  instructions_.erase(splitIt, instructions_.end());

  if (!instructions_.empty())
    endAddress_ = instructions_.back()->address + instructions_.back()->size();

  newBlock->setEndType(endType_);
  endType_ = EndType::kFallThrough;
  return newBlock;
}

