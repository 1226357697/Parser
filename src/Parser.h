#pragma once

#include "BinaryModule.h"
#include <vector>
#include <stack>
#include "Function.h"

class Parser
{
public:
  Parser(BinaryModule& bin);
  bool parseFunctions();

protected:
  bool parseFunction(RVA_t rva);
  std::set<uint32_t> searchLeaders();

private:
  BinaryModule& bin_;
  std::stack<uint32_t> workList_;
};