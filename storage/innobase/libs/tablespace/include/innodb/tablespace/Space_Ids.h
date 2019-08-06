#pragma once

#include <innodb/univ/univ.h>


#include <innodb/allocator/ut_allocator.h>

#include <vector>
#include <algorithm>

/** List of undo tablespace IDs. */
class Space_Ids : public std::vector<space_id_t, ut_allocator<space_id_t>> {
 public:
  void sort() { std::sort(begin(), end()); }

  bool contains(space_id_t id) {
    if (size() == 0) {
      return (false);
    }

    iterator it = std::find(begin(), end(), id);

    return (it != end());
  }

  iterator find(space_id_t id) { return (std::find(begin(), end(), id)); }
};
