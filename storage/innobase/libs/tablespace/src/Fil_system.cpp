#include <innodb/tablespace/Fil_system.h>

#include <innodb/tablespace/recv_recovery_on.h>
#include <innodb/tablespace/Datafile.h>

#include <fstream>

/** Constructor.
@param[in]	n_shards	Number of shards to create
@param[in]	max_open	Maximum number of open files */
Fil_system::Fil_system(size_t n_shards, size_t max_open)
    : m_shards(),
      m_max_n_open(max_open),
      m_max_assigned_id(),
      m_space_id_reuse_warned() {
  ut_ad(Fil_shard::s_open_slot == 0);
  Fil_shard::s_open_slot = EMPTY_OPEN_SLOT;

  for (size_t i = 0; i < n_shards; ++i) {
    auto shard = UT_NEW_NOKEY(Fil_shard(i));

    m_shards.push_back(shard);
  }
}

/** Destructor */
Fil_system::~Fil_system() {
  ut_ad(Fil_shard::s_open_slot == EMPTY_OPEN_SLOT);

  Fil_shard::s_open_slot = 0;

  for (auto shard : m_shards) {
    UT_DELETE(shard);
  }

  m_shards.clear();
}

/** Opens all log files and system tablespace data files in all shards. */
void Fil_system::open_all_system_tablespaces() {
  size_t n_open = 0;

  for (auto shard : m_shards) {
    shard->open_system_tablespaces(m_max_n_open, &n_open);
  }
}

/** Flush the redo log writes to disk, possibly cached by the OS. */
void Fil_system::flush_file_redo() {
    m_shards[REDO_SHARD]->flush_file_redo();
}

/** Flush to disk the writes in file spaces of the given type
possibly cached by the OS.
@param[in]	purpose		FIL_TYPE_TABLESPACE or FIL_TYPE_LOG,
                                can be ORred */
void Fil_system::flush_file_spaces(uint8_t purpose) {
  for (auto shard : m_shards) {
    shard->flush_file_spaces(purpose);
  }
}

/** Rotate the tablespace keys by new master key.
@return true if the re-encrypt succeeds */
bool Fil_system::encryption_rotate_all() {
  for (auto shard : m_shards) {
    // FIXME: We don't acquire the fil_sys::mutex here. Why?

    bool success = encryption_rotate_in_a_shard(shard);

    if (!success) {
      return (false);
    }
  }

  return (true);
}

