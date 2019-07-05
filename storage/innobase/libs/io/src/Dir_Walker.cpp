#include <innodb/io/Dir_Walker.h>

#ifndef _WIN32

#include <innodb/logger/info.h>
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



#endif
