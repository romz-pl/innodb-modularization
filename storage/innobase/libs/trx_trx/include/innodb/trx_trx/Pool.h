#pragma once

#include <innodb/univ/univ.h>

#include <innodb/assert/assert.h>
#include <innodb/allocator/ut_allocator.h>

#include <queue>
#include <vector>

/** Allocate the memory for the object in blocks. We keep the objects sorted
on pointer so that they are closer together in case they have to be iterated
over in a list. */
template <typename Type, typename Factory, typename LockStrategy>
struct Pool {
  typedef Type value_type;

  // FIXME: Add an assertion to check alignment and offset is
  // as we expect it. Also, sizeof(void*) can be 8, can we impove on this.
  struct Element {
    Pool *m_pool;
    value_type m_type;
  };

  /** Constructor
  @param size size of the memory block */
  Pool(size_t size) : m_end(), m_start(), m_size(size), m_last() {
    ut_a(size >= sizeof(Element));

    m_lock_strategy.create();

    ut_a(m_start == 0);

    m_start = reinterpret_cast<Element *>(ut_zalloc_nokey(m_size));

    m_last = m_start;

    m_end = &m_start[m_size / sizeof(*m_start)];

    /* Note: Initialise only a small subset, even though we have
    allocated all the memory. This is required only because PFS
    (MTR) results change if we instantiate too many mutexes up
    front. */

    init(std::min(size_t(16), size_t(m_end - m_start)));

    ut_ad(m_pqueue.size() <= size_t(m_last - m_start));
  }

  /** Destructor */
  ~Pool() {
    m_lock_strategy.destroy();

    for (Element *elem = m_start; elem != m_last; ++elem) {
      ut_ad(elem->m_pool == this);
      Factory::destroy(&elem->m_type);
    }

    ut_free(m_start);
    m_end = m_last = m_start = 0;
    m_size = 0;
  }

  /** Get an object from the pool.
  @return a free instance or NULL if exhausted. */
  Type *get() {
    Element *elem;

    m_lock_strategy.enter();

    if (!m_pqueue.empty()) {
      elem = m_pqueue.top();
      m_pqueue.pop();

    } else if (m_last < m_end) {
      /* Initialise the remaining elements. */
      init(m_end - m_last);

      ut_ad(!m_pqueue.empty());

      elem = m_pqueue.top();
      m_pqueue.pop();
    } else {
      elem = NULL;
    }

    m_lock_strategy.exit();

    return (elem != NULL ? &elem->m_type : 0);
  }

  /** Add the object to the pool.
  @param ptr object to free */
  static void mem_free(value_type *ptr) {
    Element *elem;
    byte *p = reinterpret_cast<byte *>(ptr + 1);

    elem = reinterpret_cast<Element *>(p - sizeof(*elem));

    elem->m_pool->put(elem);
  }

 protected:
  // Disable copying
  Pool(const Pool &);
  Pool &operator=(const Pool &);

 private:
  /* We only need to compare on pointer address. */
  typedef std::priority_queue<Element *,
                              std::vector<Element *, ut_allocator<Element *>>,
                              std::greater<Element *>>
      pqueue_t;

  /** Release the object to the free pool
  @param elem element to free */
  void put(Element *elem) {
    m_lock_strategy.enter();

    ut_ad(elem >= m_start && elem < m_last);

    ut_ad(Factory::debug(&elem->m_type));

    m_pqueue.push(elem);

    m_lock_strategy.exit();
  }

  /** Initialise the elements.
  @param n_elems Number of elements to initialise */
  void init(size_t n_elems) {
    ut_ad(size_t(m_end - m_last) >= n_elems);

    for (size_t i = 0; i < n_elems; ++i, ++m_last) {
      m_last->m_pool = this;
      Factory::init(&m_last->m_type);
      m_pqueue.push(m_last);
    }

    ut_ad(m_last <= m_end);
  }

 private:
  /** Pointer to the last element */
  Element *m_end;

  /** Pointer to the first element */
  Element *m_start;

  /** Size of the block in bytes */
  size_t m_size;

  /** Upper limit of used space */
  Element *m_last;

  /** Priority queue ordered on the pointer addresse. */
  pqueue_t m_pqueue;

  /** Lock strategy to use */
  LockStrategy m_lock_strategy;
};
