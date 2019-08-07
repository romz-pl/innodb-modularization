#include <innodb/log_redo/meb_log_print_file_hdr.h>


#ifndef UNIV_HOTBACKUP

#if defined(UNIV_HOTBACKUP) && defined(UNIV_DEBUG)

/** Print a log file header.
@param[in]	block	pointer to the log buffer */
void meb_log_print_file_hdr(byte *block) {
  ib::info(ER_IB_MSG_626) << "Log file header:"
                          << " format "
                          << mach_read_from_4(block + LOG_HEADER_FORMAT)
                          << " pad1 "
                          << mach_read_from_4(block + LOG_HEADER_PAD1)
                          << " start_lsn "
                          << mach_read_from_8(block + LOG_HEADER_START_LSN)
                          << " creator '" << block + LOG_HEADER_CREATOR << "'"
                          << " checksum " << log_block_get_checksum(block);
}
#endif /* UNIV_HOTBACKUP && UNIV_DEBUG */

#endif /* !UNIV_HOTBACKUP */
