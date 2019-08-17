#include <innodb/page/page_zip_decompress_heap_no.h>

#include <innodb/machine/data.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/record/flag.h>

/** Set the heap_no in a record, and skip the fixed-size record header
 that is not included in the d_stream.
 @return true on success, false if d_stream does not end at rec */
ibool page_zip_decompress_heap_no(
    z_stream *d_stream, /*!< in/out: compressed page stream */
    rec_t *rec,         /*!< in/out: record */
    ulint &heap_status) /*!< in/out: heap_no and status bits */
{
  if (d_stream->next_out != rec - REC_N_NEW_EXTRA_BYTES) {
    /* n_dense has grown since the page was last compressed. */
    return (FALSE);
  }

  /* Skip the REC_N_NEW_EXTRA_BYTES. */
  d_stream->next_out = rec;

  /* Set heap_no and the status bits. */
  mach_write_to_2(rec - REC_NEW_HEAP_NO, heap_status);
  heap_status += 1 << REC_HEAP_NO_SHIFT;

  /* Clear the info bits, to make sure later assertion saying
  that this record is not instant can pass in rec_init_offsets() */
  rec[-REC_N_NEW_EXTRA_BYTES] = 0;

  return (TRUE);
}
