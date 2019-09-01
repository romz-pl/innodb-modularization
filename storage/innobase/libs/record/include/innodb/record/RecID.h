#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_priv/lock_t.h>
#include <innodb/buf_block/buf_block_t.h>
#include <innodb/lock_priv/lock_rec_fold.h>
#include <innodb/page/flag.h>

/**
Record lock ID */
struct RecID {
  /** Constructor
  @param[in]	lock		Record lock
  @param[in]	heap_no		Heap number in the page */
  RecID(const lock_t *lock, ulint heap_no)
      : m_space_id(lock->rec_lock.space),
        m_page_no(lock->rec_lock.page_no),
        m_heap_no(static_cast<uint32_t>(heap_no)),
        m_fold(lock_rec_fold(m_space_id, m_page_no)) {
    ut_ad(m_space_id < UINT32_MAX);
    ut_ad(m_page_no < UINT32_MAX);
    ut_ad(m_heap_no < UINT32_MAX);
  }

  /** Constructor
  @param[in]	space_id	Tablespace ID
  @param[in]	page_no		Page number in space_id
  @param[in]	heap_no		Heap number in <space_id, page_no> */
  RecID(space_id_t space_id, page_no_t page_no, ulint heap_no)
      : m_space_id(space_id),
        m_page_no(page_no),
        m_heap_no(static_cast<uint32_t>(heap_no)),
        m_fold(lock_rec_fold(m_space_id, m_page_no)) {
    ut_ad(m_space_id < UINT32_MAX);
    ut_ad(m_page_no < UINT32_MAX);
    ut_ad(m_heap_no < UINT32_MAX);
  }

  /** Constructor
  @param[in]	block		Block in a tablespace
  @param[in]	heap_no		Heap number in the block */
  RecID(const buf_block_t *block, ulint heap_no)
      : m_space_id(block->page.id.space()),
        m_page_no(block->page.id.page_no()),
        m_heap_no(static_cast<uint32_t>(heap_no)),
        m_fold(lock_rec_fold(m_space_id, m_page_no)) {
    ut_ad(heap_no < UINT32_MAX);
  }

  /**
  @return the "folded" value of {space, page_no} */
  ulint fold() const { return (m_fold); }

  /** @return true if it's the supremum record */
  bool is_supremum() const { return (m_heap_no == PAGE_HEAP_NO_SUPREMUM); }

  /* Check if the rec id matches the lock instance.
  @param[i]	lock		Lock to compare with
  @return true if <space, page_no, heap_no> matches the lock. */
  inline bool matches(const lock_t *lock) const;

  /**
  Tablespace ID */
  space_id_t m_space_id;

  /**
  Page number within the space ID */
  page_no_t m_page_no;

  /**
  Heap number within the page */
  uint32_t m_heap_no;

  /**
  Hashed key value */
  ulint m_fold;
};
