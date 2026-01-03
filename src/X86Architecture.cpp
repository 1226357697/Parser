#include "X86Architecture.h"

std::unique_ptr<Disassembler> X86Architecture::createDisassembler()
{
  return std::make_unique<CapstoneDisassembler>(CS_ARCH_X86, CS_MODE_32);
}

size_t X86Architecture::maxInstructionSize()
{
  return 16;
}

