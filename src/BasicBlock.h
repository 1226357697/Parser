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
		IndirectJump,
		ConditionalJump,
		UnconditionalJump,
		Return,
		Call,
		FallThrough
	};

public:
	BasicBlock(RVA_t rva);

	~BasicBlock();

	void addInstruction(std::shared_ptr<Instruction> inst);
	void addSuccessor(std::shared_ptr<BasicBlock> bb);
	void addPredecessor(std::shared_ptr<BasicBlock> bb);
	inline void setEndType(EndTYpe endType) { endType_  = endType;}

	inline RVA_t startAddress() const { return startAddress_;};
	RVA_t endAddress() const { return endAddress_;}
	size_t getSize();
	inline EndTYpe endType() { return endType_; };
private:
	RVA_t startAddress_;
	RVA_t endAddress_;
	EndTYpe endType_;
	std::vector<std::shared_ptr<Instruction>> instructions_;

	std::vector<std::shared_ptr<BasicBlock>> predecessors_; // Ç°Çý
	std::vector< std::shared_ptr<BasicBlock>> successors_; //ºó¼Ì
	
};
