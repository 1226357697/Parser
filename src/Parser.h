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
  void collectEntrance();

private:
  BinaryModule& bin_;
  std::stack<uint32_t> entrances_;
};