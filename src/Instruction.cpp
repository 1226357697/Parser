#include "Instruction.h"



Instruction::Instruction(Addr_t address, std::span<uint8_t> bytes, const std::string& mnemonic, const std::string operands)
:address_(address), bytes_(bytes.begin(), bytes.end()),mnemonic_(mnemonic), operands_(operands)
{
}
Instruction::~Instruction()
{
}
