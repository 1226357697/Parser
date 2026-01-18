#pragma once

#include "BinaryTypes.h"
#include <set>

struct AddressRange {
  RVA_t start;
  RVA_t end;  // exclusive

  size_t size() const { return end - start; }
  bool contains(RVA_t addr) const { return addr >= start && addr < end; }
  bool overlaps(const AddressRange& other) const {
    return start < other.end && other.start < end;
  }

  bool operator<(const AddressRange& other) const {
    return start < other.start;
  }
};

class MemoryRegion
{
public:
  enum class Protect
  {
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
    Execute = 1 << 2,
  };

  MemoryRegion(Protect protect, RVA_t start, RVA_t end);

  bool allocate(RVA_t start, RVA_t end);

  inline const std::set<AddressRange>& freeBlock() const { return freeBlocks_;}
  inline  Protect protect() const { return protect_;}

private:
  std::set<AddressRange>::iterator findFreeBlock(RVA_t addr);

private:
  std::set<AddressRange> freeBlocks_;
  Protect protect_;

};