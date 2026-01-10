#pragma once

#include "BinaryModule.h"
#include <vector>
#include <map>
#include "Function.h"

class Parser
{
public:
  Parser(BinaryModule& bin);
  bool parseFunctions();
  bool parseFunction(RVA_t startRva);

protected:
  std::set<RVA_t> collectModuleEntrance();
  

private:
  BinaryModule& bin_;
  std::vector<std::shared_ptr<Function>> functions_;
  std::map<RVA_t, std::shared_ptr<BasicBlock>> blocks_;
};