#pragma once

#include <innodb/univ/univ.h>

#include "mysql/components/services/psi_memory_bits.h"


/** Keys for registering allocations with performance schema.
Pointers to these variables are supplied to PFS code via the pfs_info[]
array and the PFS code initializes them via PSI_MEMORY_CALL(register_memory)().
mem_key_other and mem_key_std are special in the following way.
* If the caller has not provided a key and the file name of the caller is
  unknown, then mem_key_std will be used. This happens only when called from
  within std::* containers.
* If the caller has not provided a key and the file name of the caller is
  known, but is not amongst the predefined names (see ut_new_boot()) then
  mem_key_other will be used. Generally this should not happen and if it
  happens then that means that the list of predefined names must be extended.
Keep this list alphabetically sorted. */
extern PSI_memory_key mem_key_ahi;
extern PSI_memory_key mem_key_archive;
extern PSI_memory_key mem_key_buf_buf_pool;
extern PSI_memory_key mem_key_buf_stat_per_index_t;
/** Memory key for clone */
extern PSI_memory_key mem_key_clone;
extern PSI_memory_key mem_key_dict_stats_bg_recalc_pool_t;
extern PSI_memory_key mem_key_dict_stats_index_map_t;
extern PSI_memory_key mem_key_dict_stats_n_diff_on_level;
extern PSI_memory_key mem_key_other;
extern PSI_memory_key mem_key_partitioning;
extern PSI_memory_key mem_key_row_log_buf;
extern PSI_memory_key mem_key_row_merge_sort;
extern PSI_memory_key mem_key_std;
extern PSI_memory_key mem_key_trx_sys_t_rw_trx_ids;
extern PSI_memory_key mem_key_undo_spaces;
extern PSI_memory_key mem_key_ut_lock_free_hash_t;
/* Please obey alphabetical order in the definitions above. */

#ifdef UNIV_PFS_MEMORY

/** List of filenames that allocate memory and are instrumented via PFS. */
static constexpr const char *auto_event_names[] = {
    /* Keep this list alphabetically sorted. */
    "api0api",
    "api0misc",
    "btr0btr",
    "btr0bulk",
    "btr0cur",
    "btr0pcur",
    "btr0sea",
    "btr0types",
    "buf",
    "buf0buddy",
    "buf0buf",
    "buf0checksum",
    "buf0dblwr",
    "buf0dump",
    "buf0flu",
    "buf0lru",
    "buf0rea",
    "buf0stats",
    "buf0types",
    "checksum",
    "crc32",
    "create",
    "data0data",
    "data0type",
    "data0types",
    "db0err",
    "dict",
    "dict0boot",
    "dict0crea",
    "dict0dd",
    "dict0dict",
    "dict0load",
    "dict0mem",
    "dict0priv",
    "dict0sdi",
    "dict0stats",
    "dict0stats_bg",
    "dict0types",
    "dyn0buf",
    "dyn0types",
    "eval0eval",
    "eval0proc",
    "fil0fil",
    "fil0types",
    "file",
    "fsp0file",
    "fsp0fsp",
    "fsp0space",
    "fsp0sysspace",
    "fsp0types",
    "fts0ast",
    "fts0blex",
    "fts0config",
    "fts0fts",
    "fts0opt",
    "fts0pars",
    "fts0plugin",
    "fts0priv",
    "fts0que",
    "fts0sql",
    "fts0tlex",
    "fts0tokenize",
    "fts0types",
    "fts0vlc",
    "fut0fut",
    "fut0lst",
    "gis0geo",
    "gis0rtree",
    "gis0sea",
    "gis0type",
    "ha0ha",
    "ha0storage",
    "ha_innodb",
    "ha_innopart",
    "ha_prototypes",
    "handler0alter",
    "hash0hash",
    "i_s",
    "ib0mutex",
    "ibuf0ibuf",
    "ibuf0types",
    "lexyy",
    "lob0lob",
    "lock0iter",
    "lock0lock",
    "lock0prdt",
    "lock0priv",
    "lock0types",
    "lock0wait",
    "log0log",
    "log0recv",
    "log0write",
    "mach0data",
    "mem",
    "mem0mem",
    "memory",
    "mtr0log",
    "mtr0mtr",
    "mtr0types",
    "os0atomic",
    "os0event",
    "os0file",
    "os0numa",
    "os0once",
    "os0proc",
    "os0thread",
    "page",
    "page0cur",
    "page0page",
    "page0size",
    "page0types",
    "page0zip",
    "pars0grm",
    "pars0lex",
    "pars0opt",
    "pars0pars",
    "pars0sym",
    "pars0types",
    "que0que",
    "que0types",
    "read0read",
    "read0types",
    "rec",
    "rem0cmp",
    "rem0rec",
    "rem0types",
    "row0ext",
    "row0ftsort",
    "row0import",
    "row0ins",
    "row0log",
    "row0merge",
    "row0mysql",
    "row0purge",
    "row0quiesce",
    "row0row",
    "row0sel",
    "row0types",
    "row0uins",
    "row0umod",
    "row0undo",
    "row0upd",
    "row0vers",
    "sess0sess",
    "srv0conc",
    "srv0mon",
    "srv0srv",
    "srv0start",
    "srv0tmp",
    "sync0arr",
    "sync0debug",
    "sync0policy",
    "sync0sharded_rw",
    "sync0rw",
    "sync0sync",
    "sync0types",
    "trx0i_s",
    "trx0purge",
    "trx0rec",
    "trx0roll",
    "trx0rseg",
    "trx0sys",
    "trx0trx",
    "trx0types",
    "trx0undo",
    "trx0xa",
    "usr0sess",
    "usr0types",
    "ut",
    "ut0byte",
    "ut0counter",
    "ut0crc32",
    "ut0dbg",
    "ut0link_buf",
    "ut0list",
    "ut0lock_free_hash",
    "ut0lst",
    "ut0mem",
    "ut0mutex",
    "ut0new",
    "ut0pool",
    "ut0rbt",
    "ut0rnd",
    "ut0sort",
    "ut0stage",
    "ut0ut",
    "ut0vec",
    "ut0wqueue",
    "zipdecompress",
};

