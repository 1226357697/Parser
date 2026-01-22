#include "CapstoneDisassembler.h"
#include "X86OperandDecoder.h"
CapstoneDisassembler::CapstoneDisassembler(cs_arch arch, cs_mode mode)
{
  if (cs_open(arch, mode, &handle_) != CS_ERR_OK) {
    throw std::runtime_error("Failed to initialize Capstone");
  }
  cs_option(handle_, CS_OPT_DETAIL, CS_OPT_ON);

  // 根据架构创建解码器
  switch (arch) {
  case CS_ARCH_X86:
    decoder_ = std::make_unique<X86OperandDecoder>(handle_);
    break;

    default:
      assert(false);
      break;
  }

  cachedInsn_ = cs_malloc(handle_);
}

CapstoneDisassembler::~CapstoneDisassembler()
{
  cs_free(cachedInsn_, 1);
  cs_close(&handle_);
}

std::vector<Instruction> CapstoneDisassembler::disassemble(std::span<const uint8_t> code, uint64_t addr, size_t* outBytesConsumed)
{
  std::vector<Instruction> result{};
  cs_insn* insn = nullptr;
  size_t bytesConsumed = 0;

  size_t count = cs_disasm(handle_, code.data(), code.size(), addr, 0, &insn);

  if (count > 0) {
    result.reserve(count);
    for (size_t i = 0; i < count; i++) {
      Instruction inst  {
          insn[i].address,
          { insn[i].bytes, insn[i].bytes + insn[i].size },
          insn[i].mnemonic,
          insn[i].op_str,
          insn->id
      };

      // 解码结构化操作数
      if (decoder_ && insn->detail) {
        decoder_->decode(insn, inst);
      }

      result.push_back(inst);
      bytesConsumed += insn[i].size;
    }
    cs_free(insn, count);
  }

  return result;
}

std::optional<Instruction> CapstoneDisassembler::disassembleOne(std::span<const uint8_t> code, uint64_t addr, size_t* outBytesConsumed)
{
  const uint8_t* codePtr = code.data();
  size_t codeSize = code.size();
  uint64_t address = addr;

  // cs_disasm_iter 不分配内存，复用 cachedInsn_
  if (cs_disasm_iter(handle_, &codePtr, &codeSize, &address, cachedInsn_)) {
    Instruction inst{
        cachedInsn_->address,
        {cachedInsn_->bytes, cachedInsn_->bytes + cachedInsn_->size},
        cachedInsn_->mnemonic,
        cachedInsn_->op_str,
        cachedInsn_->id
    };
    if (decoder_ && cachedInsn_->detail) {
      decoder_->decode(cachedInsn_, inst);
    }
    return inst;
  }
  return std::nullopt;
}
