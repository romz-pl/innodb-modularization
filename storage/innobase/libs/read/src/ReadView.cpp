#include <innodb/read/ReadView.h>

#include <innodb/allocator/UT_NEW_ARRAY_NOKEY.h>
#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_sys/trx_sys.h>

/** Minimum number of elements to reserve in ReadView::ids_t */
static const ulint MIN_TRX_IDS = 32;

/**
Try and increase the size of the array. Old elements are
copied across.
@param n 		Make space for n elements */
void ReadView::ids_t::reserve(ulint n) {
  if (n <= capacity()) {
    return;
  }

  /** Keep a minimum threshold */
  if (n < MIN_TRX_IDS) {
    n = MIN_TRX_IDS;
  }

  value_type *p = m_ptr;

  m_ptr = UT_NEW_ARRAY_NOKEY(value_type, n);

  m_reserved = n;

  ut_ad(size() < capacity());

  if (p != NULL) {
    ::memmove(m_ptr, p, size() * sizeof(value_type));

    UT_DELETE_ARRAY(p);
  }
}

/**
Copy and overwrite this array contents
@param start		Source array
@param end		Pointer to end of array */
void ReadView::ids_t::assign(const value_type *start, const value_type *end) {
  ut_ad(end >= start);

  ulint n = end - start;

  /* No need to copy the old contents across during reserve(). */
  clear();

  /* Create extra space if required. */
  reserve(n);

  resize(n);

  ut_ad(size() == n);

  ::memmove(m_ptr, start, size() * sizeof(value_type));
}

/**
Append a value to the array.
@param value		the value to append */
void ReadView::ids_t::push_back(value_type value) {
  if (capacity() <= size()) {
    reserve(size() * 2);
  }

  m_ptr[m_size++] = value;
  ut_ad(size() <= capacity());
}

/**
Insert the value in the correct slot, preserving the order. Doesn't
check for duplicates. */

void ReadView::ids_t::insert(value_type value) {
  ut_ad(value > 0);

  reserve(size() + 1);

  if (empty() || back() < value) {
    push_back(value);
    return;
  }

  value_type *end = data() + size();
  value_type *ub = std::upper_bound(data(), end, value);

  if (ub == end) {
    push_back(value);
  } else {
    ut_ad(ub < end);

    ulint n_elems = std::distance(ub, end);
    ulint n = n_elems * sizeof(value_type);

    /* Note: Copying overlapped memory locations. */
    ::memmove(ub + 1, ub, n);

    *ub = value;

    resize(size() + 1);
  }
}

/**
ReadView constructor */
ReadView::ReadView()
    : m_low_limit_id(),
      m_up_limit_id(),
      m_creator_trx_id(),
      m_ids(),
      m_low_limit_no() {
  ut_d(::memset(&m_view_list, 0x0, sizeof(m_view_list)));
}

/**
ReadView destructor */
ReadView::~ReadView() {
  // Do nothing
}


/**
Copy the transaction ids from the source vector */
void ReadView::copy_trx_ids(const trx_ids_t &trx_ids) {
  ulint size = trx_ids.size();

  if (m_creator_trx_id > 0) {
    ut_ad(size > 0);
    --size;
  }

  if (size == 0) {
    m_ids.clear();
    return;
  }

  m_ids.reserve(size);
  m_ids.resize(size);

  ids_t::value_type *p = m_ids.data();

  /* Copy all the trx_ids except the creator trx id */

  if (m_creator_trx_id > 0) {
    /* Note: We go through all this trouble because it is
    unclear whether std::vector::resize() will cause an
    overhead or not. We should test this extensively and
    if the vector to vector copy is fast enough then get
    rid of this code and replace it with more readable
    and obvious code. The code below does exactly one copy,
    and filters out the creator's trx id. */

    trx_ids_t::const_iterator it =
        std::lower_bound(trx_ids.begin(), trx_ids.end(), m_creator_trx_id);

    ut_ad(it != trx_ids.end() && *it == m_creator_trx_id);

    ulint i = std::distance(trx_ids.begin(), it);
    ulint n = i * sizeof(trx_ids_t::value_type);

    ::memmove(p, &trx_ids[0], n);

    n = (trx_ids.size() - i - 1) * sizeof(trx_ids_t::value_type);

    ut_ad(i + (n / sizeof(trx_ids_t::value_type)) == m_ids.size());

    if (n > 0) {
      ::memmove(p + i, &trx_ids[i + 1], n);
    }
  } else {
    ulint n = size * sizeof(trx_ids_t::value_type);

    ::memmove(p, &trx_ids[0], n);
  }

  m_up_limit_id = m_ids.front();

#ifdef UNIV_DEBUG
  /* Assert that all transaction ids in list are active. */
  for (trx_ids_t::const_iterator it = trx_ids.begin(); it != trx_ids.end();
       ++it) {
    trx_t *trx = trx_get_rw_trx_by_id(*it);
    ut_ad(trx != NULL);
    ut_ad(trx->state == TRX_STATE_ACTIVE || trx->state == TRX_STATE_PREPARED);
  }
#endif /* UNIV_DEBUG */
}


/**
Copy state from another view. Must call copy_complete() to finish.
@param other		view to copy from */

void ReadView::copy_prepare(const ReadView &other) {
  ut_ad(&other != this);

  if (!other.m_ids.empty()) {
    const ids_t::value_type *p = other.m_ids.data();

    m_ids.assign(p, p + other.m_ids.size());
  } else {
    m_ids.clear();
  }

  m_up_limit_id = other.m_up_limit_id;

  m_low_limit_no = other.m_low_limit_no;

  m_low_limit_id = other.m_low_limit_id;

  m_creator_trx_id = other.m_creator_trx_id;
}

/**
Complete the copy, insert the creator transaction id into the
m_ids too and adjust the m_up_limit_id, if required */

void ReadView::copy_complete() {
  ut_ad(!trx_sys_mutex_own());

  if (m_creator_trx_id > 0) {
    m_ids.insert(m_creator_trx_id);
  }

  if (!m_ids.empty()) {
    /* The last active transaction has the smallest id. */
    m_up_limit_id = std::min(m_ids.front(), m_up_limit_id);
  }

  ut_ad(m_up_limit_id <= m_low_limit_id);

  /* We added the creator transaction ID to the m_ids. */
  m_creator_trx_id = 0;
}

/**
Opens a read view where exactly the transactions serialized before this
point in time are seen in the view.
@param id		Creator transaction id */

void ReadView::prepare(trx_id_t id) {
  ut_ad(mutex_own(&trx_sys->mutex));

  m_creator_trx_id = id;

  m_low_limit_no = m_low_limit_id = m_up_limit_id = trx_sys->max_trx_id;

  if (!trx_sys->rw_trx_ids.empty()) {
    copy_trx_ids(trx_sys->rw_trx_ids);
  } else {
    m_ids.clear();
  }

  ut_ad(m_up_limit_id <= m_low_limit_id);

  if (UT_LIST_GET_LEN(trx_sys->serialisation_list) > 0) {
    const trx_t *trx;

    trx = UT_LIST_GET_FIRST(trx_sys->serialisation_list);

    if (trx->no < m_low_limit_no) {
      m_low_limit_no = trx->no;
    }
  }

  m_closed = false;
}
