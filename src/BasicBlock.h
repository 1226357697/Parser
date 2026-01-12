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
		kInvalid = 0,
		kIndirectJump,
		kConditionalJump,
		kUnconditionalJump,
		kReturn,
		kCall,
		kIndirectCall,
		kFallThrough,
		kSyscall,
		kITBlock,
		kTableBranch
	};

	enum class Tag
	{
		kNone = 0,
		kNormal,
		kFunctionEntry,
		kFinally,
	};

public:
	BasicBlock(RVA_t rva);

	~BasicBlock();

	void addInstruction(std::shared_ptr<Instruction> inst);
	void addSuccessor(std::shared_ptr<BasicBlock> bb);
	void addPredecessor(std::shared_ptr<BasicBlock> bb);
	inline void setEndType(EndType endType) { endType_ = endType; }
	inline void setTag(Tag tag) { tag_  = tag;}

	inline RVA_t startAddress() const { return startAddress_;};
	RVA_t endAddress() const { return endAddress_;}
	size_t getSize();
	inline EndType endType() { return endType_; };
	inline Tag tag()const { return tag_;}
	inline const std::vector<std::shared_ptr<Instruction>>& instructions(){ return instructions_;}

	std::vector<std::shared_ptr<BasicBlock>> getSuccessors() const;
	const std::vector<std::shared_ptr<BasicBlock>>& getPredecessors() const;

	std::shared_ptr<BasicBlock> splitAt(RVA_t rva);
private:
	RVA_t startAddress_;
	RVA_t endAddress_;
	Tag tag_;
	EndType endType_;
	std::vector<std::shared_ptr<Instruction>> instructions_;

	std::vector<std::shared_ptr<BasicBlock>> predecessors_; // Ç°Çý
	std::vector< std::weak_ptr<BasicBlock>> successors_; //ºó¼Ì
	
};
