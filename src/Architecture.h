#pragma once
#include <memory>
#include <string>

#include "Disassembler.h"
#include "InstructionAnalyzer.h"


enum class Endian { Little, Big };

class Architecture {
public:
  virtual ~Architecture() = default;
  virtual std::string name() const = 0;
  virtual size_t pointerSize() const = 0;
  virtual Endian endian() const = 0;
  virtual std::unique_ptr<Disassembler> createDisassembler() = 0;
  virtual std::unique_ptr<InstructionAnalyzer> createInstructionAnalzer() = 0;
  virtual size_t maxInstructionSize() = 0;
};
