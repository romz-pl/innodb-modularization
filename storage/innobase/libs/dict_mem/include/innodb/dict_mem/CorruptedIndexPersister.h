#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/Persister.h>

class PersistentTableMetadata;

/** Persister used for corrupted indexes */
class CorruptedIndexPersister : public Persister {
 public:
  /** Write the corrupted indexes of a table, we can pre-calculate
  the size by calling get_write_size()
  @param[in]	metadata	persistent metadata
  @param[out]	buffer		write buffer
  @param[in]	size		size of write buffer, should be
                                  at least get_write_size()
  @return the length of bytes written */
  ulint write(const PersistentTableMetadata &metadata, byte *buffer,
              ulint size) const;

  /** Pre-calculate the size of metadata to be written
  @param[in]	metadata	metadata to be written
  @return the size of metadata */
  ulint get_write_size(const PersistentTableMetadata &metadata) const;

  /** Read the corrupted indexes from buffer, and store them to
  metadata object
  @param[out]	metadata	metadata where we store the read data
  @param[in]	buffer		buffer to read
  @param[in]	size		size of buffer
  @param[out]	corrupt		true if we found something wrong in
                                  the buffer except incomplete buffer,
                                  otherwise false
  @return the bytes we read from the buffer if the buffer data
  is complete and we get everything, 0 if the buffer is incomplete */
  ulint read(PersistentTableMetadata &metadata, const byte *buffer, ulint size,
             bool *corrupt) const;

 private:
  /** The length of index_id_t we will write */
  static const size_t INDEX_ID_LENGTH = 12;
};
