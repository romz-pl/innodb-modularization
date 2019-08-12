#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lst/lst.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/data_types/dtuple_print.h>

#include <iosfwd>

struct dfield_t;
struct dict_index_t;


/** Structure for an SQL data tuple of fields (logical record) */
struct dtuple_t {
  ulint info_bits;    /*!< info bits of an index record:
                      the default is 0; this field is used
                      if an index record is built from
                      a data tuple */
  ulint n_fields;     /*!< number of fields in dtuple */
  ulint n_fields_cmp; /*!< number of fields which should
                      be used in comparison services
                      of rem0cmp.*; the index search
                      is performed by comparing only these
                      fields, others are ignored; the
                      default value in dtuple creation is
                      the same value as n_fields */
  dfield_t *fields;   /*!< fields */
  ulint n_v_fields;   /*!< number of virtual fields */
  dfield_t *v_fields; /*!< fields on virtual column */
  UT_LIST_NODE_T(dtuple_t) tuple_list;
  /*!< data tuples can be linked into a
  list using this field */
#ifdef UNIV_DEBUG

  /** memory heap where this tuple is allocated. */
  mem_heap_t *m_heap;

  ulint magic_n; /*!< magic number, used in
                 debug assertions */
/** Value of dtuple_t::magic_n */
#define DATA_TUPLE_MAGIC_N 65478679
#endif /* UNIV_DEBUG */

  std::ostream &print(std::ostream &out) const {
    dtuple_print(out, this);
    return (out);
  }

  /* Read the trx id from the tuple (DB_TRX_ID)
  @return transaction id of the tuple. */
  trx_id_t get_trx_id() const;

  /** Ignore at most n trailing default fields if this is a tuple
  from instant index
  @param[in]	index	clustered index object for this tuple */
  void ignore_trailing_default(const dict_index_t *index);
};
