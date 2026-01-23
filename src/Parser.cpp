#include "Parser.h"
#include <unordered_map>
#include <stack>
#include <format>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <capstone/capstone.h>
#include"JumpTableAnalyzer.h"

#define INVALID_RVA (-1)

Parser::Parser(BinaryModule& bin)
:bin_(bin)
{
  std::vector<MemoryRegion> unParseCodeRegion;

  for (auto& section : bin_.getSections())
  {
    if (bin_.isCodeSection(&section))
    {
      MemoryRegion region(MemoryRegion::Protect::Execute, section.virtual_address(), section.virtual_address() + section.size());

      unParseCodeRegion_.push_back(region);
    }
  }

  assert(unParseCodeRegion_.size() == 1);
}

void Parser::analyze()
{
  auto start = std::chrono::high_resolution_clock::now();

  // 1. 收集确定入口
  collectEntryPoints();

  // 2. 探索所有函数（包括 call 发现的）
  while (!pendingEntry_.empty()) {
    RVA_t entry = pendingEntry_.front();
    pendingEntry_.pop();

    if (visitedAddresses_.count(entry)) continue;

    exploreBlock(entry, ExploreType::Function, &unParseCodeRegion_.at(0));
  }
  // 3. 分析跳转表，可能发现新块
  analyzeJumpTables();


  //printBlocks();
  //printFunction();

  // 4. 扫描未探索的代码区域
  exploreCodeRegion();



  // 5. 分析跳转表，可能发现新块
  analyzeJumpTables();

  // 6. 构建函数
  buildFunctions();



  //printBlocks();
  //printFunction();

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  // 输出
  auto totalSeconds = duration.count() / 1000;
  auto minutes = totalSeconds / 60;
  auto seconds = totalSeconds % 60;
  auto ms = duration.count() % 1000;

  printf("Analyze completed in %lld min %lld sec %lld ms\n", minutes, seconds, ms);
}

void Parser::exploreBlock(RVA_t entry, ExploreType entryType, MemoryRegion* region)
{
  if(entryType == ExploreType::Function)
    functionEntries_.insert(entry);

  std::stack<RVA_t> workList;
  workList.push(entry);

  while (!workList.empty()) {
    RVA_t rva = workList.top();
    workList.pop();

    if (visitedAddresses_.count(rva)) continue;
    visitedAddresses_.insert(rva);

    auto block = getOrCreateBlock(rva);
    if (entryType == ExploreType::Function && rva == entry)
    {
      block->setTag(BasicBlock::Tag::kFunctionEntry);
    }

    disassembleBlock(block, workList);

    if(region)
      region->allocate(block->startAddress(), block->endAddress());
  }
}

void Parser::disassembleBlock(std::shared_ptr<BasicBlock> block, std::stack<RVA_t>& workList) 
{
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
    if (visitedAddresses_.count(rva)) {
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
        if (!visitedAddresses_.count(targetRva)) {
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
  std::stack<RVA_t> workList;  // 只存地址

  workList.push(func->entryBlock()->startAddress());

  while (!workList.empty()) {
    RVA_t rva = workList.top();
    workList.pop();

    if (visited.count(rva)) continue;
    visited.insert(rva);

    auto block = getBlock(rva);
    if (!block) continue;

    func->addBlock(block);

    for (auto& succ : block->getSuccessors()) {
      if (succ->tag() == BasicBlock::Tag::kFunctionEntry &&
        succ->startAddress() != func->rva()) {
        continue;
      }
      workList.push(succ->startAddress());
    }
  }

}

RVA_t Parser::findNextUnexploredAddress(MemoryRegion& codeRegion) {
  for (const auto& region : codeRegion.freeBlock()) {
    RVA_t addr = region.start;

    // 用 lower_bound 快速跳过已访问的地址
    auto it = visitedAddresses_.lower_bound(addr);

    // 找到第一个不在 visitedAddresses_ 中的地址
    while (addr < region.end) {
      if (it == visitedAddresses_.end() || *it != addr) {
        return addr;  // 找到未访问的地址
      }
      addr++;
      if (it != visitedAddresses_.end() && *it < addr) {
        it = visitedAddresses_.lower_bound(addr);
      }
    }
  }
  return INVALID_RVA;
}

void Parser::exploreCodeRegion()
{


  MemoryRegion& codeRegion = unParseCodeRegion_.at(0);

  while (true) {
    // 找到第一个未探索的地址
    RVA_t start = findNextUnexploredAddress(codeRegion);
    if (start == INVALID_RVA) break;


    // 跳过 NOP
    uint32_t gapSize = GetGapSize(start);
    if (gapSize > 0) {
      codeRegion.allocate(start, start + gapSize);
      continue;
    }

    // 检查是否可反汇编
    auto inst = bin_.disassembleOne(start);
    if (!inst) {
      codeRegion.allocate(start, start + 1);  // 标记为数据
      continue;
    }
    if (inst->opCode ==X86_INS_MOV || inst->opCode == X86_INS_PUSH )
    {
      int j =0;
    }
    else
    {
      codeRegion.allocate(start, start + inst->size());  // 标记为数据
      continue;
    }

    // 记录探索前的块数量
    size_t blocksBefore = blocks_.size();

    // 探索
    exploreBlock(start, ExploreType::Function, &codeRegion);

    //// 只更新新增的块
    //auto it = blocks_.begin();
    //std::advance(it, blocksBefore);  // 跳到新增的块
    //for (; it != blocks_.end(); ++it) {
    //  codeRegion.allocate(it->second->startAddress(), it->second->endAddress());
    //}

    //// 更新 codeRegion
    //for (auto& [rva, block] : blocks_) {
    //  codeRegion.allocate(block->startAddress(), block->endAddress());
    //}
  }

}



void Parser::linkBlocks(std::shared_ptr<BasicBlock> predecessor, std::shared_ptr<BasicBlock> successor)
{
  predecessor->addSuccessor(successor);
  successor->addPredecessor(predecessor);
}

void Parser::collectEntryPoints()
{
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
  if (blocks_.empty()) return nullptr;

  auto iter = blocks_.upper_bound(rva);
  if (iter == blocks_.begin()) return nullptr;

  --iter;
  if (rva >= iter->second->startAddress() && rva < iter->second->endAddress())
    return iter->second;

  return nullptr;
}

uint32_t Parser::GetGapSize(RVA_t rva)
{
  uint32_t gapSize = 0;
  auto analyzer = bin_.instructionAnalyzer();

  RVA_t currentRva = rva;
  while (bin_.isCodeAddress(currentRva)) {  // 只在代码段内检查
    if (currentRva  == 0x14ec6)
    {
      int j =0;
    }

    auto inst = bin_.disassembleOne(currentRva);
    if (!inst) break;

    if (analyzer->isNop(*inst) || analyzer->isInterrupt(*inst)) {
      gapSize += inst->size();
      currentRva += inst->size();
    }
    else {
      break;
    }
  }
  return gapSize;
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

