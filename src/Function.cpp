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


