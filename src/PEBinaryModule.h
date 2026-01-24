#pragma once

#include "BinaryModule.h"

class PEBinaryModule : public BinaryModule
{

public:

protected:
  virtual bool doLoad(const std::string& path) override;

  virtual std::vector<ExceptionEntry> getExceptionEntries() override;

  virtual bool isCodeSection(LIEF::Section* section) const override;

  virtual void cacheSections() override;

private:
  inline LIEF::PE::Binary& bin() const { return *(LIEF::PE::Binary*)binary_.get();}

};