#pragma once

#include <innodb/univ/univ.h>

#include <innodb/math/ut_2_exp.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/assert/assert.h>

#include "mysql/psi/mysql_stage.h"

/** Class used to report CLONE progress via Performance Schema. */
class Clone_Monitor {
 public:
  /** Constructor */
  Clone_Monitor()
      : m_estimate(),
        m_work_done(),
        m_progress(),
        m_estimate_bytes_left(),
        m_work_bytes_left(),
        m_cur_phase(NOT_STARTED) {
    m_pfs_data_chunk_size = ut_2_exp(PFS_DATA_CHUNK_SIZE_POW2);
  }

  /** Destructor. */
  ~Clone_Monitor() {
    if (m_progress == nullptr) {
      return;
    }

    mysql_end_stage();
  }

  /** Initialize all monitoring data.
  @param[in]	key	PFS key to register stage event
  @param[in]	enable  if true, enable PFS trackig. */
  void init_state(PSI_stage_key key, bool enable) {
    change_phase();
    m_progress = nullptr;
    m_estimate = 0;
    m_work_done = 0;
    m_estimate_bytes_left = 0;
    m_work_bytes_left = 0;

    if (enable && key != PSI_NOT_INSTRUMENTED) {
      m_progress = mysql_set_stage(key);
    }

    if (m_progress == nullptr) {
      m_cur_phase = NOT_STARTED;
      return;
    }

    m_cur_phase = ESTIMATE_WORK;
  }

  /** @return true if in estimation phase */
  bool is_estimation_phase() const { return (m_cur_phase == ESTIMATE_WORK); }

  /** @return estimated work in bytes */
  uint64_t get_estimate() {
    uint64_t ret_estimate = 0;
    if (m_estimate > 0) {
      ret_estimate = m_estimate << PFS_DATA_CHUNK_SIZE_POW2;
    }
    ret_estimate += m_estimate_bytes_left;
    return (ret_estimate);
  }

  /** Update the work estimated for the clone operation.
  @param[in]	size	size in bytes that needs to transferred
  across. */
  void add_estimate(uint64_t size) {
    m_estimate += convert_bytes_to_work(size, true);

    if (m_cur_phase == NOT_STARTED) {
      return;
    }

    ut_ad(m_cur_phase == ESTIMATE_WORK);
    ut_ad(m_progress != nullptr);

    mysql_stage_set_work_estimated(m_progress, m_estimate);
  }

  /** Update the progress of the clone operation.
  param[in]	size	size in bytes that is being transferred
  across. */
  void update_work(uint size) {
    if (m_cur_phase == NOT_STARTED) {
      return;
    }

    ut_ad(m_progress != nullptr);
    ut_ad(m_cur_phase == COMPLETE_WORK);

    m_work_done += convert_bytes_to_work(size, false);
    mysql_stage_set_work_completed(m_progress, m_work_done);
  }

  /** Change from one phase to the other. */
  void change_phase() {
    switch (m_cur_phase) {
      case NOT_STARTED:
        return;

      case ESTIMATE_WORK:
        if (m_estimate_bytes_left != 0) {
          m_estimate_bytes_left = 0;
          m_estimate++;
          mysql_stage_set_work_estimated(m_progress, m_estimate);
        }

        m_cur_phase = COMPLETE_WORK;
        break;

      case COMPLETE_WORK:
        if (m_work_bytes_left != 0) {
          m_work_bytes_left = 0;
          if (m_work_done != m_estimate) {
            m_work_done++;
          }
          mysql_stage_set_work_completed(m_progress, m_work_done);
        }

        m_cur_phase = NOT_STARTED;
        break;
    }
  }

 private:
  /** Translate bytes to  work unit.
  @param[in]	size	size in bytes that needs to be converted to the
  @param[in]	is_estimate	if called during estimation
  corresponding work unit.
  @return the number of PFS chunks that the size constitutes. */
  uint64_t convert_bytes_to_work(uint64_t size, bool is_estimate) {
    auto &bytes_left = is_estimate ? m_estimate_bytes_left : m_work_bytes_left;
    size += bytes_left;

    auto aligned_size = ut_uint64_align_down(size, m_pfs_data_chunk_size);
    bytes_left = size - aligned_size;

    return (aligned_size >> PFS_DATA_CHUNK_SIZE_POW2);
  }

  /* Number of PFS chunks which needs to be transferred across.  */
  uint64_t m_estimate;

  /* Number of PFS chunks already transferred. */
  uint64_t m_work_done;

  /* Performance schema accounting object. */
  PSI_stage_progress *m_progress;

  /* Size in bytes which couldn't fit the chunk during estimation. */
  uint64_t m_estimate_bytes_left;

  /* Size in bytes which couldn't fit the chunk during transfer. */
  uint64_t m_work_bytes_left;

  /* Current phase. */
  enum { NOT_STARTED = 0, ESTIMATE_WORK, COMPLETE_WORK } m_cur_phase;

  /* PFS Chunk size in power of 2 in unit of bytes. */
  static const int PFS_DATA_CHUNK_SIZE_POW2 = 20;

  /* PFS chunk size. */
  uint m_pfs_data_chunk_size;
};
