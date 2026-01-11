#include "CrossReference.h"


bool XrefManager::addXref(const Xref& xref)
{
  // 添加到来源映射
  xrefFrom_[xref.from].insert(xref);

  // 添加到目标映射（创建反向引用）
  Xref reverse_xref = xref;
  std::swap(reverse_xref.from, reverse_xref.to);
  xrefTo_[xref.to].insert(reverse_xref);
  return true;
}

const std::set<Xref>& XrefManager::getXrefsFrom(RVA_t rva)
{
  static const std::set<Xref> empty_set;
  auto it = xrefFrom_.find(rva);
  return it != xrefFrom_.end() ? it->second : empty_set;
}

const std::set<Xref>& XrefManager::getXrefsTo(RVA_t rva)
{
  static const std::set<Xref> empty_set;
  auto it = xrefTo_.find(rva);
  return it != xrefTo_.end() ? it->second : empty_set;
}
