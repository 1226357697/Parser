#include "JumpTableAnalyzer.h"

JumpTableAnalyzer::JumpTableAnalyzer(BinaryModule& bin)
:bin_(bin)
{
}

std::optional<JumpTableInfo> JumpTableAnalyzer::analyze(BasicBlock& bb)
{
  if (bb.endType() != BasicBlock::EndType::kIndirectJump)
    return std::nullopt;

  auto lastInst = bb.instructions().back();
  Operand& opr1 = lastInst->operands.at(0);
  if (!isJumpTableOperand(opr1))
    return std::nullopt;

  const MemOperand& mmopr = opr1.mem();
  JumpTableInfo jtInfo;
  jtInfo.tableBase = mmopr.disp;
  jtInfo.indexReg = mmopr.index;
  jtInfo.scale = mmopr.scale;

  // 双向探测：正向和负向
  auto forwardTargets = probeJumpTable(jtInfo.tableBase, 1, 256);
  auto backwardTargets = probeJumpTable(jtInfo.tableBase - jtInfo.scale, -1, 256);

  // 合并结果
  // 负向目标需要反转并放在前面
  std::reverse(backwardTargets.begin(), backwardTargets.end());

  jtInfo.minIndex = -static_cast<int32_t>(backwardTargets.size());
  jtInfo.maxIndex = static_cast<int32_t>(forwardTargets.size()) - 1;

  jtInfo.cases = std::move(backwardTargets);
  jtInfo.cases.insert(jtInfo.cases.end(),
    forwardTargets.begin(), forwardTargets.end());

  return jtInfo;
}


std::vector<Addr_t> JumpTableAnalyzer::probeJumpTable(Addr_t tableBase, int direction, int maxEntries)
{
  std::vector<Addr_t> targets;

  for (int i = 0; i < maxEntries; i++) {
    int offset = direction * i * bin_.getPointerSize();
    Addr_t entryAddr = tableBase + offset;

    // 读取跳转表条目
    auto targetOpt = bin_.readPointer(bin_.VA2RVA(entryAddr));
    if (!targetOpt) break;

    Addr_t target = *targetOpt;

    // 验证目标地址
    if (!bin_.validAddress(bin_.VA2RVA(target))) break;

    // 检查是否与已有目标重复太多（可能到达表尾）
    //if (hasTooManyDuplicates(targets, target)) break;

    targets.push_back(target);
  }

  return targets;
}

bool JumpTableAnalyzer::isJumpTableOperand(const Operand& opr)
{
  if (!opr.isMem())
    return false;

  const MemOperand& mmopr = opr.mem();
  return mmopr.hasDisp() && mmopr.hasIndex() && mmopr.hasScale();
}