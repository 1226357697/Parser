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

void BinaryModule::addBasicBlock(std::shared_ptr<BasicBlock> bb)
{
  basicBlocks_.push_back(bb);
}

std::optional<Instruction> BinaryModule::disassembleOne(uint64_t addr, size_t* outBytesConsumed)
{
  std::vector<uint8_t> buffer = readBytes(addr, arch_->maxInstructionSize());

  return disasm_->disassembleOne(buffer, addr, outBytesConsumed);

}

std::unique_ptr<InstructionAnalyzer> BinaryModule::instructionAnalyzer()
{
    return arch_->createInstructionAnalzer();
}

std::vector<uint8_t> BinaryModule::readBytes(RVA_t rva, size_t size)
{
  uint64_t va = binary_->imagebase() + rva;
  auto bytyes =  binary_->get_content_from_virtual_address(va, size);
  return std::vector<uint8_t>(bytyes.begin(), bytyes.end());
}
