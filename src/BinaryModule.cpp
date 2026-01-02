#include "BinaryModule.h"



BinaryModule::BinaryModule()
{
}

BinaryModule::~BinaryModule()
{
}

bool BinaryModule::load(const std::string& path)
{
  return true;
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
