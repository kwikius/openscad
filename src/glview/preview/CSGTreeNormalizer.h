#pragma once

#include <memory>

class CSGTreeNormalizer
{
public:
  CSGTreeNormalizer(size_t limit) : aborted(false), limit(limit), nodecount(0) {}
  ~CSGTreeNormalizer() {}

  std::shared_ptr<class CSGNode> normalize(const std::shared_ptr<CSGNode>& term);

private:
  std::shared_ptr<CSGNode> normalizePass(std::shared_ptr<CSGNode> term);
  bool match_and_replace(std::shared_ptr<class CSGNode>& term);
  std::shared_ptr<CSGNode> collapse_null_terms(const std::shared_ptr<CSGNode>& term);
  std::shared_ptr<CSGNode> cleanup_term(std::shared_ptr<CSGNode>& t);
  unsigned int count(const std::shared_ptr<CSGNode>& term) const;

  bool aborted;
  size_t limit;
  size_t nodecount;
  std::shared_ptr<class CSGNode> rootnode;
};
