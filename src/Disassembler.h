#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <vector>
#include <optional>

#include "Instruction.h"


class Disassembler {
  public:
      virtual ~Disassembler() = default;
      virtual std::vector<Instruction> disassemble(std::span<const uint8_t> code, uint64_t addr, size_t* outBytesConsumed) = 0;
      virtual std::optional<Instruction> disassembleOne(std::span<const uint8_t> code, uint64_t addr, size_t* outBytesConsumed) = 0;
  };