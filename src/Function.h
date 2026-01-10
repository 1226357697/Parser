#pragma once

#include "BasicBlock.h"
#include "BinaryModule.h"

class Function
{
public:
	Function(BinaryModule& bin, RVA_t rva);
	~Function();

	bool parse();

	inline RVA_t rva() { return rva_; }
	std::shared_ptr<BasicBlock> entryBlock() const { return entryBlock_; }
	const std::map<RVA_t, std::shared_ptr<BasicBlock>>& blocks() const { return blocks_; }

protected:
	std::shared_ptr<BasicBlock> getOrCreateBlock(RVA_t addr);
	std::shared_ptr<BasicBlock> findBlockContaining(RVA_t rva);
	void buildCFGEdges();

private:
	BinaryModule& bin_;
	RVA_t rva_;
	std::string name_;
	
	std::shared_ptr<BasicBlock> entryBlock_;
	std::map<RVA_t, std::shared_ptr<BasicBlock>> blocks_;

};
