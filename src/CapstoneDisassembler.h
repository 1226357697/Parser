#pragma once
#include "disassembler.h"
#include "OperandDecoder.h"
#include <memory>
#include <capstone/capstone.h>
#include <stdexcept>

class CapstoneDisassembler : public Disassembler
{
public:
  CapstoneDisassembler(cs_arch arch, cs_mode mode);
  ~CapstoneDisassembler();

  CapstoneDisassembler(const CapstoneDisassembler&) = delete;
  CapstoneDisassembler& operator=(const CapstoneDisassembler&) = delete;

  // Inherited via Disassembler
  std::vector<Instruction> disassemble(std::span<const uint8_t> code, uint64_t addr, size_t* outBytesConsumed) override;
  std::optional<Instruction> disassembleOne(std::span<const uint8_t> code, uint64_t addr, size_t* outBytesConsumed) override;

private:
  csh handle_;
  cs_insn* cachedInsn_ = nullptr;

  std::unique_ptr<OperandDecoder> decoder_;
};