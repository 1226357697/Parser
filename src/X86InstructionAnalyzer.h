#pragma once

#include "InstructionAnalyzer.h"

class X86InstructionAnalyzer : public InstructionAnalyzer {
public:
  bool isReturn(const Instruction& insn) const override;
  bool isCall(const Instruction& insn) const override;
  bool isUnconditionalJump(const Instruction& insn) const override;
  bool isConditionalJump(const Instruction& insn) const override;
  bool isIndirectJump(const Instruction& insn) const override;
  std::optional<uint64_t> getJumpTarget(const Instruction& insn) const override;
  std::optional<uint64_t> getCallTarget(const Instruction& insn) const override;
  RVA_t getNextAddress(const Instruction& insn) const override;
  bool isNop(const Instruction& insn) const override;
  bool isInterrupt(const Instruction& insn) const override;
  bool affectsControlFlow(const Instruction& insn) const override;

private:
  std::optional<uint64_t> parseAddress(const std::string& op) const;
  bool isNopLea(const Instruction& insn) const;

};