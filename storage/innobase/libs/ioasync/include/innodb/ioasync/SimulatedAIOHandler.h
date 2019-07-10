#pragma once

#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/assert/assert.h>
#include <innodb/io/os_file_read_func.h>
#include <innodb/io/os_file_write_func.h>
#include <innodb/io/os_offset_t.h>
#include <innodb/ioasync/AIO.h>
#include <innodb/ioasync/Slot.h>
#include <innodb/sync_event/os_event_t.h>
#include <innodb/time/ut_time.h>
#include <innodb/univ/univ.h>

#include <vector>


/** Simulated AIO handler for reaping IO requests */
class SimulatedAIOHandler {
 public:
  /** Constructor
  @param[in,out]	array	The AIO array
  @param[in]	segment	Local segment in the array */
  SimulatedAIOHandler(AIO *array, ulint segment)
      : m_oldest(),
        m_n_elems(),
        m_lowest_offset(IB_UINT64_MAX),
        m_array(array),
        m_n_slots(),
        m_segment(segment),
        m_ptr(),
        m_buf() {
    ut_ad(m_segment < 100);

    /** In simulated aio, merge at most this many consecutive i/os */
    static const ulint OS_AIO_MERGE_N_CONSECUTIVE = 64;

    m_slots.resize(OS_AIO_MERGE_N_CONSECUTIVE);
  }

  /** Destructor */
  ~SimulatedAIOHandler() {
    if (m_ptr != NULL) {
      ut_free(m_ptr);
    }
  }

  /** Reset the state of the handler
  @param[in]	n_slots	Number of pending AIO operations supported */
  void init(ulint n_slots) {
    m_oldest = 0;
    m_n_elems = 0;
    m_n_slots = n_slots;
    m_lowest_offset = IB_UINT64_MAX;

    if (m_ptr != NULL) {
      ut_free(m_ptr);
      m_ptr = m_buf = NULL;
    }

    m_slots[0] = NULL;
  }

  /** Check if there is a slot for which the i/o has already been done
  @param[out]	n_reserved	Number of reserved slots
  @return the first completed slot that is found. */
  Slot *check_completed(ulint *n_reserved) {
    ulint offset = m_segment * m_n_slots;

    *n_reserved = 0;

    Slot *slot;

    slot = m_array->at(offset);

    for (ulint i = 0; i < m_n_slots; ++i, ++slot) {
      if (slot->is_reserved) {
        if (slot->io_already_done) {
          ut_a(slot->is_reserved);

          return (slot);
        }

        ++*n_reserved;
      }
    }

    return (NULL);
  }

  /** If there are at least 2 seconds old requests, then pick the
  oldest one to prevent starvation.  If several requests have the
  same age, then pick the one at the lowest offset.
  @return true if request was selected */
  bool select() {
    if (!select_oldest()) {
      return (select_lowest_offset());
    }

    return (true);
  }

  /** Check if there are several consecutive blocks
  to read or write. Merge them if found. */
  void merge() {
    /* if m_n_elems != 0, then we have assigned
    something valid to consecutive_ios[0] */
    ut_ad(m_n_elems != 0);
    ut_ad(first_slot() != NULL);

    Slot *slot = first_slot();

    while (!merge_adjacent(slot)) {
      /* No op */
    }
  }

  /** We have now collected n_consecutive I/O requests
  in the array; allocate a single buffer which can hold
  all data, and perform the I/O
  @return the length of the buffer */
  ulint allocate_buffer() MY_ATTRIBUTE((warn_unused_result)) {
    ulint len;
    Slot *slot = first_slot();

    ut_ad(m_ptr == NULL);

    if (slot->type.is_read() && m_n_elems > 1) {
      len = 0;

      for (ulint i = 0; i < m_n_elems; ++i) {
        len += m_slots[i]->len;
      }

      m_ptr = static_cast<byte *>(ut_malloc_nokey(len + UNIV_PAGE_SIZE));

      m_buf = static_cast<byte *>(ut_align(m_ptr, UNIV_PAGE_SIZE));

    } else {
      len = first_slot()->len;
      m_buf = first_slot()->buf;
    }

    return (len);
  }

  /** We have to compress the individual pages and punch
  holes in them on a page by page basis when writing to
  tables that can be compresed at the IO level.
  @param[in]	len		Value returned by allocate_buffer */
  void copy_to_buffer(ulint len) {
    Slot *slot = first_slot();

    if (len > slot->len && slot->type.is_write()) {
      byte *ptr = m_buf;

      ut_ad(ptr != slot->buf);

      /* Copy the buffers to the combined buffer */
      for (ulint i = 0; i < m_n_elems; ++i) {
        slot = m_slots[i];

        memmove(ptr, slot->buf, slot->len);

        ptr += slot->len;
      }
    }
  }

