#pragma once

#include <innodb/univ/univ.h>

class PersistentTableMetadata;
struct mtr_t;

/** Interface for persistent dynamic table metadata. */
class Persister {
 public:
  /** Virtual desctructor */
  virtual ~Persister() {}

  /** Write the dynamic metadata of a table, we can pre-calculate
  the size by calling get_write_size()
  @param[in]	metadata	persistent data
  @param[out]	buffer		write buffer
  @param[in]	size		size of write buffer, should be
                                  at least get_write_size()
  @return the length of bytes written */
  virtual ulint write(const PersistentTableMetadata &metadata, byte *buffer,
                      ulint size) const = 0;

  /** Pre-calculate the size of metadata to be written
  @param[in]	metadata	metadata to be written
  @return the size of metadata */
  virtual ulint get_write_size(
      const PersistentTableMetadata &metadata) const = 0;

  /** Read the dynamic metadata from buffer, and store them to
  metadata object
  @param[out]	metadata	metadata where we store the read data
  @param[in]	buffer		buffer to read
  @param[in]	size		size of buffer
  @param[out]	corrupt		true if we found something wrong in
                                  the buffer except incomplete buffer,
                                  otherwise false
  @return the bytes we read from the buffer if the buffer data
  is complete and we get everything, 0 if the buffer is incompleted */
  virtual ulint read(PersistentTableMetadata &metadata, const byte *buffer,
                     ulint size, bool *corrupt) const = 0;

  /** Write MLOG_TABLE_DYNAMIC_META for persistent dynamic
  metadata of table
  @param[in]	id		table id
  @param[in]	metadata	metadata used to write the log
  @param[in,out]	mtr		mini-transaction */
  void write_log(table_id_t id, const PersistentTableMetadata &metadata,
                 mtr_t *mtr) const;
};
