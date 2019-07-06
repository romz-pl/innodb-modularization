#include <innodb/tablespace/Tablespace_dirs.h>

#include <innodb/io/os_file_stat_t.h>
#include <innodb/logger/info.h>
#include <innodb/logger/warn.h>
#include <innodb/tablespace/Dirs.h>
#include <innodb/time/ut_time.h>

#include <algorithm>

#include "dict0dict.h"

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


