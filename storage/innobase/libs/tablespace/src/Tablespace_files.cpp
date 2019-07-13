#include <innodb/tablespace/Tablespace_files.h>

#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/dict_sys_t_is_reserved.h>
#include <innodb/logger/warn.h>

/** Constructor
@param[in]	dir		Directory that the files are under */
Tablespace_files::Tablespace_files(const std::string &dir)
    : m_ibd_paths(), m_undo_paths(), m_dir(dir) {
  ut_ad(Fil_path::is_separator(dir.back()));
}

/** Get the file names that map to a space ID
@param[in]	space_id	Tablespace ID
@return the filenames that map to space id */
Tablespace_files::Names *Tablespace_files::find(space_id_t space_id) {
  ut_ad(space_id != TRX_SYS_SPACE);

  if (dict_sys_t_is_reserved(space_id) &&
      space_id != dict_sys_t_s_space_id) {
    auto it = m_undo_paths.find(space_id);

    if (it != m_undo_paths.end()) {
      return (&it->second);
    }

  } else {
    auto it = m_ibd_paths.find(space_id);

    if (it != m_ibd_paths.end()) {
      return (&it->second);
    }
  }

  return (nullptr);
}


bool Tablespace_files::erase(space_id_t space_id) {
  ut_ad(space_id != TRX_SYS_SPACE);

  if (dict_sys_t_is_reserved(space_id) &&
      space_id != dict_sys_t_s_space_id) {
    auto n_erased = m_undo_paths.erase(space_id);

    return (n_erased == 1);
  } else {
    auto n_erased = m_ibd_paths.erase(space_id);

    return (n_erased == 1);
  }

  return (false);
}

/** Add a space ID to filename mapping.
@param[in]	space_id	Tablespace ID
@param[in]	name		File name.
@return number of files that map to the space ID */
size_t Tablespace_files::add(space_id_t space_id, const std::string &name) {
  ut_a(space_id != TRX_SYS_SPACE);

  Names *names;

  if (Fil_path::is_undo_tablespace_name(name)) {
    if (!dict_sys_t_is_reserved(space_id) &&
        0 == strncmp(name.c_str(), "undo_", 5)) {
      ib::warn(ER_IB_MSG_267) << "Tablespace '" << name << "' naming"
                              << " format is like an undo tablespace"
                              << " but its ID " << space_id << " is not"
                              << " in the undo tablespace range";
    }

    names = &m_undo_paths[space_id];

  } else {
    ut_ad(Fil_path::has_suffix(IBD, name.c_str()));

    names = &m_ibd_paths[space_id];
  }

  names->push_back(name);

  return (names->size());
}



