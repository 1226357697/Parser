#include "X86InstructionAnalyzer.h"
#include <capstone/x86.h>

bool X86InstructionAnalyzer::isReturn(const Instruction& insn) const
{
  return insn.hasGroup(Instruction::Group::Ret);
}

bool X86InstructionAnalyzer::isCall(const Instruction& insn) const
{
  return insn.hasGroup(Instruction::Group::Call);
}

bool X86InstructionAnalyzer::isUnconditionalJump(const Instruction& insn) const
{
  return insn.opCode == X86_INS_JMP;
}

bool X86InstructionAnalyzer::isConditionalJump(const Instruction& insn) const
{
  // 有 Jump 组但不是 JMP 指令
  return insn.hasGroup(Instruction::Group::Jump) &&
         insn.opCode != X86_INS_JMP;
}

bool X86InstructionAnalyzer::isIndirectJump(const Instruction& insn) const
{
  if (insn.opCode != X86_INS_JMP) return false;

  // 检查操作数：如果是寄存器或内存操作数，则为间接跳转
  if (insn.operands.empty()) return false;

  const auto& op = insn.operands[0];
  return op.isReg() || op.isMem();
}

std::optional<uint64_t> X86InstructionAnalyzer::getJumpTarget(const Instruction& insn) const
{
  if (!isUnconditionalJump(insn) && !isConditionalJump(insn)) {
    return std::nullopt;
  }

  if (isIndirectJump(insn)) {
    return std::nullopt;  // 无法静态确定
  }

  // 直接跳转：操作数是立即数
  if (!insn.operands.empty() && insn.operands[0].isImm()) {
    return static_cast<uint64_t>(insn.operands[0].imm().value);
  }

  return std::nullopt;
}

std::optional<uint64_t> X86InstructionAnalyzer::getCallTarget(const Instruction& insn) const
{
  if (!isCall(insn)) return std::nullopt;

  // 间接调用: call rax, call [rax]
  if (!insn.operands.empty()) {
    const auto& op = insn.operands[0];
    if (op.isReg() || op.isMem()) {
      return std::nullopt;
    }
    if (op.isImm()) {
      return static_cast<uint64_t>(op.imm().value);
    }
  }

  return std::nullopt;
}

bool X86InstructionAnalyzer::isNop(const Instruction& insn) const
{
  if (insn.opCode == X86_INS_NOP) return true;

  // xchg eax, eax 也是 NOP
  if (insn.opCode == X86_INS_XCHG && insn.operands.size() == 2) {
    const auto& op0 = insn.operands[0];
    const auto& op1 = insn.operands[1];
    if (op0.isReg() && op1.isReg() && op0.reg().reg == op1.reg().reg) {
      return true;
    }
  }

  // lea rax, [rax + 0] 也是 NOP
  if (insn.opCode == X86_INS_LEA && insn.operands.size() == 2) {
    const auto& dst = insn.operands[0];
    const auto& src = insn.operands[1];
    if (dst.isReg() && src.isMem()) {
      const auto& mem = src.mem();
      // lea reg, [reg + 0] 且没有 index
      if (mem.hasBase() && !mem.hasIndex() && mem.disp == 0 &&
          dst.reg().reg == mem.base) {
        return true;
      }
    }
  }

  return false;
}

bool X86InstructionAnalyzer::isInterrupt(const Instruction& insn) const
{
  return insn.hasGroup(Instruction::Group::Interrupt);
}

bool X86InstructionAnalyzer::affectsControlFlow(const Instruction& insn) const
{
  return isReturn(insn) || isCall(insn) ||
    isUnconditionalJump(insn) || isConditionalJump(insn) ||
    isInterrupt(insn);
}
RVA_t X86InstructionAnalyzer::getNextAddress(const Instruction& insn) const
{
  return insn.address + insn.size();
}
