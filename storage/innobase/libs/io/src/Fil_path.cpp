#include <innodb/io/Fil_path.h>

#include <innodb/logger/error.h>
#include <innodb/logger/info.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/io/os_file_status.h>
#include <innodb/io/MySQL_datadir_path.h>

#include "my_io.h"

#include <algorithm>

#if defined(__SUNPRO_CC)
char *Fil_path::SEPARATOR = "\\/";
char *Fil_path::DOT_SLASH = "./";
char *Fil_path::DOT_DOT_SLASH = "../";
#endif /* defined(__SUNPRO_CC) */


ulint innobase_get_lower_case_table_names(void);
uint innobase_convert_to_filename_charset(char *to, const char *from, ulint len);
void innobase_casedn_path(char *a);

/* copied from mem0mem.ic Remove after modularization of memory */
/** Duplicates a NUL-terminated string.
 @return own: a copy of the string, must be deallocated with ut_free */
UNIV_INLINE
char *mem_strdup(const char *str) /*!< in: string to be copied */
{
  ulint len = strlen(str) + 1;
  return (static_cast<char *>(memcpy(ut_malloc_nokey(len), str, len)));
}

/** Sentinel value to check for "NULL" Fil_path. */
Fil_path Fil_path::s_null_path;



/** Constructor
@param[in]  path            pathname (may also include the file basename)
@param[in]  normalize_path  If false, it's the callers responsibility to
                            ensure that the path is normalized. */
Fil_path::Fil_path(const std::string &path, bool normalize_path)
    : m_path(path) {
  if (normalize_path) {
    normalize(m_path);
  }
  m_abs_path = get_real_path(m_path);
}

/** Constructor
@param[in]  path            pathname (may also include the file basename)
@param[in]  normalize_path  If false, it's the callers responsibility to
                            ensure that the path is normalized. */
Fil_path::Fil_path(const char *path, bool normalize_path) : m_path(path) {
  if (normalize_path) {
    normalize(m_path);
  }
  m_abs_path = get_real_path(m_path);
}

/** Constructor
@param[in]  path            pathname (may also include the file basename)
@param[in]  len             Length of path
@param[in]  normalize_path  If false, it's the callers responsibility to
                            ensure that the path is normalized. */
Fil_path::Fil_path(const char *path, size_t len, bool normalize_path)
    : m_path(path, len) {
  if (normalize_path) {
    normalize(m_path);
  }
  m_abs_path = get_real_path(m_path);
}

/** Default constructor. */
Fil_path::Fil_path() : m_path(), m_abs_path() { /* No op */
}


/** Check if the basename of a filepath is an undo tablespace name
@param[in]	name	Tablespace name
@return true if it is an undo tablespace name */
bool Fil_path::is_undo_tablespace_name(const std::string &name) {
  if (name.empty()) {
    return (false);
  }

  std::string basename(name);

  auto sep = basename.find_last_of(SEPARATOR);

  if (sep != std::string::npos) {
    basename.erase(basename.begin(), basename.begin() + sep + 1);
  }

  const auto end = basename.end();

  /* 5 is the minimum length for an explicit undo space name.
  It must be at least this long; "_.ibu". */
  if (basename.length() <= strlen(DOT_IBU)) {
    return (false);
  }

  /* Implicit undo names can come in two formats: undo_000 and undo000.
  Check for both. */
  size_t u = (*(end - 4) == '_') ? 1 : 0;

  if (basename.length() == sizeof("undo000") - 1 + u &&
      *(end - 7 - u) == 'u' && /* 'u' */
      *(end - 6 - u) == 'n' && /* 'n' */
      *(end - 5 - u) == 'd' && /* 'd' */
      *(end - 4 - u) == 'o' && /* 'o' */
      isdigit(*(end - 3)) &&   /* 'n' */
      isdigit(*(end - 2)) &&   /* 'n' */
      isdigit(*(end - 1))) {   /* 'n' */
    return (true);
  }

  if (basename.substr(basename.length() - 4, 4) == DOT_IBU) {
    return (true);
  }

  return (false);
}

/** Get the real path for a directory or a file name, useful for comparing
symlinked files. If path doesn't exist it will be ignored.
@param[in]	path		Directory or filename
@return the absolute path of path, or "" on error.  */
std::string Fil_path::get_real_path(const std::string &path) {
  char abspath[FN_REFLEN + 2];

  /* FIXME: This should be an assertion eventually. */
  if (path.empty()) {
    return (path);
  }

  int ret = my_realpath(abspath, path.c_str(), MYF(0));

  if (ret == -1) {
    ib::info(ER_IB_MSG_289) << "my_realpath(" << path << ") failed!";

    return (path);
  }

  std::string real_path(abspath);

  /* On Windows, my_realpath() puts a '\' at the end of any directory
  path, on non-Windows it does not. */

  if (!is_separator(real_path.back()) &&
      get_file_type(real_path) == OS_FILE_TYPE_DIR) {
    real_path.push_back(OS_SEPARATOR);
  }

  ut_a(real_path.length() < sizeof(abspath));

  return (real_path);
}

