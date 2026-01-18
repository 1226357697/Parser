#pragma once

#include "BasicBlock.h"
#include "BinaryModule.h"

#include <map>

class BasicBlock;

class Function
{
public:
	Function(BinaryModule& bin, RVA_t rva, std::shared_ptr<BasicBlock> block);
	~Function();

	inline RVA_t rva() { return rva_; }
	std::shared_ptr<BasicBlock> entryBlock() const { return entryBlock_; }
	void setEntryBlock(std::shared_ptr<BasicBlock> block) { entryBlock_ = block;}

	void addBlock(std::shared_ptr<BasicBlock> block);

	const std::map<RVA_t, std::shared_ptr<BasicBlock>> blocks() const { return blocks_; }
	size_t blockCount() const { return blocks_.size(); }
protected:


private:
	BinaryModule& bin_;
	RVA_t rva_;
	std::string name_;
	
	std::shared_ptr<BasicBlock> entryBlock_;
	std::map<RVA_t, std::shared_ptr<BasicBlock>> blocks_;
};
