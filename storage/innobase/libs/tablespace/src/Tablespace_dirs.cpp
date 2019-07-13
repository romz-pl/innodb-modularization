#include <innodb/tablespace/Tablespace_dirs.h>

#include <innodb/tablespace/Fil_system.h>
#include <innodb/logger/info.h>
#include <innodb/time/ut_time.h>
#include <innodb/tablespace/Dirs.h>
#include <innodb/io/Dir_Walker.h>


/* Implemented in fil/fil0Tablespace_dirs.cpp */

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


