#include <innodb/io/Dir_Walker.h>

#include <innodb/logger/info.h>
#include <innodb/io/os_file_type_t.h>
#include <innodb/io/os_file_status.h>

#include <sys/types.h>
#include <dirent.h>
#include <stack>

/** Depth first traversal of the directory starting from basedir
@param[in]	basedir		Start scanning from this directory
@param[in]      recursive       True if scan should be recursive
@param[in]	f		Function to call for each entry */
void Dir_Walker::walk_posix(const Path &basedir, bool recursive, Function &&f) {
  using Stack = std::stack<Entry>;

  Stack directories;

  directories.push(Entry(basedir, 0));

  while (!directories.empty()) {
    Entry current = directories.top();

    directories.pop();

    DIR *parent = opendir(current.m_path.c_str());

    if (parent == nullptr) {
      ib::info(ER_IB_MSG_784) << "Failed to walk directory"
                              << " '" << current.m_path << "'";

      continue;
    }

    if (!is_directory(current.m_path)) {
      f(current.m_path, current.m_depth);
    }

    struct dirent *dirent = nullptr;

    for (;;) {
      dirent = readdir(parent);

      if (dirent == nullptr) {
        break;
      }

      if (strcmp(dirent->d_name, ".") == 0 ||
          strcmp(dirent->d_name, "..") == 0) {
        continue;
      }

      Path path(current.m_path);

      if (path.back() != '/' && path.back() != '\\') {
        path += OS_PATH_SEPARATOR;
      }

      path.append(dirent->d_name);

      if (is_directory(path) && recursive) {
        directories.push(Entry(path, current.m_depth + 1));

      } else {
        f(path, current.m_depth + 1);
      }
    }

    closedir(parent);
  }
}


/** Check if the path is a directory. The file/directory must exist.
@param[in]	path		The path to check
@return true if it is a directory */
bool Dir_Walker::is_directory(const Path &path) {
  os_file_type_t type;
  bool exists;

  if (os_file_status(path.c_str(), &exists, &type)) {
    ut_ad(exists);
    ut_ad(type != OS_FILE_TYPE_MISSING);

    return (type == OS_FILE_TYPE_DIR);
  }

  ut_ad(exists || type == OS_FILE_TYPE_FAILED);
  ut_ad(type != OS_FILE_TYPE_MISSING);

  return (false);
}
