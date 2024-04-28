#pragma once

#include <cstddef>

class ContiguousRange {
public:
  ContiguousRange(std::size_t, void *);

  void *getStart();
  void setStart(void *);

  std::size_t getPages();
  void setPages(std::size_t);

private:
  void *m_start;
  std::size_t m_pages;
};
