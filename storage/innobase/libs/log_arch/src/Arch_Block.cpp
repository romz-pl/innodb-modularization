#include <innodb/log_arch/Arch_Block.h>

#include <innodb/log_arch/flags.h>
#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>
#include <innodb/crc32/crc32.h>
#include <innodb/logger/warn.h>
#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/univ/page_no_t.h>
#include <innodb/buf_page/buf_page_t.h>

uint Arch_Block::get_file_index(uint64_t block_num) {
  return (block_num / ARCH_PAGE_FILE_DATA_CAPACITY);
}

uint Arch_Block::get_type(byte *block) {
  return (mach_read_from_1(block + ARCH_PAGE_BLK_HEADER_TYPE_OFFSET));
}

uint Arch_Block::get_data_len(byte *block) {
  return (mach_read_from_2(block + ARCH_PAGE_BLK_HEADER_DATA_LEN_OFFSET));
}

lsn_t Arch_Block::get_stop_lsn(byte *block) {
  return (mach_read_from_8(block + ARCH_PAGE_BLK_HEADER_STOP_LSN_OFFSET));
}

uint64_t Arch_Block::get_block_number(byte *block) {
  return (mach_read_from_8(block + ARCH_PAGE_BLK_HEADER_NUMBER_OFFSET));
}

lsn_t Arch_Block::get_reset_lsn(byte *block) {
  return (mach_read_from_8(block + ARCH_PAGE_BLK_HEADER_RESET_LSN_OFFSET));
}

uint32_t Arch_Block::get_checksum(byte *block) {
  return (mach_read_from_4(block + ARCH_PAGE_BLK_HEADER_CHECKSUM_OFFSET));
}

uint64_t Arch_Block::get_file_offset(uint64_t block_num, Arch_Blk_Type type) {
  uint64_t offset = 0;

  switch (type) {
    case ARCH_RESET_BLOCK:
      offset = 0;
      break;

    case ARCH_DATA_BLOCK:
      offset = block_num % ARCH_PAGE_FILE_DATA_CAPACITY;
      offset += ARCH_PAGE_FILE_NUM_RESET_PAGE;
      offset *= ARCH_PAGE_BLK_SIZE;
      break;

    default:
      ut_ad(0);
  }

  return (offset);
}

bool Arch_Block::is_zeroes(const byte *block) {
  for (ulint i = 0; i < ARCH_PAGE_BLK_SIZE; i++) {
    if (block[i] != 0) {
      return (false);
    }
  }
  return (true);
}

bool Arch_Block::validate(byte *block) {
  auto data_length = Arch_Block::get_data_len(block);
  auto block_checksum = Arch_Block::get_checksum(block);
  auto checksum = ut_crc32(block + ARCH_PAGE_BLK_HEADER_LENGTH, data_length);

  if (checksum != block_checksum) {
    ut_ad(0);
    ib::warn() << "Page archiver's doublewrite buffer for "
               << "block " << Arch_Block::get_block_number(block)
               << " is not valid.";
    return (false);
  } else if (Arch_Block::is_zeroes(block)) {
    return (false);
  }

  return (true);
}

void Arch_Block::update_block_header(lsn_t stop_lsn, lsn_t reset_lsn) {
  mach_write_to_2(m_data + ARCH_PAGE_BLK_HEADER_DATA_LEN_OFFSET, m_data_len);

  if (stop_lsn != LSN_MAX) {
    m_stop_lsn = stop_lsn;
    mach_write_to_8(m_data + ARCH_PAGE_BLK_HEADER_STOP_LSN_OFFSET, m_stop_lsn);
  }

  if (reset_lsn != LSN_MAX) {
    m_reset_lsn = reset_lsn;
    mach_write_to_8(m_data + ARCH_PAGE_BLK_HEADER_RESET_LSN_OFFSET,
                    m_reset_lsn);
  }
}

/** Set the block ready to begin writing page ID
@param[in]	pos	position to initiate block number */
void Arch_Block::begin_write(Arch_Page_Pos pos) {
  m_data_len = 0;

  m_state = ARCH_BLOCK_ACTIVE;

  m_number =
      (m_type == ARCH_DATA_BLOCK ? pos.m_block_num
                                 : Arch_Block::get_file_index(pos.m_block_num));

  m_oldest_lsn = LSN_MAX;
  m_reset_lsn = LSN_MAX;

  if (m_type == ARCH_DATA_BLOCK) {
    arch_page_sys->update_stop_info(this);
  }
}

/** End writing to a block.
Change state to #ARCH_BLOCK_READY_TO_FLUSH */
void Arch_Block::end_write() { m_state = ARCH_BLOCK_READY_TO_FLUSH; }

/** Add page ID to current block
@param[in]	page	page from buffer pool
@param[in]	pos	Archiver current position
@return true, if successful
        false, if no more space in current block */
bool Arch_Block::add_page(buf_page_t *page, Arch_Page_Pos *pos) {
  space_id_t space_id;
  page_no_t page_num;
  byte *data_ptr;

  ut_ad(pos->m_offset <= ARCH_PAGE_BLK_SIZE);
  ut_ad(m_type == ARCH_DATA_BLOCK);
  ut_ad(pos->m_offset == m_data_len + ARCH_PAGE_BLK_HEADER_LENGTH);

  if ((pos->m_offset + ARCH_BLK_PAGE_ID_SIZE) > ARCH_PAGE_BLK_SIZE) {
    ut_ad(pos->m_offset == ARCH_PAGE_BLK_SIZE);
    return (false);
  }

  data_ptr = m_data + pos->m_offset;

  /* Write serialized page ID: tablespace ID and offset */
  space_id = page->id.space();
  page_num = page->id.page_no();

  mach_write_to_4(data_ptr + ARCH_BLK_SPCE_ID_OFFSET, space_id);
  mach_write_to_4(data_ptr + ARCH_BLK_PAGE_NO_OFFSET, page_num);

  /* Update position. */
  pos->m_offset += ARCH_BLK_PAGE_ID_SIZE;
  m_data_len += ARCH_BLK_PAGE_ID_SIZE;

  /* Update oldest LSN from page. */
  if (arch_page_sys->get_latest_stop_lsn() > m_oldest_lsn ||
      m_oldest_lsn > page->oldest_modification) {
    m_oldest_lsn = page->oldest_modification;
  }

  return (true);
}

