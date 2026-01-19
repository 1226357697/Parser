#include "Parser.h"
#include <unordered_map>
#include <stack>
#include <format>
#include <iostream>
#include <filesystem>
#include"JumpTableAnalyzer.h"


Parser::Parser(BinaryModule& bin)
:bin_(bin)
{

}

void Parser::analyze()
{
  // 1. 收集确定入口
  collectEntryPoints();

  // 2. 探索所有函数（包括 call 发现的）
  while (!pendingEntry_.empty()) {
    RVA_t entry = pendingEntry_.front();
    pendingEntry_.pop();

    if (visitedNode_.count(entry)) continue;

    exploreBlock(entry, true);
  }
  // 3. 分析跳转表，可能发现新块
  analyzeJumpTables();

  // 4. 构建函数
  buildFunctions();

  // 5. 扫描未探索的代码区域
  exploreCodeRegion();

}

void Parser::exploreBlock(RVA_t entry, bool isFunctionEntry )
{
  if(isFunctionEntry)
    functionEntries_.insert(entry);

  std::stack<RVA_t> workList;
  workList.push(entry);

  while (!workList.empty()) {
    RVA_t rva = workList.top();
    workList.pop();

    if (visitedNode_.count(rva)) continue;
    visitedNode_.insert(rva);

    auto block = getOrCreateBlock(rva);
    if (isFunctionEntry && rva == entry)
    {
      block->setTag(BasicBlock::Tag::kFunctionEntry);
    }

    disassembleBlock(block, workList);
  }
}

void Parser::disassembleBlock(std::shared_ptr<BasicBlock> block,
  std::stack<RVA_t>& workList) {
  RVA_t rva = block->startAddress();
  auto analyzer = bin_.instructionAnalyzer();

  while (auto inst = bin_.disassembleOne(rva)) {
    auto instPtr = std::make_shared<Instruction>(std::move(*inst));
    block->addInstruction(instPtr);

    // 处理控制流
    if (analyzer->isCall(*instPtr)) {
      if (auto target = analyzer->getCallTarget(*instPtr)) {
        pendingEntry_.push(*target);  // 发现新块
      }
    }

    if (analyzer->isConditionalJump(*instPtr)) {
      block->setEndType(BasicBlock::EndType::kConditionalJump);

      if (auto target = analyzer->getJumpTarget(*instPtr)) {
        workList.push(*target);
        linkBlocks(block, getOrCreateBlock(*target));
      }

      RVA_t fall = analyzer->getNextAddress(*instPtr);
      workList.push(fall);
      linkBlocks(block, getOrCreateBlock(fall));
      return;
    }

    if (analyzer->isUnconditionalJump(*instPtr)) {
      if (analyzer->isIndirectJump(*instPtr)) {
        block->setEndType(BasicBlock::EndType::kIndirectJump);
      }
      else {
        block->setEndType(BasicBlock::EndType::kUnconditionalJump);
        if (auto target = analyzer->getJumpTarget(*instPtr)) {
          workList.push(*target);
          linkBlocks(block, getOrCreateBlock(*target));
        }
      }
      return;
    }

    if (analyzer->isReturn(*instPtr)) {
      block->setEndType(BasicBlock::EndType::kReturn);
      return;
    }

    rva += instPtr->size();

    // 遇到已有块
    if (visitedNode_.count(rva)) {
      block->setEndType(BasicBlock::EndType::kFallThrough);
      linkBlocks(block, getOrCreateBlock(rva));
      return;
    }
  }
}

void Parser::analyzeJumpTables() {
  JumpTableAnalyzer jtAnalyzer(bin_);

  // 收集所有间接跳转块
  std::vector<std::shared_ptr<BasicBlock>> indirectBlocks;
  for (auto& [rva, block] : blocks_) {
    if (block->endType() == BasicBlock::EndType::kIndirectJump) {
      indirectBlocks.push_back(block);
    }
  }

  // 分析每个跳转表
  for (auto& block : indirectBlocks) {
    if (auto jtInfo = jtAnalyzer.analyze(*block)) {
      for (Addr_t target : jtInfo->cases) {
        RVA_t targetRva = bin_.VA2RVA(target);

        // 探索新目标
        if (!visitedNode_.count(targetRva)) {
          exploreBlock(targetRva);  // 复用探索逻辑
        }

        // 链接 CFG
        if (auto targetBlock = getBlock(targetRva)) {
          linkBlocks(block, targetBlock);
        }
      }
      block->addFlags(BasicBlock::Flags::kJumpTable);
    }
  }
}



bool Parser::buildFunctions()
{
  for (RVA_t entry : functionEntries_) {
    if (blocks_.find(entry) == blocks_.end()) {
      continue;  
    }

    auto func = std::make_shared<Function>(bin_, entry, blocks_[entry]);
    collectFunctionBlocks(func);
    functions_[entry] = func;
  }

  return true;
}

