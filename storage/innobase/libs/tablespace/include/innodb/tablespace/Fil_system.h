#pragma once

#include <innodb/univ/univ.h>

#include <innodb/logger/warn.h>
#include <innodb/tablespace/Fil_iterator.h>
#include <innodb/tablespace/Fil_shard.h>
#include <innodb/tablespace/Tablespace_dirs.h>
#include <innodb/tablespace/Tablespaces.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fil_load_status.h>
#include <innodb/tablespace/fil_space_t.h>

#include <innodb/tablespace/consts.h>


#include "sql/dd/object_id.h"

#include <vector>
#include <string>

/** We want to store the line number from where it was called. */
#define mutex_acquire() acquire(__LINE__)


bool fsp_is_undo_tablespace(space_id_t space_id);

/** The tablespace memory cache; also the totality of logs (the log
data space) is stored here; below we talk about tablespaces, but also
the ib_logfiles form a 'space' and it is handled here */
class Fil_system {
 public:
  using Fil_shards = std::vector<Fil_shard *>;

  /** Constructor.
  @param[in]	n_shards	Number of shards to create
  @param[in]	max_open	Maximum number of open files */
  Fil_system(size_t n_shards, size_t max_open);

  /** Destructor */
  ~Fil_system();

  /** Fetch the file names opened for a space_id during recovery.
  @param[in]	space_id	Tablespace ID to lookup
  @return pair of top level directory scanned and names that map
          to space_id or nullptr if not found for names */
  Tablespace_dirs::Result get_scanned_files(space_id_t space_id)
      MY_ATTRIBUTE((warn_unused_result)) {
    return (m_dirs.find(space_id));
  }

  /** Fetch the file name opened for a space_id during recovery
  from the file map.
  @param[in]	space_id	Undo tablespace ID
  @return Full path to the file name that was opened, empty string
          if space ID not found. */
  std::string find(space_id_t space_id) MY_ATTRIBUTE((warn_unused_result)) {
    auto result = get_scanned_files(space_id);

    if (result.second != nullptr) {
      return (result.first + result.second->front());
    }

    return ("");
  }

  /** Erase a tablespace ID and its mapping from the scanned files.
  @param[in]	space_id	Tablespace ID to erase
  @return true if successful */
  bool erase(space_id_t space_id) MY_ATTRIBUTE((warn_unused_result)) {
    return (m_dirs.erase(space_id));
  }

  /** Get the top level directory where this filename was found.
  @param[in]	path		Path to look for.
  @return the top level directory under which this file was found. */
  const std::string &get_root(const std::string &path) const
      MY_ATTRIBUTE((warn_unused_result));

  /** Update the DD if any files were moved to a new location.
  Free the Tablespace_files instance.
  @param[in]	read_only_mode	true if InnoDB is started in
                                  read only mode.
  @return DB_SUCCESS if all OK */
  dberr_t prepare_open_for_business(bool read_only_mode)
      MY_ATTRIBUTE((warn_unused_result));

  /** Flush the redo log writes to disk, possibly cached by the OS. */
  void flush_file_redo();

  /** Flush to disk the writes in file spaces of the given type
  possibly cached by the OS.
  @param[in]	purpose		FIL_TYPE_TABLESPACE or FIL_TYPE_LOG,
                                  can be ORred */
  void flush_file_spaces(uint8_t purpose);

  /** Fetch the fil_space_t instance that maps to the name.
  @param[in]	name		Tablespace name to lookup
  @return tablespace instance or nullptr if not found. */
  fil_space_t *get_space_by_name(const char *name)
      MY_ATTRIBUTE((warn_unused_result)) {
    for (auto shard : m_shards) {
      shard->mutex_acquire();

      auto space = shard->get_space_by_name(name);

      shard->mutex_release();

      if (space != nullptr) {
        return (space);
      }
    }

    return (nullptr);
  }

