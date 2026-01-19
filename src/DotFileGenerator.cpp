#include "DotFileGenerator.h"

#include "BinaryModule.h"
#include <sstream>
#include <fstream>
#include <filesystem>


namespace fs = std::filesystem;

std::string DotFileGenerator::hex(uint64_t addr) {
  std::ostringstream ss;
  ss << "0x" << std::hex << addr;
  return ss.str();
}

std::string DotFileGenerator::escape(const std::string& s) {
  std::string out;
  for (char c : s) {
    if (c == '"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else out += c;
  }
  return out;
}


DotFileGenerator::DotFileGenerator(Parser& parser)
:parser_(parser)
{
}

bool DotFileGenerator::exportFunctionToDot(RVA_t rva, const std::string& fileName)
{
  auto functions =  parser_.function();
  std::ostringstream ss;
  auto iter = functions.find(rva);
  if (iter == functions.end())
  {
    return false;
  }

  std::shared_ptr<Function> func = iter->second;
  auto analyzer = parser_.binarayModule().instructionAnalyzer();

  ss << "digraph CFG {\n";
  ss << "  node [shape=box, fontname=\"Courier\", fontsize=10];\n";
  ss << "  edge [fontsize=8];\n\n";

  for (auto& [rva, block] : func->blocks())
  {
    // 绘制节点
    ss << "  \"" << hex(block->startAddress()) << "\" [label=\"";

    // 块内指令
    for (const auto& insn : block->instructions()) {
      std::string instString = insn->mnemonic + "\t" + insn->operandsStr;
      ss << hex(insn->address) << ": " << escape(instString) << "\\l";
    }

    ss << "\"];\n";
  }


  ss << "\n";

  // 绘制边
  for (auto& [rva, block] : func->blocks())
  {
    std::vector<std::shared_ptr<BasicBlock>> successor = block->getSuccessors();
    BasicBlock::EndType endType = block->endType();

    for (auto successor : successor)
    {
      ss << "  \"" << hex(block->startAddress()) << "\" -> \"" << hex(successor->startAddress()) << "\"";
      std::shared_ptr<Instruction>backInst = block->instructions().back();
      switch (endType) {
      case BasicBlock::EndType::kConditionalJump:
      {
        auto target = analyzer->getJumpTarget(*backInst);
        auto fall = analyzer->getNextAddress(*backInst);
        if (target && target.value() == successor->startAddress())
        {
          ss << " [color=green, label=\"T\"]";
        }
        if (fall == successor->startAddress())
        {
          ss << " [color=red, label=\"F\"]";
        }

        break;
      }
      case BasicBlock::EndType::kUnconditionalJump:
      {
        auto target = analyzer->getJumpTarget(*backInst);
        if (target && target.value() == successor->startAddress())
        {
          ss << " [color=blue]";
        }
        break;
      }

      default:
        break;
      }
    }

    if (!successor.empty())
      ss << ";\n";
  }


  ss << "}\n";

  std::ofstream f(fileName);
  if (!f.is_open())
    return false;

  f << ss.str();
  return true;
}

void DotFileGenerator::exportFuncrionsToDot(const std::string& dirName)
{
  if (!fs::exists(dirName))
  {
    fs::create_directories(dirName);
  }


  for (auto& [rva, func] : parser_.function())
  {
    fs::path functionDotFile = fs::path(dirName);
    functionDotFile = functionDotFile / (hex(rva) + "_cfg.dot");
    exportFunctionToDot(rva, functionDotFile.string());
  }
}
