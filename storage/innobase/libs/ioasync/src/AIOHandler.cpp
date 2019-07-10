#include <innodb/ioasync/AIOHandler.h>

#include <innodb/io/os_free_block.h>
#include <innodb/allocator/ut_free.h>

/** Check if we need to read some more data.
@param[in]	slot		The slot that contains the IO request
@param[in]	n_bytes		Total bytes read so far
@return DB_SUCCESS or error code */
dberr_t AIOHandler::check_read(Slot *slot, ulint n_bytes) {
  dberr_t err;

  ut_ad(slot->type.is_read());
  ut_ad(slot->original_len > slot->len);

  if (is_compressed_page(slot)) {
    if (can_decompress(slot)) {
      ut_a(slot->offset > 0);

      slot->len = slot->original_len;
#ifdef _WIN32
      slot->n_bytes = static_cast<DWORD>(n_bytes);
#else
      slot->n_bytes = static_cast<ulint>(n_bytes);
#endif /* _WIN32 */

      err = io_complete(slot);
      ut_a(err == DB_SUCCESS);
    } else {
      /* Read the next block in */
      ut_ad(compressed_page_size(slot) >= n_bytes);

      err = DB_FAIL;
    }
  } else if (is_encrypted_page(slot) ||
             (slot->type.is_log() && slot->offset >= LOG_FILE_HDR_SIZE)) {
    ut_a(slot->offset > 0);

    slot->len = slot->original_len;
#ifdef _WIN32
    slot->n_bytes = static_cast<DWORD>(n_bytes);
#else
    slot->n_bytes = static_cast<ulint>(n_bytes);
#endif /* _WIN32 */

    err = io_complete(slot);
    ut_a(err == DB_SUCCESS);

  } else {
    err = DB_FAIL;
  }

  if (slot->buf_block != NULL) {
    os_free_block(slot->buf_block);
    slot->buf_block = NULL;
  }

  if (slot->encrypt_log_buf != NULL) {
    ut_free(slot->encrypt_log_buf);
    slot->encrypt_log_buf = NULL;
  }

  return (err);
}


/** Do any post processing after a read/write
@return DB_SUCCESS or error code. */
dberr_t AIOHandler::post_io_processing(Slot *slot) {
  dberr_t err;

  ut_ad(slot->is_reserved);

  /* Total bytes read so far */
  ulint n_bytes = (slot->ptr - slot->buf) + slot->n_bytes;

  /* Compressed writes can be smaller than the original length.
  Therefore they can be processed without further IO. */
  if (n_bytes == slot->original_len ||
      (slot->type.is_write() && slot->type.is_compressed() &&
       slot->len == static_cast<ulint>(slot->n_bytes))) {
    if ((slot->type.is_log() && slot->offset >= LOG_FILE_HDR_SIZE) ||
        is_compressed_page(slot) || is_encrypted_page(slot)) {
      ut_a(slot->offset > 0);

      if (slot->type.is_read()) {
        slot->len = slot->original_len;
      }

      /* The punch hole has been done on collect() */

      if (slot->type.is_read()) {
        err = io_complete(slot);
      } else {
        err = DB_SUCCESS;
      }

      ut_ad(err == DB_SUCCESS || err == DB_UNSUPPORTED ||
            err == DB_CORRUPTION || err == DB_IO_DECOMPRESS_FAIL);
    } else {
      err = DB_SUCCESS;
    }

    if (slot->buf_block != NULL) {
      os_free_block(slot->buf_block);
      slot->buf_block = NULL;
    }

    if (slot->encrypt_log_buf != NULL) {
      ut_free(slot->encrypt_log_buf);
      slot->encrypt_log_buf = NULL;
    }
  } else if ((ulint)slot->n_bytes == (ulint)slot->len) {
    /* It *must* be a partial read. */
    ut_ad(slot->len < slot->original_len);

    /* Has to be a read request, if it is less than
    the original length. */
    ut_ad(slot->type.is_read());
    err = check_read(slot, n_bytes);

  } else {
    err = DB_FAIL;
  }

  return (err);
}

