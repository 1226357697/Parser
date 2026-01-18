#include "Parser.h"
#include <unordered_map>
#include <stack>
#include <format>
#include <iostream>
#include <filesystem>
#include"JumpTableAnalyzer.h"

namespace fs = std::filesystem;

Parser::Parser(BinaryModule& bin)
:bin_(bin)
{
}

bool Parser::parseFunctions()
{
  // collect entrances
  std::set<Entrance> entrances = collectModuleEntrance();

  parseBlockAndFunction(entrances);
  mergeParseBlockAndFunction();

  rebuildFunctionsByPredict();



  return true;
}

static std::string hex(uint64_t addr) {
  std::ostringstream ss;
  ss << "0x" << std::hex << addr;
  return ss.str();
}

static std::string escape(const std::string& s) {
  std::string out;
  for (char c : s) {
    if (c == '"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else out += c;
  }
  return out;
}

bool Parser::exportFunctionToDot(RVA_t rva, const std::string& fileName)
{
  std::ostringstream ss;
  auto iter = functions_.find(rva);
  if (iter == functions_.end())
  {
    return false;
  }

  std::shared_ptr<Function> func = iter->second;
  auto analyzer = bin_.instructionAnalyzer();

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
      std::shared_ptr<Instruction>backInst =  block->instructions().back();
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

    if(!successor.empty())
      ss << ";\n";
  }


  ss << "}\n";

  std::ofstream f(fileName);
  if(!f.is_open())
    return false;

  f << ss.str();
  return true;
}

void Parser::exportFuncrionsToDot(const std::string& dirName)
{
  if (!fs::exists(dirName))
  {
    fs::create_directories(dirName);
  }


  for (auto& [rva, func] : functions_)
  {
    fs::path functionDotFile = fs::path(dirName) ;
    functionDotFile = functionDotFile  / (hex(rva) + "_cfg.dot");
    exportFunctionToDot(rva, functionDotFile.string());
  }
}

bool Parser::buildFunctions()
{
  for (auto& [rva, block] : parseingBlocks_)
  {
    if (parseingFunctions_.find(rva) == parseingFunctions_.end() &&   block->tag() == BasicBlock::Tag::kFunctionEntry)
    {
      std::shared_ptr<Function> function = std::make_shared<Function>(bin_, rva, block);
      buildFunctionCFG(function);

      parseingFunctions_.emplace(rva, function);
    }
  }

  return true;
}


void Parser::buildFunctionCFG(std::shared_ptr<Function> function)
{
  std::set<RVA_t> visited;
  std::stack<RVA_t> workLists;

  workLists.push(function->rva());

  auto analyzer = bin_.instructionAnalyzer();

  while (!workLists.empty())
  {
    RVA_t blockRva = workLists.top();
    workLists.pop();

    if (visited.count(blockRva) > 0)
      continue;

    visited.insert(blockRva);

  
    std::shared_ptr<BasicBlock>block = getBlock(blockRva);
    if (!block)
    {
      assert(false);
      continue;
    }

    function->addBlock(block);

    BasicBlock::EndType endType = block->endType();
    auto& insts = block->instructions();
    if (endType == BasicBlock::EndType::kConditionalJump)
    {
      auto inst = insts.back();
      assert(analyzer->isConditionalJump(*inst));

      auto target = analyzer->getJumpTarget(*inst);
      auto fall = analyzer->getNextAddress(*inst);

      if (target)
      {
        std::shared_ptr<BasicBlock>  successor = getBlock(target.value());
        assert(successor);

        block->addSuccessor(successor);
        successor->addPredecessor(block);

        workLists.push(successor->startAddress());
      }

      {
        std::shared_ptr<BasicBlock>  successor = getBlock(fall);
        assert(successor);

        block->addSuccessor(successor);
        successor->addPredecessor(block);

        workLists.push(successor->startAddress());
      }
    }
    else if (endType == BasicBlock::EndType::kUnconditionalJump)
    {
      auto inst = insts.back();
      assert(analyzer->isUnconditionalJump(*inst));

      auto target = analyzer->getJumpTarget(*inst);

      if (target)
      {
        std::shared_ptr<BasicBlock>  successor = getBlock(target.value());
        assert(successor);

        block->addSuccessor(successor);
        successor->addPredecessor(block);

        workLists.push(successor->startAddress());
      }
    }
    else if (endType == BasicBlock::EndType::kFallThrough)
    {
      auto inst = insts.back();
      auto fall = analyzer->getNextAddress(*inst);

      {
        std::shared_ptr<BasicBlock>  successor = getBlock(fall);
        assert(successor);

        block->addSuccessor(successor);
        successor->addPredecessor(block);


        workLists.push(successor->startAddress());
      }

    }
    else if (endType == BasicBlock::EndType::kIndirectJump)
    {
      // TODO: 跳转表分析后添加后继
    }
    // passed
    else if (endType == BasicBlock::EndType::kReturn)
    {

    }
    else
    {
      assert(false);
    }

  }



}

