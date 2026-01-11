#pragma once
#include "BinaryTypes.h"
#include <set>
#include <unordered_map>

enum class XrefType
{
  kNone = 0,
  kJmp,
  kCall,
  kPtr
};


struct Xref
{
  RVA_t from;
  RVA_t to;
  XrefType type;

  bool operator <(const Xref& other) const
  {
    if (from != other.from) return from < other.from;
    if (to != other.to) return to < other.to;
    return type < other.type;
  }
};

class XrefManager
{
public:
  XrefManager() = default;

  bool addXref(const Xref& xref);

  const std::set<Xref>& getXrefsFrom(RVA_t rva);
  const std::set<Xref>& getXrefsTo(RVA_t rva);

private:
  std::unordered_map<RVA_t, std::set<Xref>> xrefFrom_;
  std::unordered_map<RVA_t, std::set<Xref>> xrefTo_;
};