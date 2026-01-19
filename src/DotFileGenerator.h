#pragma
#include <string>

#include "Parser.h"
class DotFileGenerator
{
public:
  DotFileGenerator(Parser& parser);

  bool exportFunctionToDot(RVA_t rva, const std::string& fileName);

  void exportFuncrionsToDot(const std::string& dirName);

protected:
  
  std::string hex(uint64_t addr);

  std::string escape(const std::string& s);

private:
  Parser& parser_;
};