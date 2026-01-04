#include "X86Architecture.h"
#include "X86InstructionAnalyzer.h"

std::unique_ptr<Disassembler> X86Architecture::createDisassembler()
{
  return std::make_unique<CapstoneDisassembler>(CS_ARCH_X86, CS_MODE_32);
}

std::unique_ptr<InstructionAnalyzer> X86Architecture::createInstructionAnalzer()
{
  return std::make_unique<X86InstructionAnalyzer>();
}

size_t X86Architecture::maxInstructionSize()
{
  return 16;
}

