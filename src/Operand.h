#pragma once

#include <cstdint>
#include <variant>
#include <vector>
#include <string>


// 寄存器 ID：高 8 位是架构，低 24 位是寄存器编号
// 这样可以区分不同架构的寄存器
struct RegId
{
  uint32_t id;

	enum class Arch : uint8_t
	{
		None = 0,
		X86 = 1,
		ARM = 2,
		ARM64 = 3,
	};

	inline Arch arch() const {
		return static_cast<Arch>( id >> 24);
	}

	uint32_t reg() const {
		return id & 0x00ffffff;
	}
	
	inline static RegId make(Arch arch, uint32_t reg) {
		return { (static_cast<uint32_t>(arch) << 24) | (reg & 0x00FFFFFF) };
	}

	bool operator==(const RegId& other) const { return id == other.id; }
	bool operator!=(const RegId& other) const { return id != other.id; }

	// 无效寄存器
	static RegId invalid() { return { 0 }; }
	bool isValid() const { return id != 0; }
};

enum class OperandType
{
	Invalid = 0,
	Reg,
	Imm,
	Mem,
	FPreg,
	VecReg,
};


enum class OperandAccess : uint8_t
{
	None = 0,
	Read = 1,
	Write = 2,
	ReadWrite = 3,
};

struct RegOperand
{
	RegId reg;
	uint8_t size;
};


struct ImmOperand
{
	int64_t value;
	uint8_t size;
};


struct MemOperand
{
	RegId base;
	RegId index;
	uint32_t scale;
	int64_t disp;
	uint8_t size;

	enum class ShiftType : uint8_t {
		None = 0,
		LSL,    // 逻辑左移
		LSR,    // 逻辑右移
		ASR,    // 算术右移
		ROR,    // 循环右移
		RRX,    // 带扩展的循环右移
	};

	ShiftType shiftType = ShiftType::None;
	uint8_t shiftValue = 0;

	// 是否有基址
	bool hasBase() const { return base.isValid(); }
	// 是否有索引
	bool hasIndex() const { return index.isValid(); }
	// 是否有位移
	bool hasDisp() const { return disp != 0; }
	// 是否有缩放
	bool hasScale() const { return scale > 1; }
};


// 统一操作数
struct Operand {
	OperandType type = OperandType::Invalid;
	OperandAccess access = OperandAccess::None;

	// 使用 variant 存储不同类型
	std::variant<
		std::monostate,  // Invalid
		RegOperand,
		ImmOperand,
		MemOperand
	> data;

	// 便捷访问
	bool isReg() const { return type == OperandType::Reg; }
	bool isImm() const { return type == OperandType::Imm; }
	bool isMem() const { return type == OperandType::Mem; }

	const RegOperand& reg() const { return std::get<RegOperand>(data); }
	const ImmOperand& imm() const { return std::get<ImmOperand>(data); }
	const MemOperand& mem() const { return std::get<MemOperand>(data); }

	// 工厂方法
	static Operand makeReg(RegId reg, uint8_t size, OperandAccess access = OperandAccess::Read) {
		Operand op;
		op.type = OperandType::Reg;
		op.access = access;
		op.data = RegOperand{ reg, size };
		return op;
	}

	static Operand makeImm(int64_t value, uint8_t size) {
		Operand op;
		op.type = OperandType::Imm;
		op.access = OperandAccess::Read;
		op.data = ImmOperand{ value, size };
		return op;
	}

	static Operand makeMem(const MemOperand& mem, OperandAccess access = OperandAccess::Read) {
		Operand op;
		op.type = OperandType::Mem;
		op.access = access;
		op.data = mem;
		return op;
	}
};
