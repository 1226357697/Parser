#include "BinaryModule.h"
#include "X86Architecture.h"
#include <filesystem>

namespace fs = std::filesystem;

BinaryModule::BinaryModule()
{
}

BinaryModule::~BinaryModule()
{
}

bool BinaryModule::load(const std::string& path)
{
  if(!fs::exists(path))
    return false;

  if(!doLoad(path))
    return false;

  auto arch = binary_->header().architecture();
  if (arch == LIEF::Header::ARCHITECTURES::X86)
  {
    arch_ = std::make_unique<X86Architecture>();
  }
  assert(arch_);
  if(!arch_)
    return false;

  disasm_ = arch_->createDisassembler();
  assert(disasm_);

  if(!arch_)
    return false;


  analyzer_ = arch_->createInstructionAnalzer();

  cacheSections();

  return true;
}

uint64_t BinaryModule::imageBase()
{
    return binary_->imagebase();
}

RVA_t BinaryModule::entryPoint()
{
  return binary_->entrypoint() - binary_->imagebase();
}

std::vector<FuntionInfo> BinaryModule::exportFunctions()
{
  std::vector<FuntionInfo> exportFunction;

  for (auto& item : binary_->exported_functions())
  {
    FuntionInfo info;
    info.name = item.name();
    info.type = FunctionType::Export;
    info.rva = item.address();
    exportFunction.push_back(std::move(info));
  }
  return exportFunction;
}

std::optional<Instruction> BinaryModule::disassembleOne(uint64_t addr, size_t* outBytesConsumed)
{
  std::span<const uint8_t>buffer = readBytes(addr, arch_->maxInstructionSize());

  return disasm_->disassembleOne(buffer, addr, outBytesConsumed);

}

InstructionAnalyzer* BinaryModule::instructionAnalyzer() const
{
    return analyzer_.get();
}

std::span<const uint8_t>  BinaryModule::readBytes(RVA_t rva, size_t size)
{
  uint64_t va = binary_->imagebase() + rva;
  auto bytyes =  binary_->get_content_from_virtual_address(va, size);
  return bytyes;
}

std::optional<Addr_t> BinaryModule::readPointer(RVA_t rva)
{
  auto bytes = readBytes(rva, getPointerSize());
  if(bytes.empty())
    return std::nullopt;

  Addr_t v =0;
  if(getPointerSize() == 4)
    v = *(uint32_t*)bytes.data();
  else
    v = *(uint64_t*)bytes.data();
    
  return v;
}

RVA_t BinaryModule::VA2RVA(uint64_t address)
{
    return (RVA_t)(address - binary_->imagebase());
}

LIEF::Section* BinaryModule::getSectionByRva(uint64_t rva) const {
  for (LIEF::Section& section : binary_->sections()) {
    uint64_t start = section.virtual_address();
    uint64_t end = start + section.size();

    if (rva >= start && rva < end) {
      return &section;
    }
  }
  return nullptr;
}

const SectionInfo* BinaryModule::findSection(RVA_t rva) const
{
  if (sectionCache_.empty()) return nullptr;

  // upper_bound 找到第一个 start > rva 的
  auto it = std::upper_bound(
    sectionCache_.begin(),
    sectionCache_.end(),
    rva,
    [](RVA_t addr, const SectionInfo& s) { return addr < s.start; }
  );

  // 回退一个，检查是否包含 rva
  if (it != sectionCache_.begin()) {
    --it;
    if (rva >= it->start && rva < it->virtualEnd) {
      return &(*it);
    }
  }
  return nullptr;
}

bool BinaryModule::isValidAddress(RVA_t rva) const
{
  return findSection(rva) != nullptr;
}

bool BinaryModule::isCodeAddress(RVA_t rva) const
{
  const SectionInfo* info = findSection(rva);
  return info && info->executable && rva < info->rawEnd;
}

bool BinaryModule::isReadableAddress(RVA_t rva) const
{
  const SectionInfo* info = findSection(rva);
  return info && info->readable && rva < info->rawEnd;
}

uint32_t BinaryModule::getPointerSize()
{
  return binary_->header().is_32() ? 4 : 8;
}

const std::vector<SectionInfo>& BinaryModule::getSections() const
{
    return sectionCache_;
}
