#include <innodb/tablespace/Tablespace_dirs.h>

#include <innodb/io/Dir_Walker.h>
#include <innodb/io/os_file_stat_t.h>
#include <innodb/logger/info.h>
#include <innodb/logger/warn.h>
#include <innodb/tablespace/Dirs.h>
#include <innodb/tablespace/Fil_system.h>
#include <innodb/time/ut_time.h>

#include <algorithm>


/** Check for duplicate tablespace IDs.
@param[in]	start		Slice start
@param[in]	end		Slice end
@param[in]	thread_id	Thread ID
@param[in,out]	mutex		Mutex that covers the global state
@param[in,out]	unique		To check for duplciates
@param[in,out]	duplicates	Duplicate space IDs found */
void Tablespace_dirs::duplicate_check(const Const_iter &start,
                                      const Const_iter &end, size_t thread_id,
                                      std::mutex *mutex, Space_id_set *unique,
                                      Space_id_set *duplicates) {
  size_t count = 0;
  bool printed_msg = false;
  auto start_time = ut_time();

  for (auto it = start; it != end; ++it, ++m_checked) {
    const std::string filename = it->second;
    auto &files = m_dirs[it->first];
    const std::string phy_filename = files.path() + filename;

    space_id_t space_id;

    space_id = Fil_system::get_tablespace_id(phy_filename);

    if (space_id != 0 && space_id != dict_sys_t_s_invalid_space_id) {
      std::lock_guard<std::mutex> guard(*mutex);

      auto ret = unique->insert(space_id);

      size_t n_files;

      n_files = files.add(space_id, filename);

      if (n_files > 1 || !ret.second) {
        duplicates->insert(space_id);
      }

    } else if (space_id != 0 &&
               Fil_path::is_undo_tablespace_name(phy_filename)) {
      ib::info(ER_IB_MSG_373) << "Can't determine the undo file tablespace"
                              << " ID for '" << phy_filename << "', could be"
                              << " an undo truncate in progress";

    } else {
      ib::info(ER_IB_MSG_374) << "Ignoring '" << phy_filename << "' invalid"
                              << " tablespace ID in the header";
    }

    ++count;

    if (ut_time() - start_time >= PRINT_INTERVAL_SECS) {
      ib::info(ER_IB_MSG_375) << "Thread# " << thread_id << " - Checked "
                              << count << "/" << (end - start) << " files";

      start_time = ut_time();

      printed_msg = true;
    }
  }

  if (printed_msg) {
    ib::info(ER_IB_MSG_376) << "Checked " << count << " files";
  }
}

/** Print the duplicate filenames for a tablespce ID to the log
@param[in]	duplicates	Duplicate tablespace IDs*/
void Tablespace_dirs::print_duplicates(const Space_id_set &duplicates) {
  /* Print the duplicate names to the error log. */
  for (auto space_id : duplicates) {
    Dirs files;

    for (auto &dir : m_dirs) {
      const auto names = dir.find(space_id);

      if (names == nullptr) {
        continue;
      }

      files.insert(files.end(), names->begin(), names->end());
    }

    /* Fixes the order in the mtr tests. */
    std::sort(files.begin(), files.end());

    ut_a(files.size() > 1);

    std::ostringstream oss;

    oss << "Tablespace ID: " << space_id << " = [";

    for (size_t i = 0; i < files.size(); ++i) {
      oss << "'" << files[i] << "'";

      if (i < files.size() - 1) {
        oss << ", ";
      }
    }

    oss << "]" << std::endl;

    ib::error(ER_IB_MSG_377) << oss.str();
  }
}


/** Tokenize a path specification. Convert relative paths to absolute paths.
Check if the paths are valid and filter out invalid or unreadable directories.
Sort and filter out duplicates from dirs.
@param[in]	str		Path specification to tokenize
@param[in]	delimiters	Delimiters */
void Tablespace_dirs::tokenize_paths(const std::string &str,
                                     const std::string &delimiters) {
  std::string::size_type start = str.find_first_not_of(delimiters);
  std::string::size_type end = str.find_first_of(delimiters, start);

  using Paths = std::vector<std::pair<std::string, std::string>>;

  Paths dirs;

  /* Scan until 'end' and 'start' don't reach the end of string (npos) */
  while (std::string::npos != start || std::string::npos != end) {
    std::array<char, OS_FILE_MAX_PATH> dir;

    dir.fill(0);

    const auto path = str.substr(start, end - start);

    ut_a(path.length() < dir.max_size());

    std::copy(path.begin(), path.end(), dir.data());

    /* Filter out paths that contain '*'. */
    auto pos = path.find('*');

    /* Filter out invalid path components. */

    if (path == "/") {
      ib::warn(ER_IB_MSG_365) << "Scan path '" << path << "' ignored";

    } else if (pos == std::string::npos) {
      Fil_path::normalize(dir.data());

      std::string cur_path;
      std::string d{dir.data()};

      if (Fil_path::get_file_type(dir.data()) == OS_FILE_TYPE_DIR) {
        cur_path = Fil_path::get_real_path(d);

      } else {
        cur_path = d;
      }

      if (!Fil_path::is_separator(d.back())) {
        d.push_back(Fil_path::OS_SEPARATOR);
      }

      using value = Paths::value_type;

      dirs.push_back(value(d, cur_path));

    } else {
      ib::warn(ER_IB_MSG_366) << "Scan path '" << path << "' ignored"
                              << " contains '*'";
    }

    start = str.find_first_not_of(delimiters, end);

    end = str.find_first_of(delimiters, start);
  }

  /* Remove duplicate paths by comparing the real paths.  Note, this
  will change the order of the directory scan because of the sort. */

  using type = Paths::value_type;

  std::sort(dirs.begin(), dirs.end(), [](const type &lhs, const type &rhs) {
    return (lhs.second < rhs.second);
  });

  dirs.erase(std::unique(dirs.begin(), dirs.end(),
                         [](const type &lhs, const type &rhs) {
                           return (lhs.second == rhs.second);
                         }),
             dirs.end());

  /* Eliminate sub-trees */

  Dirs scan_dirs;

  for (size_t i = 0; i < dirs.size(); ++i) {
    const auto &path_i = dirs[i].second;

    for (size_t j = i + 1; j < dirs.size(); ++j) {
      auto &path_j = dirs[j].second;

      if (Fil_path::is_ancestor(path_i, path_j)) {
        path_j.resize(0);
      }
    }
  }

  for (auto &dir : dirs) {
    if (dir.second.length() == 0) {
      continue;
    }

    Fil_path::normalize(dir.first);

    m_dirs.push_back(Tablespace_files{dir.first});
  }
}


