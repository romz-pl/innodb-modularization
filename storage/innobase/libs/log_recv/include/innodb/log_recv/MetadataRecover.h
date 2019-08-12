#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <map>

class PersistentTableMetadata;

/** Class to parse persistent dynamic metadata redo log, store and
merge them and apply them to in-memory table objects finally */
class MetadataRecover {
  using PersistentTables = std::map<
      table_id_t, PersistentTableMetadata *, std::less<table_id_t>,
      ut_allocator<std::pair<const table_id_t, PersistentTableMetadata *>>>;

 public:
  /** Default constructor */
  MetadataRecover() UNIV_NOTHROW {}

  /** Destructor */
  ~MetadataRecover();

  /** Parse a dynamic metadata redo log of a table and store
  the metadata locally
  @param[in]	id		table id
  @param[in]	version		table dynamic metadata version
  @param[in]	ptr		redo log start
  @param[in]	end		end of redo log
  @retval ptr to next redo log record, NULL if this log record
  was truncated */
  byte *parseMetadataLog(table_id_t id, uint64_t version, byte *ptr, byte *end);

  /** Apply the collected persistent dynamic metadata to in-memory
  table objects */
  void apply();

  /** Store the collected persistent dynamic metadata to
  mysql.innodb_dynamic_metadata */
  void store();

  /** If there is any metadata to be applied
  @return	true if any metadata to be applied, otherwise false */
  bool empty() const { return (m_tables.empty()); }

 private:
  /** Get the dynamic metadata of a specified table,
  create a new one if not exist
  @param[in]	id	table id
  @return the metadata of the specified table */
  PersistentTableMetadata *getMetadata(table_id_t id);

 private:
  /** Map used to store and merge persistent dynamic metadata */
  PersistentTables m_tables;
};
