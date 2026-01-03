#pragma once

#include <vector>
#include <memory>

#include "Instruction.h"
#include "BinaryTypes.h"

class BasicBlock
{
public:
	enum class EndTYpe
	{
		Invalid = 0,
		Jump,
		ConditionalJump,
		Return,
		Call,
		FallThrough
	};

public:
	BasicBlock(RVA_t rva);

	~BasicBlock();

	void addInstruction(std::shared_ptr<Instruction> inst);
	size_t getSize();

	EndTYpe endType();
private:
	RVA_t startAddress_;
	RVA_t endAddress_;
	std::vector<BasicBlock*> predecessors_; // Ç°Çý
	std::vector<BasicBlock*> successors_; //ºó¼Ì
};
