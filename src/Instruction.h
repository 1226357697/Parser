#pragma once

#include <vector>
#include <string>
#include <span>
#include "BinaryTypes.h"

struct  Instruction
{
	inline size_t size()const { return bytes.size(); }

	Addr_t address;
	std::vector<uint8_t> bytes;
	std::string mnemonic;
	std::string operands;
};
