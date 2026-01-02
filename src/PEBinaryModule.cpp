#include "PEBinaryModule.h"

using namespace LIEF::PE;

bool PEBinaryModule::load(const std::string& path)
{
  binary_ = LIEF::PE::Parser::parse(path);
  if(!binary_)
    return false;

  return true;
}
