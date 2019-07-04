#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_type_t.h>
#include <innodb/io/ib_file_suffix.h>
#include <innodb/assert/assert.h>

#include <string>
#include <string.h>

/** Wrapper for a path to a directory that may or may not exist. */
class Fil_path {
 public:
  /** schema '/' table separator */
  static constexpr auto DB_SEPARATOR = '/';

  /** OS specific path separator. */
  static constexpr auto OS_SEPARATOR = OS_PATH_SEPARATOR;

  /** Directory separators that are supported. */
#if defined(__SUNPRO_CC)
  static char *SEPARATOR;
  static char *DOT_SLASH;
  static char *DOT_DOT_SLASH;
#else
  static constexpr auto SEPARATOR = "\\/";
#ifdef _WIN32
  static constexpr auto DOT_SLASH = ".\\";
  static constexpr auto DOT_DOT_SLASH = "..\\";
#else
  static constexpr auto DOT_SLASH = "./";
  static constexpr auto DOT_DOT_SLASH = "../";
#endif /* _WIN32 */

#endif /* __SUNPRO_CC */

  /** Various types of file paths. */
  enum path_type { absolute, relative, file_name_only, invalid };

  /** Default constructor. Defaults to MySQL_datadir_path.  */
  Fil_path();

  /** Constructor
  @param[in]  path            Path, not necessarily NUL terminated
  @param[in]  len             Length of path
  @param[in]  normalize_path  If false, it's the callers responsibility to
                              ensure that the path is normalized. */
  explicit Fil_path(const char *path, size_t len, bool normalize_path = false);

  /** Constructor
  @param[in]  path            Path, not necessarily NUL terminated
  @param[in]  normalize_path  If false, it's the callers responsibility to
                              ensure that the path is normalized. */
  explicit Fil_path(const char *path, bool normalize_path = false);

  /** Constructor
  @param[in]  path            pathname (may also include the file basename)
  @param[in]  normalize_path  If false, it's the callers responsibility to
                              ensure that the path is normalized. */
  explicit Fil_path(const std::string &path, bool normalize_path = false);

  /** Implicit type conversion
  @return pointer to m_path.c_str() */
  operator const char *() const { return (m_path.c_str()); }

  /** Explicit type conversion
  @return pointer to m_path.c_str() */
  const char *operator()() const { return (m_path.c_str()); }