static constexpr size_t n_auto = UT_ARR_SIZE(auto_event_names);
extern PSI_memory_key auto_event_keys[n_auto];
extern PSI_memory_info pfs_info_auto[n_auto];
/** Array of PFS keys. */
extern PSI_memory_info pfs_info[];

/** gcc 5 fails to evalutate costexprs at compile time. */
#if defined(__GNUG__) && (__GNUG__ == 5)

/** Compute whether a string begins with a given prefix, compile-time.
@param[in]	a	first string, taken to be zero-terminated
@param[in]	b	second string (prefix to search for)
@param[in]	b_len	length in bytes of second string
@param[in]	index	character index to start comparing at
@return whether b is a prefix of a */
constexpr bool ut_string_begins_with(const char *a, const char *b, size_t b_len,
                                     size_t index = 0) {
  return (index == b_len || (a[index] == b[index] &&
                             ut_string_begins_with(a, b, b_len, index + 1)));
}

/** Find the length of the filename without its file extension.
@param[in]	file	filename, with extension but without directory
@param[in]	index	character index to start scanning for extension
                        separator at
@return length, in bytes */
constexpr size_t ut_len_without_extension(const char *file, size_t index = 0) {
  return ((file[index] == '\0' || file[index] == '.')
              ? index
              : ut_len_without_extension(file, index + 1));
}

/** Retrieve a memory key (registered with PFS), given the file name of the
caller.
@param[in]	file	portion of the filename - basename, with extension
@param[in]	len	length of the filename to check for
@param[in]	index	index of first PSI key to check
@return registered memory key or PSI_NOT_INSTRUMENTED if not found */
constexpr PSI_memory_key ut_new_get_key_by_base_file(const char *file,
                                                     size_t len,
                                                     size_t index = 0) {
  return ((index == n_auto)
              ? PSI_NOT_INSTRUMENTED
              : (ut_string_begins_with(auto_event_names[index], file, len)
                     ? auto_event_keys[index]
                     : ut_new_get_key_by_base_file(file, len, index + 1)));
}

/** Retrieve a memory key (registered with PFS), given the file name of
the caller.
@param[in]	file	portion of the filename - basename, with extension
@return registered memory key or PSI_NOT_INSTRUMENTED if not found */
constexpr PSI_memory_key ut_new_get_key_by_file(const char *file) {
  return (ut_new_get_key_by_base_file(file, ut_len_without_extension(file)));
}

#define UT_NEW_THIS_FILE_PSI_KEY ut_new_get_key_by_file(MY_BASENAME)

#else /* __GNUG__ == 5 */

/** Compute whether a string begins with a given prefix, compile-time.
@param[in]	a	first string, taken to be zero-terminated
@param[in]	b	second string (prefix to search for)
@param[in]	b_len	length in bytes of second string
@return whether b is a prefix of a */
constexpr bool ut_string_begins_with(const char *a, const char *b,
                                     size_t b_len) {
  for (size_t i = 0; i < b_len; ++i) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

/** Find the length of the filename without its file extension.
@param[in]	file	filename, with extension but without directory
@return length, in bytes */
constexpr size_t ut_len_without_extension(const char *file) {
  for (size_t i = 0;; ++i) {
    if (file[i] == '\0' || file[i] == '.') {
      return i;
    }
  }
}

/** Retrieve a memory key (registered with PFS), given the file name of the
caller.
@param[in]	file	portion of the filename - basename, with extension
@param[in]	len	length of the filename to check for
@return index to registered memory key or -1 if not found */
constexpr int ut_new_get_key_by_base_file(const char *file, size_t len) {
  for (size_t i = 0; i < n_auto; ++i) {
    if (ut_string_begins_with(auto_event_names[i], file, len)) {
      return i;
    }
  }
  return -1;
}

/** Retrieve a memory key (registered with PFS), given the file name of
the caller.
@param[in]	file	portion of the filename - basename, with extension
@return index to memory key or -1 if not found */
constexpr int ut_new_get_key_by_file(const char *file) {
  return ut_new_get_key_by_base_file(file, ut_len_without_extension(file));
}

// Sending an expression through a template variable forces the compiler to
// evaluate the expression at compile time (constexpr in itself has no such
// guarantee, only that the compiler is allowed).
template <int Value>
struct force_constexpr {
  static constexpr int value = Value;
};

#define UT_NEW_THIS_FILE_PSI_INDEX \
  (force_constexpr<ut_new_get_key_by_file(MY_BASENAME)>::value)

#define UT_NEW_THIS_FILE_PSI_KEY    \
  (UT_NEW_THIS_FILE_PSI_INDEX == -1 \
       ? PSI_NOT_INSTRUMENTED       \
       : auto_event_keys[UT_NEW_THIS_FILE_PSI_INDEX])

#endif /* __GNUG__ == 5 */

#endif /* UNIV_PFS_MEMORY */
