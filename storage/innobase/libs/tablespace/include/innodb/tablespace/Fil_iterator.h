#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

#include <functional>

struct fil_node_t;

/** Iterate over the files in all the tablespaces. */
class Fil_iterator {
 public:
  using Function = std::function<dberr_t(fil_node_t *)>;

  /** For each data file, exclude redo log files.
  @param[in]	include_log	include files, if true
  @param[in]	f		Callback */
  template <typename F>
  static dberr_t for_each_file(bool include_log, F &&f) {
    return (iterate(include_log, [=](fil_node_t *file) { return (f(file)); }));
  }

  /** Iterate over the spaces and file lists.
  @param[in]	include_log	if true then fetch log files too
  @param[in,out]	f		Callback */
  static dberr_t iterate(bool include_log, Function &&f);
};