bool Arch_Block::get_data(Arch_Page_Pos *read_pos, uint read_len,
                          byte *read_buff) {
  ut_ad(read_pos->m_offset + read_len <= m_size);

  if (m_state == ARCH_BLOCK_INIT || m_number != read_pos->m_block_num) {
    /* The block is already overwritten. */
    return (false);
  }

  byte *src = m_data + read_pos->m_offset;

  memcpy(read_buff, src, read_len);

  return (true);
}

bool Arch_Block::set_data(uint read_len, byte *read_buff, uint read_offset) {
  ut_ad(m_state != ARCH_BLOCK_INIT);
  ut_ad(read_offset + read_len <= m_size);

  byte *dest = m_data + read_offset;

  memcpy(dest, read_buff, read_len);

  return (true);
}

/** Flush this block to the file group.
@param[in]	file_group	current archive group
@param[in]	type		flush type
@return error code. */
dberr_t Arch_Block::flush(Arch_Group *file_group, Arch_Blk_Flush_Type type) {
  dberr_t err = DB_SUCCESS;
  uint32_t checksum;

  checksum = ut_crc32(m_data + ARCH_PAGE_BLK_HEADER_LENGTH, m_data_len);

  /* Update block header. */
  mach_write_to_1(m_data + ARCH_PAGE_BLK_HEADER_VERSION_OFFSET,
                  ARCH_PAGE_FILE_VERSION);
  mach_write_to_1(m_data + ARCH_PAGE_BLK_HEADER_TYPE_OFFSET, m_type);
  mach_write_to_2(m_data + ARCH_PAGE_BLK_HEADER_DATA_LEN_OFFSET, m_data_len);
  mach_write_to_4(m_data + ARCH_PAGE_BLK_HEADER_CHECKSUM_OFFSET, checksum);
  mach_write_to_8(m_data + ARCH_PAGE_BLK_HEADER_STOP_LSN_OFFSET, m_stop_lsn);
  mach_write_to_8(m_data + ARCH_PAGE_BLK_HEADER_RESET_LSN_OFFSET, m_reset_lsn);
  mach_write_to_8(m_data + ARCH_PAGE_BLK_HEADER_NUMBER_OFFSET, m_number);

  switch (m_type) {
    case ARCH_RESET_BLOCK:
      err = file_group->write_file_header(m_data, m_size);
      break;

    case ARCH_DATA_BLOCK: {
      bool is_partial_flush = (type == ARCH_FLUSH_PARTIAL);

      /** We allow partial flush to happen even if there were no pages added
      since the last partial flush as the header might contain some useful
      info required during recovery. */
      err = file_group->write_to_file(nullptr, m_data, m_size, is_partial_flush,
                                      true);
      break;
    }

    default:
      ut_ad(false);
  }

  return (err);
}

void Arch_Block::add_reset(lsn_t reset_lsn, Arch_Page_Pos reset_pos) {
  ut_ad(m_type == ARCH_RESET_BLOCK);
  ut_ad(m_data_len <= ARCH_PAGE_BLK_SIZE);
  ut_ad(m_data_len + ARCH_PAGE_FILE_HEADER_RESET_POS_SIZE <=
        ARCH_PAGE_BLK_SIZE);

  byte *buf = m_data + ARCH_PAGE_BLK_HEADER_LENGTH;

  if (m_data_len == 0) {
    /* Write file lsn. */

    mach_write_to_8(buf, reset_lsn);
    m_data_len += ARCH_PAGE_FILE_HEADER_RESET_LSN_SIZE;
  }

  ut_ad(m_data_len >= ARCH_PAGE_FILE_HEADER_RESET_LSN_SIZE);

  mach_write_to_2(buf + m_data_len, reset_pos.m_block_num);
  m_data_len += ARCH_PAGE_FILE_HEADER_RESET_BLOCK_NUM_SIZE;

  mach_write_to_2(buf + m_data_len, reset_pos.m_offset);
  m_data_len += ARCH_PAGE_FILE_HEADER_RESET_BLOCK_OFFSET_SIZE;
}

void Arch_Block::copy_data(const Arch_Block *block) {
  m_data_len = block->m_data_len;
  m_size = block->m_size;
  m_state = block->m_state;
  m_number = block->m_number;
  m_type = block->m_type;
  m_stop_lsn = block->m_stop_lsn;
  m_reset_lsn = block->m_reset_lsn;
  m_oldest_lsn = block->m_oldest_lsn;
  set_data(m_size, block->m_data, 0);
}

void Arch_Block::read(Arch_Group *group, uint64_t offset) {
  switch (m_type) {
    case ARCH_DATA_BLOCK:
      group->recovery_read_latest_blocks(m_data, offset, ARCH_DATA_BLOCK);
      m_reset_lsn = Arch_Block::get_reset_lsn(m_data);
      break;

    case ARCH_RESET_BLOCK:
      group->recovery_read_latest_blocks(m_data, offset, ARCH_RESET_BLOCK);
      break;
  }
}
