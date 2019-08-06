#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <innodb/io/Fil_path.h>

#include <string>
#include <vector>
#include <unordered_map>

/** Tablespace files disovered during startup. */
class Tablespace_files {
 public:
  using Names = std::vector<std::string, ut_allocator<std::string>>;
  using Paths = std::unordered_map<space_id_t, Names>;

  /** Default constructor
  @param[in]	dir		Directory that the files are under */
  explicit Tablespace_files(const std::string &dir);

  /** Add a space ID to filename mapping.
  @param[in]	space_id	Tablespace ID
  @param[in]	name		File name.
  @return number of files that map to the space ID */
  size_t add(space_id_t space_id, const std::string &name)
      MY_ATTRIBUTE((warn_unused_result));

  /** Get the file names that map to a space ID
  @param[in]	space_id	Tablespace ID
  @return the filenames that map to space id */
  Names *find(space_id_t space_id) MY_ATTRIBUTE((warn_unused_result));

  bool erase(space_id_t space_id) MY_ATTRIBUTE((warn_unused_result));

  /** Clear all the tablespace data. */
  void clear() {
    m_ibd_paths.clear();
    m_undo_paths.clear();
  }

  /** @return m_dir */
  const Fil_path &root() const { return (m_dir); }

  /** @return the directory path specified by the user. */
  const std::string &path() const { return (m_dir.path()); }

  /** @return the real path of the directory searched. */
  const std::string &real_path() const { return (m_dir.abs_path()); }

 private:
  /* Note:  The file names in m_ibd_paths and m_undo_paths are relative
  to m_real_path. */

  /** Mapping from tablespace ID to data filenames */
  Paths m_ibd_paths;

  /** Mapping from tablespace ID to Undo files */
  Paths m_undo_paths;

  /** Top level directory where the above files were found. */
  const Fil_path m_dir;
};
