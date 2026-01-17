#pragma once

#include <vector>
#include <string>
#include <span>
#include "Operand.h"
#include "BinaryTypes.h"

struct  Instruction
{
	Addr_t address;
	std::vector<uint8_t> bytes;
	std::string mnemonic;
	std::string operandsStr;

  uint32_t opCode;
	std::vector<Operand> operands;

  // 指令分组（用于快速分类）
  enum class Group : uint64_t {
    None = 0,
    Jump = 1 << 0,
    Call = 1 << 1,
    Ret = 1 << 2,
    Interrupt = 1 << 3,
    Arithmetic = 1 << 4,
    Logic = 1 << 5,
    Move = 1 << 6,
    Compare = 1 << 7,
    Stack = 1 << 8,
    // ...
  };
  uint64_t groups = 0;

  // 隐式读写的寄存器（如 push 隐式写 esp）
  std::vector<RegId> implicitReads;
  std::vector<RegId> implicitWrites;

  // 便捷方法
  size_t size() const { return bytes.size(); }
  size_t operandCount() const { return operands.size(); }

  bool hasGroup(Group g) const { return (groups & static_cast<uint32_t>(g)) != 0; }

  // 获取第一个内存操作数（跳转表分析常用）
  const MemOperand* getMemOperand() const {
    for (const auto& op : operands) {
      if (op.isMem()) return &op.mem();
    }
    return nullptr;
  }
};
