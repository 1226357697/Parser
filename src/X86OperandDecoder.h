#pragma once

#include "OperandDecoder.h"

class X86OperandDecoder : public OperandDecoder
{

public:
  X86OperandDecoder(csh handle);
  void decode(cs_insn* csInsn, Instruction& inst) override;
  std::string regName(RegId reg) const override;


private:
  RegId makeRegId(x86_reg reg)const;
  Operand decodeOperand(cs_x86_op* op) const;
  OperandAccess mapAccess(uint8_t csAccess )const ;
  uint32_t mapGroup(uint8_t csGroup)const;
  csh csHandle()const;
};