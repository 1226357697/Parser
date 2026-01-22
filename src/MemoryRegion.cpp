#include "MemoryRegion.h"
#include <assert.h>

MemoryRegion::MemoryRegion(Protect protect, RVA_t start, RVA_t end)
:protect_(protect)
{
  freeBlocks_.insert({start, end});
}

bool MemoryRegion::allocate(RVA_t start, RVA_t end)
{
  if(end < start)
  {
    assert(false);
    return false;
  }


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
  if (freeBlocks_.empty()) return freeBlocks_.end();

  // upper_bound 找到第一个 start > addr 的块
  auto it = freeBlocks_.upper_bound({ addr, addr });
  if (it != freeBlocks_.begin()) {
    --it;
    if (it->contains(addr)) return it;
  }
  return freeBlocks_.end();
}