/** Allocate and build a file name from a path, a table or tablespace name
and a suffix.
@param[in]	path_in		nullptr or the direcory path or the full path
                                and filename
@param[in]	name_in		nullptr if path is full, or Table/Tablespace
                                name
@param[in]	ext		the file extension to use
@param[in]	trim		whether last name on the path should be trimmed
@return own: file name; must be freed by ut_free() */
char *Fil_path::make(const std::string &path_in, const std::string &name_in,
                     ib_file_suffix ext, bool trim) {
  /* The path should be a directory and should not contain the
  basename of the file. If the path is empty, we will use  the
  default path, */

  ut_ad(!path_in.empty() || !name_in.empty());

  std::string path;

  if (path_in.empty()) {
    if (is_absolute_path(name_in)) {
      path = "";
    } else {
      path.assign(MySQL_datadir_path);
    }
  } else {
    path.assign(path_in);
  }

  std::string name;

  if (!name_in.empty()) {
    name.assign(name_in);
  }

  /* Do not prepend the datadir path (which must be DOT_SLASH)
  if the name is an absolute path or a relative path like
  DOT_SLASH or DOT_DOT_SLASH.  */
  if (is_absolute_path(name) || has_prefix(name, DOT_SLASH) ||
      has_prefix(name, DOT_DOT_SLASH)) {
    path.clear();
  }

  std::string filepath;

  if (!path.empty()) {
    filepath.assign(path);
  }

  if (trim) {
    /* Find the offset of the last DIR separator and set it to
    null in order to strip off the old basename from this path. */
    auto pos = filepath.find_last_of(SEPARATOR);

    if (pos != std::string::npos) {
      filepath.resize(pos);
    }
  }

  if (!name.empty()) {
    if (!filepath.empty() && !is_separator(filepath.back())) {
      filepath.push_back(OS_SEPARATOR);
    }

    filepath.append(name);
  }

  /* Make sure that the specified suffix is at the end. */
  if (ext != NO_EXT) {
    const auto suffix = dot_ext[ext];
    size_t len = strlen(suffix);

    /* This assumes that the suffix starts with '.'.  If the
    first char of the suffix is found in the filepath at the
    same length as the suffix from the end, then we will assume
    that there is a previous suffix that needs to be replaced. */

    ut_ad(*suffix == '.');

    if (filepath.length() > len && *(filepath.end() - len) == *suffix) {
      filepath.replace(filepath.end() - len, filepath.end(), suffix);
    } else {
      filepath.append(suffix);
    }
  }

  normalize(filepath);

  return (mem_strdup(filepath.c_str()));
}


/** Create an IBD path name after replacing the basename in an old path
with a new basename.  The old_path is a full path name including the
extension.  The tablename is in the normal form "schema/tablename".

@param[in]	path_in			Pathname
@param[in]	name_in			Contains new base name
@return own: new full pathname */
std::string Fil_path::make_new_ibd(const std::string &path_in,
                                   const std::string &name_in) {
  ut_a(Fil_path::has_suffix(IBD, path_in));
  ut_a(!Fil_path::has_suffix(IBD, name_in));

  std::string path(path_in);

  auto pos = path.find_last_of(SEPARATOR);

  ut_a(pos != std::string::npos);

  path.resize(pos);

  pos = path.find_last_of(SEPARATOR);

  ut_a(pos != std::string::npos);

  path.resize(pos + 1);

  path.append(name_in + ".ibd");

  normalize(path);

  return (path);
}

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
void Fil_path::make_data_dir_path(char *data_dir_path) {
  /* Replace the period before the extension with a null byte. */
  ut_ad(has_suffix(IBD, data_dir_path));
  char *dot = strrchr((char *)data_dir_path, '.');
  *dot = '\0';

  /* The tablename starts after the last slash. */
  char *base_slash = strrchr((char *)data_dir_path, OS_PATH_SEPARATOR);
  ut_ad(base_slash != nullptr);

  *base_slash = '\0';

  std::string base_name{base_slash + 1};

  /* The database name starts after the next to last slash. */
  char *db_slash = strrchr((char *)data_dir_path, OS_SEPARATOR);
  ut_ad(db_slash != nullptr);
  char *db_name = db_slash + 1;

  /* Overwrite the db_name with the base_name. */
  memmove(db_name, base_name.c_str(), base_name.length());
  db_name[base_name.length()] = '\0';
}

/** @return true if the path exists and is a file . */
bool Fil_path::is_file_and_exists() const {
  return (get_file_type(m_abs_path) == OS_FILE_TYPE_FILE);
}

/** @return true if the path exists and is a directory. */
bool Fil_path::is_directory_and_exists() const {
  return (get_file_type(m_abs_path) == OS_FILE_TYPE_DIR);
}