  /** Check a space ID against the maximum known tablespace ID.
  @param[in]	space_id	Tablespace ID to check
  @return true if it is > than maximum known tablespace ID. */
  bool is_greater_than_max_id(space_id_t space_id) const
      MY_ATTRIBUTE((warn_unused_result)) {
    ut_ad(mutex_owned_all());

    return (space_id > m_max_assigned_id);
  }

  /** Update the maximum known tablespace ID.
  @param[in]	space		Tablespace instance */
  void set_maximum_space_id(const fil_space_t *space) {
    ut_ad(mutex_owned_all());

    if (!m_space_id_reuse_warned) {
      m_space_id_reuse_warned = true;

      ib::warn(ER_IB_MSG_266) << "Allocated tablespace ID " << space->id
                              << " for " << space->name << ", old maximum"
                              << " was " << m_max_assigned_id;
    }

    m_max_assigned_id = space->id;
  }

  /** Update the maximim known space ID if it's smaller than max_id.
  @param[in]	space_id		Value to set if it's greater */
  void update_maximum_space_id(space_id_t space_id) {
    mutex_acquire_all();

    if (is_greater_than_max_id(space_id)) {
      m_max_assigned_id = space_id;
    }

    mutex_release_all();
  }

  /** Assigns a new space id for a new single-table tablespace. This
  works simply by incrementing the global counter. If 4 billion ids
  is not enough, we may need to recycle ids.
  @param[out]	space_id	Set this to the new tablespace ID
  @return true if assigned, false if not */
  bool assign_new_space_id(space_id_t *space_id)
      MY_ATTRIBUTE((warn_unused_result));

  /** Tries to close a file in all the LRU lists.
  The caller must hold the mutex.
  @param[in] print_info		if true, prints information why it
                                  cannot close a file
  @return true if success, false if should retry later */
  bool close_file_in_all_LRU(bool print_info)
      MY_ATTRIBUTE((warn_unused_result));

  /** Opens all log files and system tablespace data files in
  all shards. */
  void open_all_system_tablespaces();

  /** Close all open files in a shard
  @param[in,out]	shard		Close files of this shard */
  void close_files_in_a_shard(Fil_shard *shard);

  /** Close all open files. */
  void close_all_files();

  /** Close all the log files in all shards.
  @param[in]	free_all	If set then free all instances */
  void close_all_log_files(bool free_all);

  /** Iterate through all persistent tablespace files
  (FIL_TYPE_TABLESPACE) returning the nodes via callback function cbk.
  @param[in]	include_log	Include log files, if true
  @param[in]	f		Callback
  @return any error returned by the callback function. */
  dberr_t iterate(bool include_log, Fil_iterator::Function &f)
      MY_ATTRIBUTE((warn_unused_result));

  /** Rotate the tablespace keys by new master key.
  @param[in,out]	shard		Rotate the keys in this shard
  @return true if the re-encrypt succeeds */
  bool encryption_rotate_in_a_shard(Fil_shard *shard);

  /** Rotate the tablespace keys by new master key.
  @return true if the re-encrypt succeeds */
  bool encryption_rotate_all() MY_ATTRIBUTE((warn_unused_result));

  /** Detach a space object from the tablespace memory cache.
  Closes the tablespace files but does not delete them.
  There must not be any pending I/O's or flushes on the files.
  @param[in,out]	space		tablespace */
  void space_detach(fil_space_t *space);

  /** @return the maximum assigned ID so far */
  space_id_t get_max_space_id() const { return (m_max_assigned_id); }

  /** Lookup the tablespace ID.
  @param[in]	space_id	Tablespace ID to lookup
  @return true if the space ID is known. */
  bool lookup_for_recovery(space_id_t space_id)
      MY_ATTRIBUTE((warn_unused_result));

  /** Open a tablespace that has a redo log record to apply.
  @param[in]	space_id		Tablespace ID
  @return true if the open was successful */
  bool open_for_recovery(space_id_t space_id)
      MY_ATTRIBUTE((warn_unused_result));

