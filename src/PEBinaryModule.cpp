#include "PEBinaryModule.h"
#include <iostream>
using namespace LIEF::PE;

bool PEBinaryModule::doLoad(const std::string& path)
{
  binary_ = LIEF::PE::Parser::parse(path);
  if(!binary_)
    return false;
  

  return true;
}

std::vector<ExceptionEntry> PEBinaryModule::getExceptionEntries()
{
  return std::vector<ExceptionEntry>();
}

bool PEBinaryModule::isCodeSection(LIEF::Section* section) const
{
  auto* peSection = dynamic_cast<const LIEF::PE::Section*>(section);
  if (!peSection) return false;

  constexpr uint32_t CODE_FLAGS =
    static_cast<uint32_t>(LIEF::PE::Section::CHARACTERISTICS::MEM_EXECUTE) |
    static_cast<uint32_t>(LIEF::PE::Section::CHARACTERISTICS::CNT_CODE);

  return (peSection->characteristics() & CODE_FLAGS) != 0;
}

void PEBinaryModule::cacheSections()
{
  sectionCache_.clear();

  for (const auto& section : bin().sections()) {
    SectionInfo info;
    info.start = static_cast<RVA_t>(section.virtual_address());
    info.virtualEnd = info.start + static_cast<RVA_t>(section.virtual_size());
    info.rawEnd = info.start + static_cast<RVA_t>(section.sizeof_raw_data());

    // PE 特定的属性检查
    info.executable = false;
    info.readable = false;
    info.writable = false;

    if (auto* pe = dynamic_cast<const LIEF::PE::Section*>(&section)) {
      uint32_t chars = pe->characteristics();
      info.executable = (chars & static_cast<uint32_t>(LIEF::PE::Section::CHARACTERISTICS::MEM_EXECUTE)) != 0;
      info.readable = (chars & static_cast<uint32_t>(LIEF::PE::Section::CHARACTERISTICS::MEM_READ)) != 0;
      info.writable = (chars & static_cast<uint32_t>(LIEF::PE::Section::CHARACTERISTICS::MEM_WRITE)) != 0;
    }

    sectionCache_.push_back(info);
  }

  // 按起始地址排序，便于二分查找
  std::sort(sectionCache_.begin(), sectionCache_.end(),
    [](const SectionInfo& a, const SectionInfo& b) {
      return a.start < b.start;
    });
}

bool BinaryModule::hasFileData(RVA_t rva)
{
  const SectionInfo* info = findSection(rva);
  return info && rva < info->rawEnd;
}

