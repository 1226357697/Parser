#pragma once

#include <optional>
#include "Instruction.h"

class InstructionAnalyzer {
public:
  virtual ~InstructionAnalyzer() = default;

  // 控制流分类
  virtual bool isReturn(const Instruction& insn) const = 0;
  virtual bool isCall(const Instruction& insn) const = 0;
  virtual bool isUnconditionalJump(const Instruction& insn) const = 0;
  virtual bool isConditionalJump(const Instruction& insn) const = 0;
  virtual bool isIndirectJump(const Instruction& insn) const = 0;

  // 获取跳转目标
  virtual std::optional<uint64_t> getJumpTarget(const Instruction& insn) const = 0;
  virtual std::optional<uint64_t> getCallTarget(const Instruction& insn) const = 0;
  virtual RVA_t getNextAddress(const Instruction& insn) const = 0;

  // 更多分析
  virtual bool isNop(const Instruction& insn) const = 0;
  virtual bool isInterrupt(const Instruction& insn) const = 0;
  virtual bool affectsControlFlow(const Instruction& insn) const = 0;
};