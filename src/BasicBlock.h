#pragma once

#include <vector>
#include <memory>

#include "Instruction.h"
#include "BinaryTypes.h"

class BasicBlock
{
public:
	enum class EndType
	{
		Invalid = 0,
		IndirectJump,
		ConditionalJump,
		UnconditionalJump,
		Return,
		Call,
		IndirectCall,
		FallThrough,
		Syscall,
		ITBlock,
		TableBranch
	};

public:
	BasicBlock(RVA_t rva);

	~BasicBlock();

	void addInstruction(std::shared_ptr<Instruction> inst);
	void addSuccessor(std::shared_ptr<BasicBlock> bb);
	void addPredecessor(std::shared_ptr<BasicBlock> bb);
	inline void setEndType(EndType endType) { endType_  = endType;}

	inline RVA_t startAddress() const { return startAddress_;};
	RVA_t endAddress() const { return endAddress_;}
	size_t getSize();
	inline EndType endType() { return endType_; };

	std::shared_ptr<BasicBlock> splitAt(RVA_t rva);
private:
	RVA_t startAddress_;
	RVA_t endAddress_;
	EndType endType_;
	std::vector<std::shared_ptr<Instruction>> instructions_;

	std::vector<std::shared_ptr<BasicBlock>> predecessors_; // Ç°Çý
	std::vector< std::shared_ptr<BasicBlock>> successors_; //ºó¼Ì
	
};
