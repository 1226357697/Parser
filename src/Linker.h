#pragma once

#include "BinaryModule.h"

#include <string>

class Linker
{
public:
  Linker(BinaryModule& bin);

  bool save(const std::string& outPath);

private:
  BinaryModule& bin_;
};