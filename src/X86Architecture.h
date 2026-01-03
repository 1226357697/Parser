#pragma once
#include "architecture.h"
#include "CapstoneDisassembler.h"


class X86Architecture : public Architecture {
public:
  inline std::string name() const override { return "x86"; }
  inline size_t pointerSize() const override { return 4; }
  inline Endian endian() const override { return Endian::Little; }

  std::unique_ptr<Disassembler> createDisassembler() override;


  // Inherited via Architecture
  size_t maxInstructionSize() override;

};