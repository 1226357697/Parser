#include "X86InstructionAnalyzer.h"
#include <unordered_set>

bool X86InstructionAnalyzer::isReturn(const Instruction& insn) const
{
  return insn.mnemonic == "ret" ||
    insn.mnemonic == "retn" ||
    insn.mnemonic == "retf";
}

bool X86InstructionAnalyzer::isCall(const Instruction& insn) const
{
  return insn.mnemonic == "call";
}

bool X86InstructionAnalyzer::isUnconditionalJump(const Instruction& insn) const
{
  return insn.mnemonic == "jmp";
}

bool X86InstructionAnalyzer::isConditionalJump(const Instruction& insn) const
{
  const auto& m = insn.mnemonic;
  if (m.empty() || m[0] != 'j') return false;
  if (m == "jmp") return false;

  // je, jne, jl, jle, jg, jge, jb, jbe, ja, jae, js, jns, jo, jno...
  static const std::unordered_set<std::string> jcc = {
      "je", "jz", "jne", "jnz", "jl", "jnge", "jle", "jng",
      "jg", "jnle", "jge", "jnl", "jb", "jnae", "jc", "jbe", "jna",
      "ja", "jnbe", "jae", "jnb", "jnc", "js", "jns", "jo", "jno",
      "jp", "jpe", "jnp", "jpo", "jcxz", "jecxz", "jrcxz",
      "loop", "loope", "loopz", "loopne", "loopnz"
  };
  return jcc.count(m) > 0;
}

bool X86InstructionAnalyzer::isIndirectJump(const Instruction& insn) const
{
  if (insn.mnemonic != "jmp") return false;

  const auto& op = insn.operands;
  // jmp rax, jmp [rax], jmp qword ptr [rax+rcx*8]
  // 直接跳转是 jmp 0x1234

  // 简单判断：不是纯数字就是间接
  return !op.empty() && !std::isdigit(op[0]) && op[0] != '0';
}

std::optional<uint64_t> X86InstructionAnalyzer::getJumpTarget(const Instruction& insn) const
{
  if (!isUnconditionalJump(insn) && !isConditionalJump(insn)) {
    return std::nullopt;
  }

  if (isIndirectJump(insn)) {
    return std::nullopt;  // 无法静态确定
  }

  return parseAddress(insn.operands);
}

std::optional<uint64_t> X86InstructionAnalyzer::getCallTarget(const Instruction& insn) const
{
  if (!isCall(insn)) return std::nullopt;

  // 间接调用: call rax, call [rax]
  const auto& op = insn.operands;
  if (!op.empty() && !std::isdigit(op[0]) && op[0] != '0') {
    return std::nullopt;
  }

  return parseAddress(op);
}

bool X86InstructionAnalyzer::isNop(const Instruction& insn) const
{
  return insn.mnemonic == "nop" ||
    (insn.mnemonic == "xchg" && insn.operands == "eax, eax") ||
    (insn.mnemonic == "lea" && isNopLea(insn));
}

bool X86InstructionAnalyzer::isInterrupt(const Instruction& insn) const
{
  return insn.mnemonic == "int" ||
    insn.mnemonic == "int3" ||
    insn.mnemonic == "syscall" ||
    insn.mnemonic == "sysenter";
}

bool X86InstructionAnalyzer::affectsControlFlow(const Instruction& insn) const
{
  return isReturn(insn) || isCall(insn) ||
    isUnconditionalJump(insn) || isConditionalJump(insn) ||
    isInterrupt(insn);
}
std::optional<uint64_t> X86InstructionAnalyzer::parseAddress(const std::string& op) const {
  try {
    return std::stoull(op, nullptr, 0);
  }
  catch (...) {
    return std::nullopt;
  }
}

bool X86InstructionAnalyzer::isNopLea(const Instruction& insn) const {
  // lea rax, [rax + 0] 等 NOP 变体
  return insn.operands.find("[") != std::string::npos &&
    insn.operands.find("0x0") != std::string::npos;
}
