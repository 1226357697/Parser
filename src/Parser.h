#pragma once

#include "BinaryModule.h"
#include <vector>
#include <map>
#include "Function.h"
#include "CrossReference.h"

enum EntranceType : uint32_t
{
  kNone = 0,
  kFunction,
  kBlock
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
  bool parseFunctions();
  
  bool exportFunctionToDot(RVA_t rva,  const std::string& fileName);
  void exportFuncrionsToDot(const std::string& dirName);

protected:
  std::set<Entrance> collectModuleEntrance();
  bool buildFunctions();
  void buildFunctionCFG(std::shared_ptr<Function> function);
  void exploreBuildBlock(const std::set<Entrance>& entrance);
  void exploreBlocks(const Entrance& entrance, std::set<Entrance>& exploreCall);
  
  std::shared_ptr<BasicBlock> getBlock(RVA_t addr);
  std::shared_ptr<BasicBlock> getOrCreateBlock(RVA_t addr, bool* isNew = nullptr);
  std::shared_ptr<BasicBlock> findBlockContaining(RVA_t rva);

  inline void addXref(const Xref& xref) { xrefManager_ .addXref( xref); };
private:
  BinaryModule& bin_;
  std::map<RVA_t, std::shared_ptr<BasicBlock>> blocks_;
  std::map<RVA_t, std::shared_ptr<Function>> functions_;


  XrefManager xrefManager_;
};