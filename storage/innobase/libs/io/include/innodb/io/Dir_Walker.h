#pragma once

#include <innodb/univ/univ.h>

#include <functional>
#include <string>

/** Class to scan the directory heirarch using a depth first scan. */
class Dir_Walker {
 public:
  using Path = std::string;

  /** Check if the path is a directory. The file/directory must exist.
  @param[in]	path		The path to check
  @return true if it is a directory */
  static bool is_directory(const Path &path);

  /** Depth first traversal of the directory starting from basedir
  @param[in]	basedir		Start scanning from this directory
  @param[in]    recursive       True if scan should be recursive
  @param[in]	f		Function to call for each entry */
  template <typename F>
  static void walk(const Path &basedir, bool recursive, F &&f) {
#ifdef _WIN32
    walk_win32(basedir, recursive,
               [&](const Path &path, size_t depth) { f(path); });
#else
    walk_posix(basedir, recursive,
               [&](const Path &path, size_t depth) { f(path); });
#endif /* _WIN32 */
  }

 private:
  /** Directory names for the depth first directory scan. */
  struct Entry {
    /** Constructor
    @param[in]	path		Directory to traverse
    @param[in]	depth		Relative depth to the base
                                    directory in walk() */
    Entry(const Path &path, size_t depth) : m_path(path), m_depth(depth) {}

    /** Path to the directory */
    Path m_path;

    /** Relative depth of m_path */
    size_t m_depth;
  };

  using Function = std::function<void(const Path &, size_t)>;

  /** Depth first traversal of the directory starting from basedir
  @param[in]	basedir		Start scanning from this directory
  @param[in]    recursive       True if scan should be recursive
  @param[in]	f		Function to call for each entry */
#ifdef _WIN32
  static void walk_win32(const Path &basedir, bool recursive, Function &&f);
#else
  static void walk_posix(const Path &basedir, bool recursive, Function &&f);
#endif /* _WIN32 */
};