  /** @return the value of m_path */
  const std::string &path() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_path);
  }

  /** @return the length of m_path */
  size_t len() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_path.length());
  }

  /** @return the length of m_abs_path */
  size_t abs_len() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_abs_path.length());
  }

  /** Determine if this path is equal to the other path.
  @param[in]	lhs		Path to compare to
  @return true if the paths are the same */
  bool operator==(const Fil_path &lhs) const {
    return (m_path.compare(lhs.m_path));
  }

  /** Check if m_path is the same as path.
  @param[in]	path	directory path to compare to
  @return true if m_path is the same as path */
  bool is_same_as(const std::string &path) const
      MY_ATTRIBUTE((warn_unused_result)) {
    if (m_path.empty() || path.empty()) {
      return (false);
    }

    return (m_abs_path == get_real_path(path));
  }

  /** Check if m_path is the parent of name.
  @param[in]	name		Path to compare to
  @return true if m_path is an ancestor of name */
  bool is_ancestor(const std::string &name) const
      MY_ATTRIBUTE((warn_unused_result)) {
    if (m_path.empty() || name.empty()) {
      return (false);
    }

    return (is_ancestor(m_abs_path, get_real_path(name)));
  }

  /** Check if m_path is the parent of other.m_path.
  @param[in]	other		Path to compare to
  @return true if m_path is an ancestor of name */
  bool is_ancestor(const Fil_path &other) const
      MY_ATTRIBUTE((warn_unused_result)) {
    if (m_path.empty() || other.m_path.empty()) {
      return (false);
    }

    return (is_ancestor(m_abs_path, other.m_abs_path));
  }

  /** @return true if m_path exists and is a file. */
  bool is_file_and_exists() const MY_ATTRIBUTE((warn_unused_result));

  /** @return true if m_path exists and is a directory. */
  bool is_directory_and_exists() const MY_ATTRIBUTE((warn_unused_result));

  /** Return the absolute path */
  const std::string &abs_path() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_abs_path);
  }

  /** This validation is only for ':'.
  @return true if the path is valid. */
  bool is_valid() const MY_ATTRIBUTE((warn_unused_result));

  /** Remove quotes e.g., 'a;b' or "a;b" -> a;b.
  Assumes matching quotes.
  @return pathspec with the quotes stripped */
  static std::string parse(const char *pathspec) {
    std::string path(pathspec);

    ut_ad(!path.empty());

    if (path.size() >= 2 && (path.front() == '\'' || path.back() == '"')) {
      path.erase(0, 1);

      if (path.back() == '\'' || path.back() == '"') {
        path.erase(path.size() - 1);
      }
    }

    return (path);
  }

  /** Convert the paths into absolute paths and compare them. The
  paths to compare must be valid paths, otherwise the result is
  undefined.
  @param[in]	lhs		Filename to compare
  @param[in]	rhs		Filename to compare
  @return true if they are the same */
  static bool equal(const std::string &lhs, const std::string &rhs)
      MY_ATTRIBUTE((warn_unused_result)) {
    Fil_path path1(lhs);
    Fil_path path2(rhs);

    return (path1.abs_path().compare(path2.abs_path()) == 0);
  }

  /** @return true if the path is an absolute path. */
  bool is_relative_path() const MY_ATTRIBUTE((warn_unused_result)) {
    return (type_of_path(m_path) == relative);
  }

  /** @return true if the path is an absolute path. */
  bool is_absolute_path() const MY_ATTRIBUTE((warn_unused_result)) {
    return (type_of_path(m_path) == absolute);
  }

  /** Determine if a path is an absolute path or not.
  @param[in]	path		OS directory or file path to evaluate
  @retval true if an absolute path
  @retval false if a relative path */
  static bool is_absolute_path(const std::string &path) {
    return (type_of_path(path) == absolute);
  }

  /** Determine if a path is an absolute path or not.
  @param[in]	path		OS directory or file path to evaluate
  @retval true if an absolute path
  @retval false if a relative path */
  static path_type type_of_path(const std::string &path)
      MY_ATTRIBUTE((warn_unused_result)) {
    if (path.empty()) {
      return (invalid);
    }

    /* The most likely type is a file name only with no separators. */
    if (path.find('\\', 0) == std::string::npos &&
        path.find('/', 0) == std::string::npos) {
      return (file_name_only);
    }

    /* Any string that starts with an OS_SEPARATOR is
    an absolute path. This includes any OS and even
    paths like "\\Host\share" on Windows. */
    if (path.at(0) == '\\' || path.at(0) == '/') {
      return (absolute);
    }

#ifdef _WIN32
    /* Windows may have an absolute path like 'A:\' */
    if (path.length() >= 3 && isalpha(path.at(0)) && path.at(1) == ':' &&
        (path.at(2) == '\\' || path.at(2) == '/')) {
      return (absolute);
    }
#endif /* _WIN32 */

    /* Since it contains separators and is not an absolute path,
    it must be a relative path. */
    return (relative);
  }

  /* Check if the path is prefixed with pattern.
  @return true if prefix matches */
  static bool has_prefix(const std::string &path, const std::string prefix)
      MY_ATTRIBUTE((warn_unused_result)) {
    return (path.size() >= prefix.size() &&
            std::equal(prefix.begin(), prefix.end(), path.begin()));
  }

  /** Normalizes a directory path for the current OS:
  On Windows, we convert '/' to '\', else we convert '\' to '/'.
  @param[in,out]	path	Directory and file path */
  static void normalize(std::string &path) {
    for (auto &c : path) {
      if (c == OS_PATH_SEPARATOR_ALT) {
        c = OS_SEPARATOR;
      }
    }
  }

  /** Normalizes a directory path for the current OS:
  On Windows, we convert '/' to '\', else we convert '\' to '/'.
  @param[in,out]	path	A NUL terminated path */
  static void normalize(char *path) {
    for (auto ptr = path; *ptr; ++ptr) {
      if (*ptr == OS_PATH_SEPARATOR_ALT) {
        *ptr = OS_SEPARATOR;
      }
    }
  }

  /** @return true if the path exists and is a file . */
  static os_file_type_t get_file_type(const std::string &path)
      MY_ATTRIBUTE((warn_unused_result));

  /** Get the real path for a directory or a file name, useful for
  comparing symlinked files.
  @param[in]	path		Directory or filename
  @return the absolute path of dir + filename, or "" on error.  */
  static std::string get_real_path(const std::string &path)
      MY_ATTRIBUTE((warn_unused_result));

  /** Check if lhs is the ancestor of rhs. If the two paths are the
  same it will return false.
  @param[in]	lhs		Parent path to check
  @param[in]	rhs		Descendent path to check
  @return true if lhs is an ancestor of rhs */
  static bool is_ancestor(const std::string &lhs, const std::string &rhs)
      MY_ATTRIBUTE((warn_unused_result)) {
    if (lhs.empty() || rhs.empty() || rhs.length() <= lhs.length()) {
      return (false);
    }

    return (std::equal(lhs.begin(), lhs.end(), rhs.begin()));
  }

  /** Check if the name is an undo tablespace name.
  @param[in]	name		Tablespace name
  @return true if it is an undo tablespace name */
  static bool is_undo_tablespace_name(const std::string &name)
      MY_ATTRIBUTE((warn_unused_result));

  /** Check if the file has the the specified suffix
  @param[in]	sfx		suffix to look for
  @param[in]	path		Filename to check
  @return true if it has the the ".ibd" suffix. */
  static bool has_suffix(ib_file_suffix sfx, const std::string &path) {
    const auto suffix = dot_ext[sfx];
    size_t len = strlen(suffix);

    return (path.size() >= len &&
            path.compare(path.size() - len, len, suffix) == 0);
  }

  /** Check if a character is a path separator ('\' or '/')
  @param[in]	c		Character to check
  @return true if it is a separator */
  static bool is_separator(char c) { return (c == '\\' || c == '/'); }

  /** Allocate and build a file name from a path, a table or
  tablespace name and a suffix.
  @param[in]	path_in		nullptr or the direcory path or
                                  the full path and filename
  @param[in]	name_in		nullptr if path is full, or
                                  Table/Tablespace name
  @param[in]	ext		the file extension to use
  @param[in]      trim            whether last name on the path should
                                  be trimmed
  @return own: file name; must be freed by ut_free() */
  static char *make(const std::string &path_in, const std::string &name_in,
                    ib_file_suffix ext, bool trim = false)
      MY_ATTRIBUTE((warn_unused_result));

  /** Allocate and build a CFG file name from a path.
  @param[in]	path_in		Full path to the filename
  @return own: file name; must be freed by ut_free() */
  static char *make_cfg(const std::string &path_in)
      MY_ATTRIBUTE((warn_unused_result)) {
    return (make(path_in, "", CFG));
  }

  /** Allocate and build a CFP file name from a path.
  @param[in]	path_in		Full path to the filename
  @return own: file name; must be freed by ut_free() */
  static char *make_cfp(const std::string &path_in)
      MY_ATTRIBUTE((warn_unused_result)) {
    return (make(path_in, "", CFP));
  }

  /** Allocate and build a file name from a path, a table or
  tablespace name and a suffix.
  @param[in]	path_in		nullptr or the direcory path or
                                  the full path and filename
  @param[in]	name_in		nullptr if path is full, or
                                  Table/Tablespace name
  @return own: file name; must be freed by ut_free() */
  static char *make_ibd(const std::string &path_in, const std::string &name_in)
      MY_ATTRIBUTE((warn_unused_result)) {
    return (make(path_in, name_in, IBD));
  }

  /** Allocate and build a file name from a path, a table or
  tablespace name and a suffix.
  @param[in]	name_in		Table/Tablespace name
  @return own: file name; must be freed by ut_free() */
  static char *make_ibd_from_table_name(const std::string &name_in)
      MY_ATTRIBUTE((warn_unused_result)) {
    return (make("", name_in, IBD));
  }

  /** Create an IBD path name after replacing the basename in an old path
  with a new basename.  The old_path is a full path name including the
  extension.  The tablename is in the normal form "schema/tablename".
  @param[in]	path_in			Pathname
  @param[in]	name_in			Contains new base name
  @return new full pathname */
  static std::string make_new_ibd(const std::string &path_in,
                                  const std::string &name_in)
      MY_ATTRIBUTE((warn_unused_result));

  /** This function reduces a null-terminated full remote path name
  into the path that is sent by MySQL for DATA DIRECTORY clause.
  It replaces the 'databasename/tablename.ibd' found at the end of the
  path with just 'tablename'.

  Since the result is always smaller than the path sent in, no new
  memory is allocated. The caller should allocate memory for the path
  sent in. This function manipulates that path in place. If the path
  format is not as expected, set data_dir_path to "" and return.

  The result is used to inform a SHOW CREATE TABLE command.
  @param[in,out]	data_dir_path	Full path/data_dir_path */
  static void make_data_dir_path(char *data_dir_path);

  /** @return the null path */
  static const Fil_path &null() MY_ATTRIBUTE((warn_unused_result)) {
    return (s_null_path);
  }

#ifndef UNIV_HOTBACKUP
  /** Check if the filepath provided is in a valid placement.
  1) File-per-table must be in a dir named for the schema.
  2) File-per-table must not be in the datadir.
  3) General tablespace must no be under the datadir.
  @param[in]	space_name	tablespace name
  @param[in]	path		filepath to validate
  @retval true if the filepath is a valid datafile location */
  static bool is_valid_location(const char *space_name,
                                const std::string &path);

  /** Convert filename to the file system charset format.
  @param[in,out]	name		Filename to convert */
  static void convert_to_filename_charset(std::string &name);

  /** Convert to lower case using the file system charset.
  @param[in,out]	path		Filepath to convert */
  static void convert_to_lower_case(std::string &path);

#endif /* !UNIV_HOTBACKUP */

 protected:
  /** Path to a file or directory. */
  std::string m_path;

  /** A full absolute path to the same file. */
  std::string m_abs_path;

  /** Empty (null) path. */
  static Fil_path s_null_path;
};
