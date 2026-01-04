#pragma once

#include <string>
#include <map>

#include <LIEF/LIEF.hpp>
#include "BinaryTypes.h"
#include "Function.h"
#include "CrossReference.h"
#include "Architecture.h"
#include "InstructionAnalyzer.h"

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

  bool load(const std::string& path);

  virtual RVA_t entryPoint();

  virtual std::vector<FuntionInfo> exportFunctions();
  
  virtual void addBasicBlock(std::shared_ptr<BasicBlock> bb);
 

  virtual std::optional<Instruction> disassembleOne(uint64_t addr, size_t* outBytesConsumed = nullptr);

  virtual std::unique_ptr<InstructionAnalyzer> instructionAnalyzer();

  virtual std::vector<uint8_t> readBytes(RVA_t rva, size_t size);

protected:

  virtual bool doLoad(const std::string& path) = 0;

protected:
  std::unique_ptr<Architecture> arch_;
  std::unique_ptr<Disassembler> disasm_;

  std::shared_ptr<LIEF::Binary> binary_;
  
  std::vector<std::shared_ptr<BasicBlock>> basicBlocks_;
  std::vector<std::shared_ptr<BasicBlock>> Functions_;

  std::map<RVA_t, std::set<RVA_t>> xrefsTo_;
  std::map<RVA_t, std::set<RVA_t>> xrefsFrom_;
};
