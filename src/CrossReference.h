#pragma once

enum class ReferenceType
{
  None = 0,
  Jmp,
  Call,
  Ptr
};


class CrossReference
{
public:
  CrossReference();
  ~CrossReference();
private:
  ReferenceType type_;
};