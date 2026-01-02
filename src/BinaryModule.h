#pragma once

#include <string>
#include <map>

#include <LIEF/LIEF.hpp>
#include "BinaryTypes.h"
#include "Function.h"
#include "CrossReference.h"

enum class FunctionType : uint32_t
{
  NONE = 0,
  Export,
  Import
};

struct FuntionInfo
{
  std::string name;
  RVA_t rva;
  FunctionType type;
};

class BinaryModule
{
public:
  BinaryModule();
  virtual ~BinaryModule();

  virtual bool load(const std::string& path);

  virtual RVA_t entryPoint();

  virtual std::vector<FuntionInfo> exportFunctions();

  

protected:
  std::shared_ptr<LIEF::Binary> binary_;
  
  std::vector<std::shared_ptr<BasicBlock>> basicBlocks_;
  std::vector<std::shared_ptr<BasicBlock>> Functions_;

  std::map<RVA_t, std::set<RVA_t>> xrefsTo_;
  std::map<RVA_t, std::set<RVA_t>> xrefsFrom_;
};
