#pragma once

#include "BinaryModule.h"

class PEBinaryModule : public BinaryModule
{

public:

  virtual bool load(const std::string& path) override;
private:
};