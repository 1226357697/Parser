#pragma once

#include "Instruction.h"
#include <capstone/capstone.h>

class OperandDecoder 
{
public:
  OperandDecoder(void* handle);

  virtual ~OperandDecoder() = default;

  virtual void decode(cs_insn* csInsn, Instruction& inst) = 0;


  virtual std::string regName(RegId reg) const = 0;

protected:
  virtual inline void* handle() const{ return handle_;}
private:
  void* handle_;
};