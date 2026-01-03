#pragma once

#include <vector>
#include <string>
#include <span>
#include "BinaryTypes.h"

class Instruction
{
public:
	Instruction(
		Addr_t address, 
		std::span<uint8_t> bytes, 
		const std::string& mnemonic,
		const std::string operands);

	~Instruction();

	inline Addr_t address() { return address_; }
	inline const std::vector<uint8_t>& bytes() { return bytes_; }
	inline const std::string& mnemonic() { return mnemonic_; }
	inline const std::string& operands() { return operands_; }
	inline size_t size(){ return bytes_.size(); }

private:
	Addr_t address_;
	std::vector<uint8_t> bytes_;
	std::string mnemonic_;
	std::string operands_;
};