void Parser::insertIndirectCFG(std::shared_ptr<BasicBlock> mainBb, std::shared_ptr<BasicBlock> bb)
{
  if (mainBb->endType() != BasicBlock::EndType::kIndirectJump)
  {
    assert(false);
    return ;
  }

  mainBb->addSuccessor(bb);
  bb->addPredecessor(mainBb);
}

void Parser::exploreBuildBlock(const std::set<Entrance>& entrance)
{
  std::set<Entrance> exploreCall;
  std::set<Entrance> exploreWork;

  exploreWork.insert(entrance.begin(), entrance.end());

  do
  {
    for (const auto& iter : exploreWork)
    {
      std::set<Entrance> calls;
      exploreBlocks(iter, calls);
      exploreCall.insert(calls.begin(), calls.end());
    }

    exploreWork = std::move(exploreCall);
    exploreCall.clear();

  } while (!exploreWork.empty());

}

void Parser::exploreBlocks(const Entrance& entrance, std::set<Entrance>& exploreCall)
{
  if (blocks_.find(entrance.rva) != blocks_.end()
   || parseingBlocks_.find(entrance.rva) != parseingBlocks_.end())
  {
    // explored
    return;
  }

  std::set<RVA_t> leaders;
  std::set<RVA_t> visited;
  std::stack<RVA_t> workLists;

  leaders.insert(entrance.rva);
  workLists.push(entrance.rva);

  auto analyzer = bin_.instructionAnalyzer();

  while (!workLists.empty())
  {
    RVA_t rva = workLists.top();
    workLists.pop();

    if (visited.count(rva))
      continue;
    visited.insert(rva);

    bool isNewBlock = false;
    std::shared_ptr<BasicBlock> block = getOrCreateBlock(rva, &isNewBlock);
    
    if (isNewBlock)
    {
      block->setTag(BasicBlock::Tag::kNormal);

      if (entrance.type == EntranceType::kFunction && rva == entrance.rva)
      {
        block->setTag(BasicBlock::Tag::kFunctionEntry);
      }
    }


    RVA_t currentRva = rva;
    while (auto inst = bin_.disassembleOne(currentRva))
    {
      auto inst_ptr = std::make_shared<Instruction>(std::move(*inst));
      block->addInstruction(inst_ptr);

      //std::cout
      //  << std::format("{:08X} {}\t{}", inst_ptr->address + bin_.imageBase(), inst_ptr->mnemonic, inst_ptr->operands)
      //  << std::endl;


      if (analyzer->isConditionalJump(*inst_ptr))
      {
        block->setEndType(BasicBlock::EndType::kConditionalJump);
        auto target = analyzer->getJumpTarget(*inst_ptr);
        auto fall = analyzer->getNextAddress(*inst_ptr);
        if (target && leaders.insert(target.value()).second)
        {
          // add xref
          addXref({.from = currentRva, .to = (RVA_t)target.value(), .type = XrefType::kJmp});

          workLists.push(target.value());
        }

        if (leaders.insert(fall).second)
        {
          workLists.push(fall);
        }

        break;
      }

      if (analyzer->isUnconditionalJump(*inst_ptr))
      {
        if (analyzer->isIndirectJump(*inst_ptr))
        {
          std::cout
          << std::format("IndirectJump {:08X} {}\t{}", inst_ptr->address + bin_.imageBase(), inst_ptr->mnemonic, inst_ptr->operandsStr)
          << std::endl;
          block->setEndType(BasicBlock::EndType::kIndirectJump);
        }
        else
        {
          block->setEndType(BasicBlock::EndType::kUnconditionalJump);
        }

        auto target = analyzer->getJumpTarget(*inst_ptr);

        if (target && leaders.insert(target.value()).second)
        {
          // add xref
          addXref({ .from = currentRva, .to = (RVA_t)target.value(), .type = XrefType::kJmp });

          workLists.push(target.value());
        }

        break;
      }

      if(analyzer->isCall(*inst_ptr))
      {
        auto target = analyzer->getCallTarget(*inst_ptr);
        if (target)
        {
          // add xref
          
          addXref({ .from = currentRva, .to = (RVA_t)target.value() , .type = XrefType::kCall});

          exploreCall.insert(Entrance{ EntranceType ::kFunction, (RVA_t)target.value()});
        }
      }

      if (analyzer->isReturn(*inst_ptr))
      {
        if (block->tag() == BasicBlock::Tag::kNormal)
        {
          block->setTag(BasicBlock::Tag::kFinally);
        }

        block->setEndType(BasicBlock::EndType::kReturn);


        break;
      }
      currentRva += inst_ptr->size();

      // 是否到到已有Block
      if (leaders.count(currentRva))
      {
        block->setEndType(BasicBlock::EndType::kFallThrough);
        break;
      }

      // 检查是否进入了已有块的中间（需要分割）
      if (auto existing = findBlockContaining(currentRva)) {
        auto newBlock = existing->splitAt(currentRva);
        parseingBlocks_[currentRva] = newBlock;
        leaders.insert(currentRva);
        block->setEndType(BasicBlock::EndType::kFallThrough);
        break;
      }
    }

    //std::cout << "---------------------------------------------------" << std::endl;
  }

}



