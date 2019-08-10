#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Client_State.h>
#include <innodb/log_arch/Log_Arch_Cbk.h>
#include <innodb/log_sn/lsn_t.h>

class Arch_Group;

/** Redo Log archiver client context */
class Log_Arch_Client_Ctx {
 public:
  /** Constructor: Initialize elements */
  Log_Arch_Client_Ctx()
      : m_state(ARCH_CLIENT_STATE_INIT),
        m_group(nullptr),
        m_begin_lsn(LSN_MAX),
        m_end_lsn(LSN_MAX) {}

  /** Get redo file, header and trailer size
  @param[out]	file_sz		redo file size
  @param[out]	header_sz	redo header size
  @param[out]	trailer_sz	redo trailer size */
  void get_header_size(ib_uint64_t &file_sz, uint &header_sz, uint &trailer_sz);

  /** Start redo log archiving
  @param[out]	header	redo header. Caller must allocate buffer.
  @param[in]	len	buffer length
  @return error code */
  int start(byte *header, uint len);

  /** Stop redo log archiving
  @param[out]	trailer	redo trailer. Caller must allocate buffer.
  @param[in,out]	len	buffer length
  @param[out]	offset	trailer block offset
  @return error code */
  int stop(byte *trailer, uint32_t &len, uint64_t &offset);

  /** Get archived data file details
  @param[in]	cbk_func	callback called for each file
  @param[in]	ctx		callback function context
  @return error code */
  int get_files(Log_Arch_Cbk *cbk_func, void *ctx);

  /** Release archived data so that system can purge it */
  void release();

 private:
  /** Archiver client state */
  Arch_Client_State m_state;

  /** Archive group the client is attached to */
  Arch_Group *m_group;

  /** Start LSN for archived data */
  lsn_t m_begin_lsn;

  /** Stop LSN for archived data */
  lsn_t m_end_lsn;
};
