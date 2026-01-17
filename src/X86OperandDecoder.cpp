#include "X86OperandDecoder.h"



X86OperandDecoder::X86OperandDecoder(csh handle)
:OperandDecoder((void*)handle)
{
}

void X86OperandDecoder::decode(cs_insn* csInsn, Instruction& inst)
{
  cs_x86* x86 = &csInsn->detail->x86;

  // 解码显式操作数
  for (uint8_t i = 0; i < x86->op_count; i++) {
    cs_x86_op* op = &x86->operands[i];
    inst.operands.push_back(decodeOperand(op));
  }

  
  // 解码隐式寄存器
  for (uint8_t i = 0; i < csInsn->detail->regs_read_count; i++) {
    inst.implicitReads.push_back(makeRegId((x86_reg)csInsn->detail->regs_read[i]));
  }
  for (uint8_t i = 0; i < csInsn->detail->regs_write_count; i++) {
    inst.implicitWrites.push_back(makeRegId((x86_reg)csInsn->detail->regs_write[i]));
  }

  // 解码指令分组
  for (uint8_t i = 0; i < csInsn->detail->groups_count; i++) {
    inst.groups |= mapGroup(csInsn->detail->groups[i]);
  }
}

std::string X86OperandDecoder::regName(RegId reg) const
{
  if (reg.arch() != RegId::Arch::X86) return "??";
  return cs_reg_name(csHandle(), reg.reg());
}

RegId X86OperandDecoder::makeRegId(x86_reg reg) const
{
  return RegId::make(RegId::Arch::X86, static_cast<uint32_t>(reg));
}

Operand X86OperandDecoder::decodeOperand(cs_x86_op* op) const
{
  switch (op->type) {
  case X86_OP_REG:
    return Operand::makeReg(
      makeRegId(op->reg),
      op->size,
      mapAccess(op->access)
    );

  case X86_OP_IMM:
    return Operand::makeImm(op->imm, op->size);

  case X86_OP_MEM: {
    MemOperand mem;
    mem.base = op->mem.base != X86_REG_INVALID
      ? makeRegId(static_cast<x86_reg>(op->mem.base))
      : RegId::invalid();
    mem.index = op->mem.index != X86_REG_INVALID
      ? makeRegId(static_cast<x86_reg>(op->mem.index))
      : RegId::invalid();
    mem.scale = op->mem.scale;
    mem.disp = op->mem.disp;
    mem.size = op->size;
    return Operand::makeMem(mem, mapAccess(op->access));
  }

  default:
    return Operand{};
  }
}

OperandAccess X86OperandDecoder::mapAccess(uint8_t csAccess) const
{
  uint8_t result = 0;
  if (csAccess & CS_AC_READ) result |= static_cast<uint8_t>(OperandAccess::Read);
  if (csAccess & CS_AC_WRITE) result |= static_cast<uint8_t>(OperandAccess::Write);
  return static_cast<OperandAccess>(result);
}

uint32_t X86OperandDecoder::mapGroup(uint8_t csGroup) const
{
  switch (csGroup) {
  case X86_GRP_JUMP: return static_cast<uint32_t>(Instruction::Group::Jump);
  case X86_GRP_CALL: return static_cast<uint32_t>(Instruction::Group::Call);
  case X86_GRP_RET: return static_cast<uint32_t>(Instruction::Group::Ret);
  case X86_GRP_INT: return static_cast<uint32_t>(Instruction::Group::Interrupt);
  default: return 0;
  }
}

csh X86OperandDecoder::csHandle()const
{
  return (csh)OperandDecoder::handle();
}