#pragma once

#include "BinaryModule.h"
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include "Function.h"
#include "CrossReference.h"
#include "MemoryRegion.h"

enum EntranceType : uint32_t
{
  kNone = 0,
  kFunction,
  kBlock,
  kMaybeGap
};

struct Entrance
{
  EntranceType type;
  RVA_t rva;

  bool operator<(const Entrance& other) const
  {
    return rva < other.rva;
  }
};

class Parser
{
public:
  Parser(BinaryModule& bin); 
  void analyze();

  void exploreBlock(RVA_t entry, bool isFunctionEntry = false);

  inline std::map<RVA_t, std::shared_ptr<Function>> function()const { return functions_;}

  inline const BinaryModule& binarayModule() const { return bin_;}

protected:

  std::set<Entrance> collectEntryPoints();

  void disassembleBlock(std::shared_ptr<BasicBlock> block, std::stack<RVA_t>& workList);

  void analyzeJumpTables();

  bool buildFunctions();

  void collectFunctionBlocks(std::shared_ptr<Function> func);

  void exploreCodeRegion();


  void linkBlocks(std::shared_ptr<BasicBlock> predecessor, std::shared_ptr<BasicBlock> successor);
  
  std::shared_ptr<BasicBlock> getBlock(RVA_t addr);
  std::shared_ptr<BasicBlock> getOrCreateBlock(RVA_t addr, bool* isNew = nullptr);
  std::shared_ptr<BasicBlock> findBlockContaining(RVA_t rva);
  uint32_t GetGasSize(RVA_t rva);

  inline void addXref(const Xref& xref) { xrefManager_ .addXref( xref); };

  // 打印相关信息
  void printSummary();
  void printBlocks();
  void printFunction();
  void printRemainRegion(const MemoryRegion& region);

private:
  BinaryModule& bin_;

  std::set<RVA_t> visitedNode_;
  std::queue<RVA_t> pendingEntry_;
  std::set<RVA_t> functionEntries_;

  std::map<RVA_t, std::shared_ptr<BasicBlock>> blocks_;
  std::map<RVA_t, std::shared_ptr<Function>> functions_;


  XrefManager xrefManager_;
};