#include <innodb/trx_roll/trx_roll_check_undo_rec_ordering.h>

#ifdef UNIV_DEBUG
/** Check if undo numbering is maintained while processing undo records
 for rollback.
 @return true if undo numbering is maintained. */
UNIV_INLINE
bool trx_roll_check_undo_rec_ordering(
    undo_no_t curr_undo_rec_no,    /*!< in: record number of
                                   undo record to process. */
    space_id_t curr_undo_space_id, /*!< in: space-id of rollback
                                   segment that contains the
                                   undo record to process. */
    const trx_t *trx)              /*!< in: transaction */
{
  /* Each transaction now can have multiple rollback segments.
  If a transaction involves temp and non-temp tables, both the rollback
  segments will be active. In this case undo records will be distrubuted
  across the two rollback segments.
  CASE-1: UNDO action will apply all undo records from one rollback
  segment before moving to next. This means undo record numbers can't be
  sequential but ordering is still enforced as next undo record number
  should be < processed undo record number.
  CASE-2: For normal rollback (not initiated by crash) all rollback
  segments will be active (including non-redo).
  Based on transaction operation pattern undo record number of first
  undo record from this new rollback segment can be > last undo number
  from previous rollback segment and so we ignore this check if
  rollback segments are switching. Once switched new rollback segment
  should re-follow undo record number pattern (as mentioned in CASE-1). */

  return (curr_undo_space_id != trx->undo_rseg_space ||
          curr_undo_rec_no + 1 <= trx->undo_no);
}
#endif /* UNIV_DEBUG */