void Parser::collectFunctionBlocks(std::shared_ptr<Function> func)
{
  std::set<RVA_t> visited;
  std::stack<std::shared_ptr<BasicBlock>> workList;

  workList.push(func->entryBlock());

  while (!workList.empty()) {
    auto block = workList.top();
    workList.pop();

    if (visited.count(block->startAddress())) continue;
    visited.insert(block->startAddress());

    func->addBlock(block);

    // 添加后继块（但不跨越其他函数入口）
    for (auto& succ : block->getSuccessors()) {
      // 如果后继是另一个函数的入口，不加入当前函数
      if (succ->tag() == BasicBlock::Tag::kFunctionEntry &&
        succ->startAddress() != func->rva()) {
        continue;  // 这是 tail call 或跳转到其他函数
      }
      workList.push(succ);
    }
  }

}

void Parser::exploreCodeRegion()
{
  printBlocks();
  printFunction();

  std::vector<MemoryRegion> unParseCodeRegion;

  for (auto& section : bin_.getSections())
  {
    if (bin_.isCodeSection(&section))
    {
      MemoryRegion region(MemoryRegion::Protect::Execute, section.virtual_address(), section.virtual_address() + section.size());

      unParseCodeRegion.push_back(region);
    }
  }

  assert(unParseCodeRegion.size() == 1);
  MemoryRegion& codeRegion = unParseCodeRegion.at(0);

  for (auto& [rva, func ] : functions_)
  {
    codeRegion.allocate(func->rva(), func->end());
  }

  printRemainRegion(codeRegion);


}



void Parser::linkBlocks(std::shared_ptr<BasicBlock> predecessor, std::shared_ptr<BasicBlock> successor)
{
  predecessor->addSuccessor(successor);
  successor->addPredecessor(predecessor);
}

std::set<Entrance> Parser::collectEntryPoints()
{
  std::set<Entrance> entrance;

  if (bin_.entryPoint() != 0)
  {
    //entrance.insert({ EntranceType::kFunction, bin_.entryPoint()});
    pendingEntry_.push(bin_.entryPoint());
  }

  for (auto& item : bin_.exportFunctions())
  {
    //entrance.insert({ EntranceType::kFunction, item.rva});
    pendingEntry_.push(item.rva);
  }

  return entrance;
}


std::shared_ptr<BasicBlock> Parser::getBlock(RVA_t addr)
{
  if (auto iter = blocks_.find(addr); iter != blocks_.end())
    return iter->second;

  return nullptr;
}

std::shared_ptr<BasicBlock> Parser::getOrCreateBlock(RVA_t addr, bool* isNew)
{
  if(isNew) *isNew = false;

  // 已存在
  if (auto existing = getBlock(addr)) {
    return existing;
  }

  if (isNew) *isNew = true;

  // 检查是否在已有块中间
  if (auto existing = findBlockContaining(addr)) {
    auto newBlock = existing->splitAt(addr);
    newBlock->setTag(BasicBlock::Tag::kNormal);
    blocks_[addr] = newBlock;
    return newBlock;
  }

  // 创建新块
  auto block = std::make_shared<BasicBlock>(addr);
  block->setTag(BasicBlock::Tag::kNormal);
  blocks_[addr] = block;
  return block;
}

std::shared_ptr<BasicBlock> Parser::findBlockContaining(RVA_t rva)
{
  auto iter = blocks_.upper_bound(rva);
  if (iter == blocks_.begin())
  {
    iter = blocks_.upper_bound(rva);
    if (iter == blocks_.begin())
      return nullptr;
  }

  --iter;
  if (rva >= iter->second->startAddress() && rva < iter->second->endAddress())
    return iter->second;

  return nullptr;
}

uint32_t Parser::GetGasSize(RVA_t rva)
{
  uint32_t gasSize = 0;
  auto analyzer = bin_.instructionAnalyzer();

  RVA_t currentRva = rva;
  while (auto inst = bin_.disassembleOne(currentRva))
  {
    if (analyzer->isNop(*inst))
    {
      gasSize += inst->size();
    }
    else
    {
      break;
    }

    currentRva += inst->size();
  }
  return gasSize;
}

void Parser::printSummary()
{
  printFunction();
  //printRemainRegion();
}

void Parser::printBlocks()
{
  printf("------------------------------- blocks ------------------------------- \n");
  printf("Count: %llu\n",blocks_.size());
  for (auto& [rva, bb] : blocks_)
  {
    printf("block rva:%08X size:%08x\n", bb->startAddress(), bb->getSize());
  }
  printf("===================================================================== \n\n");
}

void Parser::printFunction()
{
  printf("------------------------------- functions ------------------------------- \n");
  printf("Count: %llu\n", functions_.size());
  for (auto& [rva, func]: functions_)
  {
    printf("function rva:%08X end:%08X blocks:%d\n", func->rva(), func->end(), func->blocks().size());
  }
  printf("========================================================================= \n\n");
}

void Parser::printRemainRegion(const MemoryRegion& region)
{
  printf("------------------------------- Un Parse CodeRegion ------------------------------- \n");
  printf("Count: %llu\n", region.freeBlock().size());
  for (auto& b : region.freeBlock())
  {
    printf("unparse region start:%08X end:%08X\n", b.start, b.end);
  }
  printf("================================================================================== \n\n");
}

