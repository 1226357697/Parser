#include "MemoryRegion.h"

MemoryRegion::MemoryRegion(Protect protect, RVA_t start, RVA_t end)
:protect_(protect)
{
  freeBlocks_.insert({start, end});
}

bool MemoryRegion::allocate(RVA_t start, RVA_t end)
{
  // 找到包含这个区域的空闲块，分割它
  auto it = findFreeBlock(start);
  if (it == freeBlocks_.end()) return false;

  AddressRange block = *it;
  freeBlocks_.erase(it);

  // 分割：保留前后的空闲部分
  if (block.start < start) {
    freeBlocks_.insert({ block.start, start });
  }
  if (end < block.end) {
    freeBlocks_.insert({ end, block.end });
  }

  return true;
}

std::set<AddressRange>::iterator MemoryRegion::findFreeBlock(RVA_t addr)
{
  for (auto it = freeBlocks_.begin(); it != freeBlocks_.end(); ++it) {
    if (it->contains(addr)) return it;
  }
  return freeBlocks_.end();
}
