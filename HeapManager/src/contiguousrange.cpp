#include <contiguousrange.h>

ContiguousRange::ContiguousRange(std::size_t pages, void *start)
    : m_pages(pages), m_start(start) {}

std::size_t ContiguousRange::getPages() { return m_pages; }

void *ContiguousRange::getStart() { return m_start; }

void ContiguousRange::setPages(std::size_t pages) { m_pages = pages; }

void ContiguousRange::setStart(void *start) { m_start = start; }