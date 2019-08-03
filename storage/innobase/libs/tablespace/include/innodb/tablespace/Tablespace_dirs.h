#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/disk/flags.h>
#include <innodb/tablespace/Scanned_files.h>
#include <innodb/tablespace/Space_id_set.h>
#include <innodb/tablespace/Tablespace_files.h>

#include <atomic>
#include <mutex>


/** Directories scanned during startup and the files discovered. */
class Tablespace_dirs {
 public:
  using Result = std::pair<std::string, Tablespace_files::Names *>;

  /** Constructor */
  Tablespace_dirs()
      : m_dirs()
#if !defined(__SUNPRO_CC)
        ,
        m_checked()
#endif /* !__SUNPRO_CC */
  {
#if defined(__SUNPRO_CC)
    m_checked = ATOMIC_VAR_INIT(0);
#endif /* __SUNPRO_CC */
  }

  /** Discover tablespaces by reading the header from .ibd files.
  @param[in]	in_directories	Directories to scan
  @return DB_SUCCESS if all goes well */
  dberr_t scan(const std::string &in_directories)
      MY_ATTRIBUTE((warn_unused_result));

  /** Clear all the tablespace file data but leave the list of
  scanned directories in place. */
  void clear() {
    for (auto &dir : m_dirs) {
      dir.clear();
    }

    m_checked = 0;
  }

  /** Erase a space ID to filename mapping.
  @param[in]	space_id	Tablespace ID to erase
  @return true if successful */
  bool erase(space_id_t space_id) MY_ATTRIBUTE((warn_unused_result)) {
    for (auto &dir : m_dirs) {
      if (dir.erase(space_id)) {
        return (true);
      }
    }

    return (false);
  }

  /* Find the first matching space ID -> name mapping.
  @param[in]	space_id	Tablespace ID
  @return directory searched and pointer to names that map to the
          tablespace ID */
  Result find(space_id_t space_id) MY_ATTRIBUTE((warn_unused_result)) {
    for (auto &dir : m_dirs) {
      const auto names = dir.find(space_id);

      if (names != nullptr) {
        return (Result{dir.path(), names});
      }
    }

    return (Result{"", nullptr});
  }

  /** @return the directory that contains path */
  const Fil_path &contains(const std::string &path) const
      MY_ATTRIBUTE((warn_unused_result)) {
    Fil_path file{path};

    for (const auto &dir : m_dirs) {
      const auto &d = dir.root().abs_path();
      auto abs_path = Fil_path::get_real_path(d);

      if (dir.root().is_ancestor(file) ||
          abs_path.compare(file.abs_path()) == 0) {
        return (dir.root());
      }
    }

    return (Fil_path::null());
  }

  /** Get the list of directories that InnoDB knows about.
  @return the list of directories 'dir1;dir2;....;dirN' */
  std::string get_dirs() const {
    std::string dirs;

    ut_ad(!m_dirs.empty());

    for (const auto &dir : m_dirs) {
      dirs.append(dir.root());
      dirs.push_back(FIL_PATH_SEPARATOR);
    }

    dirs.pop_back();

    ut_ad(!dirs.empty());

    return (dirs);
  }

 private:
  /** Print the duplicate filenames for a tablespce ID to the log
  @param[in]	duplicates	Duplicate tablespace IDs*/
  void print_duplicates(const Space_id_set &duplicates);

  /** first=dir path from the user, second=files found under first. */
  using Scanned = std::vector<Tablespace_files>;

  /** Tokenize a path specification. Convert relative paths to
  absolute paths. Check if the paths are valid and filter out
  invalid or unreadable directories.  Sort and filter out duplicates
  from dirs.
  @param[in]	str		Path specification to tokenize
  @param[in]	delimiters	Delimiters */
  void tokenize_paths(const std::string &str, const std::string &delimiters);

  using Const_iter = Scanned_files::const_iterator;

  /** Check for duplicate tablespace IDs.
  @param[in]	start		Start of slice
  @param[in]	end		End of slice
  @param[in]	thread_id	Thread ID
  @param[in,out]	mutex		Mutex protecting the global state
  @param[in,out]	unique		To check for duplciates
  @param[in,out]	duplicates	Duplicate space IDs found */
  void duplicate_check(const Const_iter &start, const Const_iter &end,
                       size_t thread_id, std::mutex *mutex,
                       Space_id_set *unique, Space_id_set *duplicates);

 private:
  /** Directories scanned and the files discovered under them. */
  Scanned m_dirs;

  /** Number of files checked. */
  std::atomic_size_t m_checked;
};