  /** This function should be called after recovery has completed.
  Check for tablespace files for which we did not see any
  MLOG_FILE_DELETE or MLOG_FILE_RENAME record. These could not
  be recovered.
  @return true if there were some filenames missing for which we had to
          ignore redo log records during the apply phase */
  bool check_missing_tablespaces() MY_ATTRIBUTE((warn_unused_result));

  /** Note that a file has been relocated.
  @param[in]	object_id	Server DD tablespace ID
  @param[in]	space_id	InnoDB tablespace ID
  @param[in]	space_name	Tablespace name
  @param[in]	old_path	Path to the old location
  @param[in]	new_path	Path scanned from disk */
  void moved(dd::Object_id object_id, space_id_t space_id,
             const char *space_name, const std::string &old_path,
             const std::string &new_path) {
    auto tuple =
        std::make_tuple(object_id, space_id, space_name, old_path, new_path);

    m_moved.push_back(tuple);
  }

  /** Check if a path is known to InnoDB.
  @param[in]	path		Path to check
  @return true if path is known to InnoDB */
  bool check_path(const std::string &path) const {
    const auto &dir = m_dirs.contains(path);

    return (dir != Fil_path::null());
  }

  /** Get the list of directories that InnoDB knows about.
  @return the list of directories 'dir1;dir2;....;dirN' */
  std::string get_dirs() const { return (m_dirs.get_dirs()); }

  /** Determines if a file belongs to the least-recently-used list.
  @param[in]	space		Tablespace to check
  @return true if the file belongs to fil_system->m_LRU mutex. */
  static bool space_belongs_in_LRU(const fil_space_t *space)
      MY_ATTRIBUTE((warn_unused_result));

  /** Scan the directories to build the tablespace ID to file name
  mapping table. */
  dberr_t scan(const std::string &directories) {
    return (m_dirs.scan(directories));
  }

  /** Get the tablespace ID from an .ibd and/or an undo tablespace.
  If the ID is == 0 on the first page then check for at least
  MAX_PAGES_TO_CHECK  pages with the same tablespace ID. Do a Light
  weight check before trying with DataFile::find_space_id().
  @param[in]	filename	File name to check
  @return s_invalid_space_id if not found, otherwise the space ID */
  static space_id_t get_tablespace_id(const std::string &filename)
      MY_ATTRIBUTE((warn_unused_result));

  /** Fil_shard by space ID.
  @param[in]	space_id	Tablespace ID
  @return reference to the shard */
  Fil_shard *shard_by_id(space_id_t space_id) const
      MY_ATTRIBUTE((warn_unused_result)) {
#ifndef UNIV_HOTBACKUP
    if (space_id == dict_sys_t_s_log_space_first_id) {
      return (m_shards[REDO_SHARD]);

    } else if (fsp_is_undo_tablespace(space_id)) {
      const size_t limit = space_id % UNDO_SHARDS;

      return (m_shards[UNDO_SHARDS_START + limit]);
    }

    ut_ad(m_shards.size() == MAX_SHARDS);

    return (m_shards[space_id % UNDO_SHARDS_START]);
#else  /* !UNIV_HOTBACKUP */
    ut_ad(m_shards.size() == 1);

    return (m_shards[0]);
#endif /* !UNIV_HOTBACKUP */
  }

  /** Acquire all the mutexes. */
  void mutex_acquire_all() const {
#ifdef UNIV_HOTBACKUP
    ut_ad(m_shards.size() == 1);
#endif /* UNIV_HOTBACKUP */

    for (auto shard : m_shards) {
      shard->mutex_acquire();
    }
  }

  /** Release all the mutexes. */
  void mutex_release_all() const {
#ifdef UNIV_HOTBACKUP
    ut_ad(m_shards.size() == 1);
#endif /* UNIV_HOTBACKUP */

    for (auto shard : m_shards) {
      shard->mutex_release();
    }
  }

#ifdef UNIV_DEBUG

