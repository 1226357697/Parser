#pragma once

#include "BinaryModule.h"

class PEBinaryModule : public BinaryModule
{

public:

protected:
  virtual bool doLoad(const std::string& path) override;

private:

};