#pragma once

#include <string>
#include <map>
#include <span>

#include <LIEF/LIEF.hpp>
#include "BinaryTypes.h"
#include "CrossReference.h"
#include "Architecture.h"
#include "InstructionAnalyzer.h"


class BasicBlock;

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

  uint64_t imageBase();

  virtual RVA_t entryPoint();

  virtual std::vector<FuntionInfo> exportFunctions();
  
  virtual void addBasicBlock(std::shared_ptr<BasicBlock> bb);
 

  virtual std::optional<Instruction> disassembleOne(uint64_t addr, size_t* outBytesConsumed = nullptr);

  virtual InstructionAnalyzer* instructionAnalyzer()const;

  virtual std::span<const uint8_t> readBytes(RVA_t rva, size_t size);

  virtual std::optional<Addr_t> readPointer(RVA_t rva);

  virtual RVA_t VA2RVA(uint64_t address);

  LIEF::Section* getSectionByRva(uint64_t rva) const;

  // 地址是否在任意段中（有映射）
  bool isValidAddress(RVA_t rva) const;

  // 地址是否在代码段中（可执行）
  bool isCodeAddress(RVA_t rva) const;

  // 地址是否在可读数据段中（用于跳转表）
  bool isReadableAddress(RVA_t rva) const;

  bool isValidCodeAddress(RVA_t rva) const;

  virtual uint32_t getPointerSize();

  bool isCodeSection(LIEF::Section* section) const ;

  LIEF::Binary::it_sections getSections()const;
protected:

  virtual bool doLoad(const std::string& path) = 0;

protected:
  std::unique_ptr<Architecture> arch_;
  std::unique_ptr<Disassembler> disasm_;
  std::unique_ptr<InstructionAnalyzer> analyzer_;

  std::shared_ptr<LIEF::Binary> binary_;
  
  std::vector<std::shared_ptr<BasicBlock>> basicBlocks_;
  std::vector<std::shared_ptr<BasicBlock>> Functions_;

  std::map<RVA_t, std::set<RVA_t>> xrefsTo_;
  std::map<RVA_t, std::set<RVA_t>> xrefsFrom_;
};
