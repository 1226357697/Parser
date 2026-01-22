#pragma once

#include "BinaryModule.h"
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include "Function.h"
#include "CrossReference.h"
#include "MemoryRegion.h"

enum class ExploreType {
  Function,  // 函数入口
  Block,      // 普通块（跳转表目标等）
  MaybeFunction
};

class Parser
{
public:
  Parser(BinaryModule& bin); 
  void analyze();

  void exploreBlock(RVA_t entry, ExploreType entryType = ExploreType::Block, MemoryRegion* region = nullptr);

  inline std::map<RVA_t, std::shared_ptr<Function>> function()const { return functions_;}

  inline const BinaryModule& binarayModule() const { return bin_;}

protected:

  void collectEntryPoints();

  void disassembleBlock(std::shared_ptr<BasicBlock> block, std::stack<RVA_t>& workList);

  void analyzeJumpTables();

  bool buildFunctions();

  void collectFunctionBlocks(std::shared_ptr<Function> func);

  RVA_t findNextUnexploredAddress(MemoryRegion& codeRegion);

  void exploreCodeRegion();


  void linkBlocks(std::shared_ptr<BasicBlock> predecessor, std::shared_ptr<BasicBlock> successor);
  
  std::shared_ptr<BasicBlock> getBlock(RVA_t addr);
  std::shared_ptr<BasicBlock> getOrCreateBlock(RVA_t addr, bool* isNew = nullptr);
  std::shared_ptr<BasicBlock> findBlockContaining(RVA_t rva);
  uint32_t GetGapSize(RVA_t rva);

  inline void addXref(const Xref& xref) { xrefManager_ .addXref( xref); };

  // 打印相关信息
  void printSummary();
  void printBlocks();
  void printFunction();
  void printRemainRegion(const MemoryRegion& region);

private:
  BinaryModule& bin_;

  std::set<RVA_t> visitedAddresses_;
  std::queue<RVA_t> pendingEntry_;
  std::set<RVA_t> functionEntries_;

  std::map<RVA_t, std::shared_ptr<BasicBlock>> blocks_;
  std::map<RVA_t, std::shared_ptr<Function>> functions_;

  std::vector<MemoryRegion> unParseCodeRegion_;

  XrefManager xrefManager_;
};