std::set<Entrance> Parser::collectModuleEntrance()
{
  std::set<Entrance> entrance;

  if (bin_.entryPoint() != 0)
  {
    entrance.insert({ EntranceType::kFunction, bin_.entryPoint()});
  }

  for (auto& item : bin_.exportFunctions())
  {
    entrance.insert({ EntranceType::kFunction, item.rva});
  }

  return entrance;
}

void Parser::parseBlockAndFunction(const std::set<Entrance>& entrance)
{
  // explore blocks
  exploreBuildBlock(entrance);

  buildFunctions();

}

void Parser::rebuildFunctionsByPredict()
{
  for(auto& [rva, func] : functions_)
  {
    rebuildFunctionByPredict(*func);
  }

}

void Parser::rebuildFunctionByPredict(Function& func)
{
  JumpTableAnalyzer jtAnalyzer(bin_);
  std::stack<RVA_t> toReparseBlocks;
  for (auto& [rva, bb] : func.blocks())
  {
    toReparseBlocks.push(rva);
  }

  while (!toReparseBlocks.empty())
  {
    RVA_t rva = toReparseBlocks.top();
    toReparseBlocks.pop();

    auto bb =  blocks_.at(rva);

    auto jtInfo = jtAnalyzer.analyze(*bb);

    if (jtInfo && !jtInfo->cases.empty())
    {
      std::set<Entrance> entrance;
      for (Addr_t va : jtInfo->cases)
      {
        entrance.insert({ EntranceType::kBlock, bin_.VA2RVA(va) });
        toReparseBlocks.push(bin_.VA2RVA(va));
      }

      parseBlockAndFunction(entrance);

      // 更新间接跳转的CFG
      bb->addFlags(BasicBlock::Flags::kJumpTable);
      for (auto& [rva, casebb] : parseingBlocks_)
      {
        insertIndirectCFG(bb, casebb);
      }

      mergeParseBlockAndFunction();
    }

    bb->setFlags(BasicBlock::Flags::kParseFinish);
  }

}

void Parser::mergeParseBlockAndFunction()
{

  blocks_.merge(parseingBlocks_);
  functions_.merge(parseingFunctions_);
}




std::shared_ptr<BasicBlock> Parser::getBlock(RVA_t addr)
{
  if (auto iter = blocks_.find(addr); iter != blocks_.end())
    return iter->second;

  if (auto iter = parseingBlocks_.find(addr); iter != parseingBlocks_.end())
    return iter->second;

  return nullptr;
}

std::shared_ptr<BasicBlock> Parser::getOrCreateBlock(RVA_t addr, bool* isNew)
{
  if(isNew) *isNew = false;

  // 已存在
  if (auto existing = getBlock(addr)) {
    return existing;
  }

  if (isNew) *isNew = true;

  // 检查是否在已有块中间
  if (auto existing = findBlockContaining(addr)) {
    auto newBlock = existing->splitAt(addr);
    parseingBlocks_[addr] = newBlock;
    return newBlock;
  }

  // 创建新块
  auto block = std::make_shared<BasicBlock>(addr);
  parseingBlocks_[addr] = block;
  return block;
}

std::shared_ptr<BasicBlock> Parser::findBlockContaining(RVA_t rva)
{
  auto iter = blocks_.upper_bound(rva);
  if (iter == blocks_.begin())
  {
    iter = parseingBlocks_.upper_bound(rva);
    if (iter == parseingBlocks_.begin())
      return nullptr;
  }

  --iter;
  if (rva >= iter->second->startAddress() && rva < iter->second->endAddress())
    return iter->second;

  return nullptr;
}

uint32_t Parser::GetGasSize(RVA_t rva)
{
  uint32_t gasSize = 0;
  auto analyzer = bin_.instructionAnalyzer();

  RVA_t currentRva = rva;
  while (auto inst = bin_.disassembleOne(currentRva))
  {
    if (analyzer->isNop(*inst))
    {
      gasSize += inst->size();
    }
    else
    {
      break;
    }

    currentRva += inst->size();
  }
  return gasSize;
}

