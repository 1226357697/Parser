#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

#include "PEBinaryModule.h"
#include "Parser.h"

int main()
{
  std::string parsePath = "D:\\Work\\cpp\\Parser\\bin\\winmine.exe";
  if (!fs::exists(parsePath))
  {
    std::cout <<"not found " << parsePath << std::endl;
    return 1;
  }

  PEBinaryModule bin;
  if (!bin.load(parsePath))
  {
    std::cout << "failed to load " << parsePath << std::endl;
    return 1;
  }

  Parser parser(bin);
  parser.analyze();
  //if (!)
  {
    std::cout << "failed to parse functions " << std::endl;
    return 1;
  }

  //parser.exportFuncrionsToDot("./cfg_dir");

  return 0;
}
