#pragma once

#include "BinaryModule.h"
#include "BasicBlock.h"
#include "Instruction.h"
#include <vector>

struct JumpTableInfo
{
  Addr_t tableBase;
  RegId indexReg;
  uint8_t scale;
  int32_t minIndex;
  int32_t maxIndex;
  std::vector<Addr_t> cases;
};


class JumpTableAnalyzer
{
public:
  JumpTableAnalyzer(BinaryModule& bin);
  virtual ~JumpTableAnalyzer() = default;

  std::optional<JumpTableInfo> analyze(BasicBlock& bb);

private:
  std::vector<Addr_t> probeJumpTable(Addr_t tableBase, int direction, int maxEntries);

  virtual bool isJumpTableOperand(const Operand& opr);
private:
  BinaryModule& bin_;
};
