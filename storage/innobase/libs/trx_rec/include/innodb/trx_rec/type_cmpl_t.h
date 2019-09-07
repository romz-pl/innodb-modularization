#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_rec/flags.h>
#include <innodb/machine/data.h>

/** The type and compilation info flag in the undo record for update.
For easier understanding let the 8 bits be numbered as
7, 6, 5, 4, 3, 2, 1, 0. */
struct type_cmpl_t {
  type_cmpl_t() : m_flag(0) {}

  const byte *read(const byte *ptr) {
    m_flag = mach_read_from_1(ptr);
    return (ptr + 1);
  }

  ulint type_info() {
    /* Get 0-3 */
    return (m_flag & 0x0F);
  }

  ulint cmpl_info() {
    /* Get bits 5 and 4 */
    return ((m_flag >> 4) & 0x03);
  }

  /** Is an LOB updated by this update operation.
  @return true if LOB is updated, false otherwise. */
  bool is_lob_updated() {
    /* Check if bit 7 is set. */
    return (m_flag & TRX_UNDO_UPD_EXTERN);
  }

  /** Does the undo log record contains information about LOB partial
  update vector.
  @return true if undo contains LOB update info. */
  bool is_lob_undo() const {
    /* Check if bit 6 is set. */
    return (m_flag & TRX_UNDO_MODIFY_BLOB);
  }

 private:
  uint8_t m_flag;
};