  /** Checks the consistency of the tablespace cache.
  @return true if ok */
  bool validate() const MY_ATTRIBUTE((warn_unused_result));

  /** Check if all mutexes are owned
  @return true if all owned. */
  bool mutex_owned_all() const MY_ATTRIBUTE((warn_unused_result)) {
#ifdef UNIV_HOTBACKUP
    ut_ad(m_shards.size() == 1);
#endif /* UNIV_HOTBACKUP */

    for (const auto shard : m_shards) {
      ut_ad(shard->mutex_owned());
    }

    return (true);
  }

#endif /* UNIV_DEBUG */

  /** Rename a tablespace.  Use the space_id to find the shard.
  @param[in]	space_id	tablespace ID
  @param[in]	old_name	old tablespace name
  @param[in]	new_name	new tablespace name
  @return DB_SUCCESS on success */
  dberr_t rename_tablespace_name(space_id_t space_id, const char *old_name,
                                 const char *new_name)
      MY_ATTRIBUTE((warn_unused_result));

  /** Free the data structures required for recovery. */
  void free_scanned_files() { m_dirs.clear(); }

#ifdef UNIV_HOTBACKUP
  /** Extends all tablespaces to the size stored in the space header.
  During the mysqlbackup --apply-log phase we extended the spaces
  on-demand so that log records could be applied, but that may have
  left spaces still too small compared to the size stored in the space
  header. */
  void meb_extend_tablespaces_to_stored_len() {
    ut_ad(m_shards.size() == 1);

    /* We use a single shard for MEB. */
    auto shard = shard_by_id(SPACE_UNKNOWN);

    shard->mutex_acquire();

    shard->meb_extend_tablespaces_to_stored_len();

    shard->mutex_release();
  }

  /** Process a file name passed as an input
  Wrapper around meb_name_process()
  @param[in,out]	name		absolute path of tablespace file
  @param[in]	space_id	The tablespace ID
  @param[in]	deleted		true if MLOG_FILE_DELETE */
  void meb_name_process(char *name, space_id_t space_id, bool deleted);

#endif /* UNIV_HOTBACKUP */

 private:
  /** Open an ibd tablespace and add it to the InnoDB data structures.
  This is similar to fil_ibd_open() except that it is used while
  processing the redo log, so the data dictionary is not available
  and very little validation is done. The tablespace name is extracted
  from the dbname/tablename.ibd portion of the filename, which assumes
  that the file is a file-per-table tablespace.  Any name will do for
  now.  General tablespace names will be read from the dictionary after
  it has been recovered.  The tablespace flags are read at this time
  from the first page of the file in validate_for_recovery().
  @param[in]	space_id	tablespace ID
  @param[in]	path		path/to/databasename/tablename.ibd
  @param[out]	space		the tablespace, or nullptr on error
  @return status of the operation */
  fil_load_status ibd_open_for_recovery(space_id_t space_id,
                                        const std::string &path,
                                        fil_space_t *&space)
      MY_ATTRIBUTE((warn_unused_result));

 private:
  /** Fil_shards managed */
  Fil_shards m_shards;

  /** n_open is not allowed to exceed this */
  const size_t m_max_n_open;

  /** Maximum space id in the existing tables, or assigned during
  the time mysqld has been up; at an InnoDB startup we scan the
  data dictionary and set here the maximum of the space id's of
  the tables there */
  space_id_t m_max_assigned_id;

  /** true if fil_space_create() has issued a warning about
  potential space_id reuse */
  bool m_space_id_reuse_warned;

  /** List of tablespaces that have been relocated. We need to
  update the DD when it is safe to do so. */
  dd_fil::Tablespaces m_moved;

  /** Tablespace directories scanned at startup */
  Tablespace_dirs m_dirs;

  // Disable copying
  Fil_system(Fil_system &&) = delete;
  Fil_system(const Fil_system &) = delete;
  Fil_system &operator=(const Fil_system &) = delete;

  friend class Fil_shard;
};
