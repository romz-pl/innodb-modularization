#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Blk_Flush_Type.h>
#include <innodb/log_arch/Arch_Blk_State.h>
#include <innodb/log_types/lsn_t.h>
#include <innodb/log_arch/Arch_Blk_Type.h>
#include <innodb/log_arch/Arch_Page_Pos.h>
#include <innodb/log_arch/flags.h>
#include <innodb/error/dberr_t.h>

class buf_page_t;
class Arch_Group;

/** In memory data block in Page ID archiving system */
class Arch_Block {
 public:
  /** Constructor: Initialize elements
  @param[in]	blk_buf	buffer for data block
  @param[in]	size	buffer size
  @param[in]	type	block type */
  Arch_Block(byte *blk_buf, uint size, Arch_Blk_Type type)
      : m_data(blk_buf), m_size(size), m_type(type) {}

  /** Do a deep copy of the members of the block passed as the parameter.
  @note This member needs to be updated whenever a new data member is added to
  this class. */
  void copy_data(const Arch_Block *block);

  /** Set the block ready to begin writing page ID
  @param[in]	pos		position to initiate block number */
  void begin_write(Arch_Page_Pos pos);

  /** End writing to a block.
  Change state to #ARCH_BLOCK_READY_TO_FLUSH */
  void end_write();

  /** Check if block is initialised or not.
  @return true if it has been initialised, else false  */
  bool is_init() const { return (m_state == ARCH_BLOCK_INIT); }

  bool is_active() const { return (m_state == ARCH_BLOCK_ACTIVE); }
  /** Check if the block can be flushed or not.
  @return true, if the block cannot be flushed */
  bool is_flushable() const { return (m_state != ARCH_BLOCK_READY_TO_FLUSH); }

  /** Set current block flushed.
  Must hold page archiver sys operation mutex.  */
  void set_flushed() { m_state = ARCH_BLOCK_FLUSHED; }

  /** Add page ID to current block
  @param[in]	page	page from buffer pool
  @param[in]	pos	Archiver current position
  @return true, if successful
          false, if no more space in current block */
  bool add_page(buf_page_t *page, Arch_Page_Pos *pos);

  /* Add reset information to the current reset block.
  @param[in]	reset_lsn	reset lsn info
  @param[in]	reset_pos	reset pos info which needs to be added
  to the current reset block */
  void add_reset(lsn_t reset_lsn, Arch_Page_Pos reset_pos);

  /** Copy page Ids from this block at read position to a buffer.
  @param[in]	read_pos	current read position
  @param[in]	read_len	length of data to copy
  @param[out]	read_buff	buffer to copy page IDs.
                                  Caller must allocate the buffer.
  @return true, if successful
          false, if block is already overwritten */
  bool get_data(Arch_Page_Pos *read_pos, uint read_len, byte *read_buff);

  /** Copy page Ids from a buffer to this block.
  @param[in]	read_len	length of data to copy
  @param[in]	read_buff	buffer to copy page IDs from
  @param[in]	read_offset	offset from where to write
  @return true if successful */
  bool set_data(uint read_len, byte *read_buff, uint read_offset);

  /** Flush this block to the file group
  @param[in]	file_group	current archive group
  @param[in]	type		flush type
  @return error code. */
  dberr_t flush(Arch_Group *file_group, Arch_Blk_Flush_Type type);

  /* Update the block header with the given LSN
  @param[in]	stop_lsn	stop LSN to update in the block header
  @param[in]	reset_lsn	reset LSN to update in the blk header */
  void update_block_header(lsn_t stop_lsn, lsn_t reset_lsn);

  void read(Arch_Group *group, uint64_t offset);

  /** Set the data length of the block.
  @param[in]	data_len	data length */
  void set_data_len(uint data_len) { m_data_len = data_len; }

  /** @return data length of the block. */
  uint get_data_len() const { return (m_data_len); }

  /** @return block number of the block. */
  uint64_t get_number() const { return (m_number); }

  /** @return stop lsn */
  lsn_t get_stop_lsn() const { return (m_stop_lsn); }

  /** Get oldest LSN among the pages that are added to this block
  @return oldest LSN in block pages */
  lsn_t get_oldest_lsn() const { return (m_oldest_lsn); }

  /** Get current state of the block
  @return block state */
  Arch_Blk_State get_state() const { return (m_state); }

  /** Check if the block contains only zeroes.
  @param[in]  block   block data
  @return true if block is filled with zeroes. */
  static bool is_zeroes(const byte *block);

  /** Check if the block data is valid.
  @param[in]  block   block to be validated
  @return true if it's a valid block, else false */
  static bool validate(byte *block);

  /** Get file index of the file the block belongs to.
  @return file index */
  static uint get_file_index(uint64_t block_num);

  /** Get block type from the block header.
  @param[in]     block   block from where to get the type
  @return block type */
  static uint get_type(byte *block);

  /** Get block data length from the block header.
  @param[in]     block   block from where to get the data length
  @return block data length */
  static uint get_data_len(byte *block);

  /** Get the stop lsn stored in the block header.
  @param[in]     block   block from where to fetch the stop lsn
  @return stop lsn */
  static lsn_t get_stop_lsn(byte *block);

  /** Get the block number from the block header.
  @param[in]     block   block from where to fetch the block number
  @return block number */
  static uint64_t get_block_number(byte *block);

  /** Get the reset lsn stored in the block header.
  @param[in]     block   block from where to fetch the reset lsn
  @return reset lsn */
  static lsn_t get_reset_lsn(byte *block);

  /** Get the checksum stored in the block header.
  @param[in]     block   block from where to fetch the checksum
  @return checksum */
  static uint32_t get_checksum(byte *block);

  /** Fetch the offset for a block in the archive file.
  @param[in]	block_num	block number
  @param[in]	type		type of block
  @return file offset of the block */
  static uint64_t get_file_offset(uint64_t block_num, Arch_Blk_Type type);

 private:
  /* @note member function copy_data needs to be updated whenever a new data
  member is added to this class. */

  /** Block data buffer */
  byte *m_data;

  /** Block data length in bytes */
  uint m_data_len{};

  /** Total block size in bytes */
  uint m_size;

  /** State of the block. */
  Arch_Blk_State m_state{ARCH_BLOCK_INIT};

  /** Unique block number */
  uint64_t m_number{};

  /** Type of block. */
  Arch_Blk_Type m_type;

  /** Checkpoint lsn at the time the last page ID was added to the
  block. */
  lsn_t m_stop_lsn{LSN_MAX};

  /** Oldest LSN of all the page IDs added to the block since the last
   * checkpoint */
  lsn_t m_oldest_lsn{LSN_MAX};

  /** Start LSN or the last reset LSN of the group */
  lsn_t m_reset_lsn{LSN_MAX};
};