/** This validation is only for ':'.
@return true if the path is valid. */
bool Fil_path::is_valid() const {
  auto count = std::count(m_path.begin(), m_path.end(), ':');

  if (count == 0) {
    return (true);
  }

#ifdef _WIN32
  /* Do not allow names like "C:name.ibd" because it
  specifies the "C:" drive but allows a relative location.
  It should be like "c:\". If a single colon is used it
  must be the second byte and the third byte must be a
  separator. */

  /* 8 == strlen("c:\a,ibd") */
  if (count == 1 && m_path.length() >= 8 && isalpha(m_path.at(0)) &&
      m_path.at(1) == ':' && (m_path.at(2) == '\\' || m_path.at(2) == '/')) {
    return (true);
  }
#endif /* _WIN32 */

  return (false);
}



#ifndef UNIV_HOTBACKUP

/** Check if the filepath provided is in a valid placement.
1) File-per-table must be in a dir named for the schema.
2) File-per-table must not be in the datadir.
3) General tablespace must not be under the datadir.
@param[in]	space_name	tablespace name
@param[in]	path		filepath to validate
@retval true if the filepath is a valid datafile location */
bool Fil_path::is_valid_location(const char *space_name,
                                 const std::string &path) {
  ut_ad(!path.empty());
  ut_ad(space_name != nullptr);

  std::string name{space_name};

  /* The path is a realpath to a file. Make sure it is not an
  undo tablespace filename. Undo datafiles can be located anywhere. */
  if (Fil_path::is_undo_tablespace_name(path)) {
    return (true);
  }

  /* Strip off the filename to reduce the path to a directory. */
  std::string dirpath{path};
  auto pos = dirpath.find_last_of(SEPARATOR);

  dirpath.resize(pos);

  /* Get the subdir that the file is in. */
  pos = dirpath.find_last_of(SEPARATOR);

  std::string subdir = (pos == std::string::npos)
                           ? dirpath
                           : dirpath.substr(pos + 1, dirpath.length());
  if (innobase_get_lower_case_table_names() == 2) {
    Fil_path::convert_to_lower_case(subdir);
  }

  pos = name.find_last_of(SEPARATOR);

  if (pos == std::string::npos) {
    /* This is a general or system tablespace. */

    if (MySQL_datadir_path.is_ancestor(dirpath)) {
      ib::error(ER_IB_MSG_388) << "A general tablespace cannot"
                               << " be located under the datadir."
                               << " Cannot open file '" << path << "'.";
      return (false);
    }

  } else {
    /* This is a file-per-table datafile.
    Reduce the name to just the db name. */

    if (MySQL_datadir_path.is_same_as(dirpath)) {
      ib::error(ER_IB_MSG_389) << "A file-per-table tablespace cannot"
                               << " be located in the datadir."
                               << " Cannot open file" << path << "'.";
      return (false);
    }

    /* In case of space_name in system charset, there is a possibility
    that the space_name contains more than one SEPARATOR character.
    We cannot rely on finding the last SEPARATOR only once.
    Search the space_name string backwards until we find the
    db name that matches with the directory name in dirpath. */

    while (pos < std::string::npos) {
      name.resize(pos);
      std::string temp = name;

      if (temp == subdir) {
        break;
      }

      /* Convert to filename charset and compare again. */
      Fil_path::convert_to_filename_charset(temp);
      if (temp == subdir) {
        break;
      }

      /* Still no match, iterate through the next SEPARATOR. */
      pos = name.find_last_of(SEPARATOR);

      /* If end of string is hit, there is no match. */
      if (pos == std::string::npos) {
        return (false);
      }
    }
  }

  return (true);
}


/** Convert filename to the file system charset format.
@param[in,out]	name		Filename to convert */
void Fil_path::convert_to_filename_charset(std::string &name) {
  uint errors = 0;
  char old_name[MAX_TABLE_NAME_LEN + 20];
  char filename[MAX_TABLE_NAME_LEN + 20];

  strncpy(filename, name.c_str(), sizeof(filename) - 1);
  strncpy(old_name, filename, sizeof(old_name));

  innobase_convert_to_filename_charset(filename, old_name, MAX_TABLE_NAME_LEN);

  if (errors == 0) {
    name.assign(filename);
  }
}

/** Convert to lower case using the file system charset.
@param[in,out]	path		Filepath to convert */
void Fil_path::convert_to_lower_case(std::string &path) {
  char lc_path[MAX_TABLE_NAME_LEN + 20];

  ut_ad(path.length() < sizeof(lc_path) - 1);

  strncpy(lc_path, path.c_str(), sizeof(lc_path) - 1);

  innobase_casedn_path(lc_path);

  path.assign(lc_path);
}

#endif




/** @return true if the path exists and is a file . */
os_file_type_t Fil_path::get_file_type(const std::string &path) {
  const std::string *ptr;
  os_file_type_t type;
  bool exists;

#ifdef _WIN32
  /* Temporarily strip the trailing_separator since it will cause
  stat64() to fail on Windows unless the path is the root of some
  drive; like "C:\".  _stat64() will fail if it is "C:". */

  std::string p{path};

  if (path.length() > 3 && is_separator(path.back()) &&
      path.at(p.length() - 2) != ':') {
    p.pop_back();
  }

  ptr = &p;
#else
  ptr = &path;
#endif /* WIN32 */

  os_file_status(ptr->c_str(), &exists, &type);

  return (type);
}
