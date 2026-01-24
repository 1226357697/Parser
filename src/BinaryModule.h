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

struct SectionInfo {
  RVA_t start;        // 起始地址
  RVA_t virtualEnd;   // 内存映射结束 (start + VirtualSize)
  RVA_t rawEnd;       // 实际数据结束 (start + SizeOfRawData)
  bool executable;    // 是否可执行
  bool readable;      // 是否可读
  bool writable;      // 是否可写
};  

struct ExceptionEntry {
  RVA_t funcStart;
  RVA_t funcEnd;
  RVA_t handlerRva;      // 异常处理函数
  RVA_t unwindInfoRva;   // x64 unwind info
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

  virtual std::vector<ExceptionEntry> getExceptionEntries() = 0;

  virtual std::optional<Instruction> disassembleOne(uint64_t addr, size_t* outBytesConsumed = nullptr);

  virtual InstructionAnalyzer* instructionAnalyzer()const;

  virtual std::span<const uint8_t> readBytes(RVA_t rva, size_t size);

  virtual std::optional<Addr_t> readPointer(RVA_t rva);

  virtual RVA_t VA2RVA(uint64_t address);

  LIEF::Section* getSectionByRva(uint64_t rva) const;

  const SectionInfo* findSection(RVA_t rva) const;

  virtual void cacheSections() = 0;
  
  bool hasFileData(RVA_t rva);

  // 地址是否在任意段中（有映射）
  bool isValidAddress(RVA_t rva) const;

  // 地址是否在代码段中（可执行）
  bool isCodeAddress(RVA_t rva) const;

  bool isReadableAddress(RVA_t rva) const;

  virtual uint32_t getPointerSize();

  virtual bool isCodeSection(LIEF::Section* section) const  = 0;

  const std::vector<SectionInfo>&  getSections()const;
protected:

  virtual bool doLoad(const std::string& path) = 0;

protected:
  std::unique_ptr<Architecture> arch_;
  std::unique_ptr<Disassembler> disasm_;
  std::unique_ptr<InstructionAnalyzer> analyzer_;


  std::vector<SectionInfo> sectionCache_;  // 缓存，按 start 排序
  std::shared_ptr<LIEF::Binary> binary_;
  
  std::vector<std::shared_ptr<BasicBlock>> basicBlocks_;
  std::vector<std::shared_ptr<BasicBlock>> Functions_;

  std::map<RVA_t, std::set<RVA_t>> xrefsTo_;
  std::map<RVA_t, std::set<RVA_t>> xrefsFrom_;
};