  /** Do the I/O with ordinary, synchronous i/o functions: */
  void io() {
    if (first_slot()->type.is_write()) {
      for (ulint i = 0; i < m_n_elems; ++i) {
        write(m_slots[i]);
      }

    } else {
      for (ulint i = 0; i < m_n_elems; ++i) {
        read(m_slots[i]);
      }
    }
  }

  /** Do the decompression of the pages read in */
  void io_complete() {
    // Note: For non-compressed tables. Not required
    // for correctness.
  }

  /** Mark the i/os done in slots */
  void done() {
    for (ulint i = 0; i < m_n_elems; ++i) {
      m_slots[i]->io_already_done = true;
    }
  }

  /** @return the first slot in the consecutive array */
  Slot *first_slot() MY_ATTRIBUTE((warn_unused_result)) {
    ut_a(m_n_elems > 0);

    return (m_slots[0]);
  }

  /** Wait for I/O requests
  @param[in]	global_segment	The global segment
  @param[in,out]	event		Wait on event if no active requests
  @return the number of slots */
  ulint check_pending(ulint global_segment, os_event_t event)
      MY_ATTRIBUTE((warn_unused_result));

 private:
  /** Do the file read
  @param[in,out]	slot		Slot that has the IO context */
  void read(Slot *slot) {
    dberr_t err = os_file_read_func(slot->type, slot->file.m_file, slot->ptr,
                                    slot->offset, slot->len);
    ut_a(err == DB_SUCCESS);
  }

  /** Do the file write
  @param[in,out]	slot		Slot that has the IO context */
  void write(Slot *slot) {
    dberr_t err = os_file_write_func(slot->type, slot->name, slot->file.m_file,
                                     slot->ptr, slot->offset, slot->len);
    ut_a(err == DB_SUCCESS || err == DB_IO_NO_PUNCH_HOLE);
  }

  /** @return true if the slots are adjacent and can be merged */
  bool adjacent(const Slot *s1, const Slot *s2) const {
    return (s1 != s2 && s1->file.m_file == s2->file.m_file &&
            s2->offset == s1->offset + s1->len && s1->type == s2->type);
  }

  /** @return true if merge limit reached or no adjacent slots found. */
  bool merge_adjacent(Slot *&current) {
    Slot *slot;
    ulint offset = m_segment * m_n_slots;

    slot = m_array->at(offset);

    for (ulint i = 0; i < m_n_slots; ++i, ++slot) {
      if (slot->is_reserved && adjacent(current, slot)) {
        current = slot;

        /* Found a consecutive i/o request */

        m_slots[m_n_elems] = slot;

        ++m_n_elems;

        return (m_n_elems >= m_slots.capacity());
      }
    }

    return (true);
  }

  /** There were no old requests. Look for an I/O request at the lowest
  offset in the array (we ignore the high 32 bits of the offset in these
  heuristics) */
  bool select_lowest_offset() {
    ut_ad(m_n_elems == 0);

    ulint offset = m_segment * m_n_slots;

    m_lowest_offset = IB_UINT64_MAX;

    for (ulint i = 0; i < m_n_slots; ++i) {
      Slot *slot;

      slot = m_array->at(i + offset);

      if (slot->is_reserved && slot->offset < m_lowest_offset) {
        /* Found an i/o request */
        m_slots[0] = slot;

        m_n_elems = 1;

        m_lowest_offset = slot->offset;
      }
    }

    return (m_n_elems > 0);
  }

  /** Select the slot if it is older than the current oldest slot.
  @param[in]	slot		The slot to check */
  void select_if_older(Slot *slot) {
    ulint age;

    age = (ulint)difftime(ut_time(), slot->reservation_time);

    if ((age >= 2 && age > m_oldest) ||
        (age >= 2 && age == m_oldest && slot->offset < m_lowest_offset)) {
      /* Found an i/o request */
      m_slots[0] = slot;

      m_n_elems = 1;

      m_oldest = age;

      m_lowest_offset = slot->offset;
    }
  }

  /** Select th oldest slot in the array
  @return true if oldest slot found */
  bool select_oldest() {
    ut_ad(m_n_elems == 0);

    Slot *slot;
    ulint offset = m_n_slots * m_segment;

    slot = m_array->at(offset);

    for (ulint i = 0; i < m_n_slots; ++i, ++slot) {
      if (slot->is_reserved) {
        select_if_older(slot);
      }
    }

    return (m_n_elems > 0);
  }

  typedef std::vector<Slot *> slots_t;

 private:
  ulint m_oldest;
  ulint m_n_elems;
  os_offset_t m_lowest_offset;

  AIO *m_array;
  ulint m_n_slots;
  ulint m_segment;

  slots_t m_slots;

  byte *m_ptr;
  byte *m_buf;
};
