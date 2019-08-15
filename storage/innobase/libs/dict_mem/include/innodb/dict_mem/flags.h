#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_types/flags.h>
#include "sql/dd/object_id.h"

/** Type flags of an index: OR'ing of the flags is allowed to define a
combination of types */
/* @{ */
#define DICT_CLUSTERED                                     \
  1                   /*!< clustered index; for other than \
                      auto-generated clustered indexes,    \
                      also DICT_UNIQUE will be set */
#define DICT_UNIQUE 2 /*!< unique index */
#define DICT_IBUF 8   /*!< insert buffer tree */
#define DICT_CORRUPT                      \
  16 /*!< bit to store the corrupted flag \
     in SYS_INDEXES.TYPE */
#define DICT_FTS                              \
  32 /* FTS index; can't be combined with the \
     other flags */
#define DICT_SPATIAL                                                  \
  64                     /* SPATIAL index; can't be combined with the \
                         other flags */
#define DICT_VIRTUAL 128 /* Index on Virtual column */

#define DICT_SDI                                  \
  256 /* Tablespace dictionary Index. Set only in \
      in-memory index structure. */

#define DICT_IT_BITS             \
  9 /*!< number of bits used for \
    SYS_INDEXES.TYPE */
/* @} */

#if 0                         /* not implemented, retained for history */
/** Types for a table object */
#define DICT_TABLE_ORDINARY 1 /*!< ordinary table */
#define DICT_TABLE_CLUSTER_MEMBER 2
#define DICT_TABLE_CLUSTER          \
  3 /* this means that the table is \
    really a cluster definition */
#endif

/* Table and tablespace flags are generally not used for the Antelope file
format except for the low order bit, which is used differently depending on
where the flags are stored.

==================== Low order flags bit =========================
                    | REDUNDANT | COMPACT | COMPRESSED and DYNAMIC
SYS_TABLES.TYPE     |     1     |    1    |     1
dict_table_t::flags |     0     |    1    |     1
FSP_SPACE_FLAGS     |     0     |    0    |     1
fil_space_t::flags  |     0     |    0    |     1

Before the 5.1 plugin, SYS_TABLES.TYPE was always DICT_TABLE_ORDINARY (1)
and the tablespace flags field was always 0. In the 5.1 plugin, these fields
were repurposed to identify compressed and dynamic row formats.

The following types and constants describe the flags found in dict_table_t
and SYS_TABLES.TYPE.  Similar flags found in fil_space_t and FSP_SPACE_FLAGS
are described in fsp0fsp.h. */

/* @{ */
/** dict_table_t::flags bit 0 is equal to 0 if the row format = Redundant */
#define DICT_TF_REDUNDANT 0 /*!< Redundant row format. */
/** dict_table_t::flags bit 0 is equal to 1 if the row format = Compact */
#define DICT_TF_COMPACT 1 /*!< Compact row format. */

/** This bitmask is used in SYS_TABLES.N_COLS to set and test whether
the Compact page format is used, i.e ROW_FORMAT != REDUNDANT */
#define DICT_N_COLS_COMPACT 0x80000000UL

/** Width of the COMPACT flag */
#define DICT_TF_WIDTH_COMPACT 1

/** Width of the ZIP_SSIZE flag */
#define DICT_TF_WIDTH_ZIP_SSIZE 4

/** Width of the ATOMIC_BLOBS flag.  The ROW_FORMAT=REDUNDANT and
ROW_FORMAT=COMPACT broke up BLOB and TEXT fields, storing the first 768 bytes
in the clustered index. ROW_FORMAT=DYNAMIC and ROW_FORMAT=COMPRESSED
store the whole blob or text field off-page atomically.
Secondary indexes are created from this external data using row_ext_t
to cache the BLOB prefixes. */
#define DICT_TF_WIDTH_ATOMIC_BLOBS 1

/** If a table is created with the MYSQL option DATA DIRECTORY and
innodb-file-per-table, an older engine will not be able to find that table.
This flag prevents older engines from attempting to open the table and
allows InnoDB to update_create_info() accordingly. */
#define DICT_TF_WIDTH_DATA_DIR 1

/** Width of the SHARED tablespace flag.
It is used to identify tables that exist inside a shared general tablespace.
If a table is created with the TABLESPACE=tsname option, an older engine will
not be able to find that table. This flag prevents older engines from attempting
to open the table and allows InnoDB to quickly find the tablespace. */

#define DICT_TF_WIDTH_SHARED_SPACE 1

/** Width of all the currently known table flags */
#define DICT_TF_BITS                                     \
  (DICT_TF_WIDTH_COMPACT + DICT_TF_WIDTH_ZIP_SSIZE +     \
   DICT_TF_WIDTH_ATOMIC_BLOBS + DICT_TF_WIDTH_DATA_DIR + \
   DICT_TF_WIDTH_SHARED_SPACE)

/** A mask of all the known/used bits in table flags */
#define DICT_TF_BIT_MASK (~(~0 << DICT_TF_BITS))

/** Zero relative shift position of the COMPACT field */
#define DICT_TF_POS_COMPACT 0
/** Zero relative shift position of the ZIP_SSIZE field */
#define DICT_TF_POS_ZIP_SSIZE (DICT_TF_POS_COMPACT + DICT_TF_WIDTH_COMPACT)
/** Zero relative shift position of the ATOMIC_BLOBS field */
#define DICT_TF_POS_ATOMIC_BLOBS \
  (DICT_TF_POS_ZIP_SSIZE + DICT_TF_WIDTH_ZIP_SSIZE)
/** Zero relative shift position of the DATA_DIR field */
#define DICT_TF_POS_DATA_DIR \
  (DICT_TF_POS_ATOMIC_BLOBS + DICT_TF_WIDTH_ATOMIC_BLOBS)
/** Zero relative shift position of the SHARED TABLESPACE field */
#define DICT_TF_POS_SHARED_SPACE (DICT_TF_POS_DATA_DIR + DICT_TF_WIDTH_DATA_DIR)
/** Zero relative shift position of the start of the UNUSED bits */
#define DICT_TF_POS_UNUSED \
  (DICT_TF_POS_SHARED_SPACE + DICT_TF_WIDTH_SHARED_SPACE)

/** Bit mask of the COMPACT field */
#define DICT_TF_MASK_COMPACT \
  ((~(~0U << DICT_TF_WIDTH_COMPACT)) << DICT_TF_POS_COMPACT)
/** Bit mask of the ZIP_SSIZE field */
#define DICT_TF_MASK_ZIP_SSIZE \
  ((~(~0U << DICT_TF_WIDTH_ZIP_SSIZE)) << DICT_TF_POS_ZIP_SSIZE)
/** Bit mask of the ATOMIC_BLOBS field */
#define DICT_TF_MASK_ATOMIC_BLOBS \
  ((~(~0U << DICT_TF_WIDTH_ATOMIC_BLOBS)) << DICT_TF_POS_ATOMIC_BLOBS)
/** Bit mask of the DATA_DIR field */
#define DICT_TF_MASK_DATA_DIR \
  ((~(~0U << DICT_TF_WIDTH_DATA_DIR)) << DICT_TF_POS_DATA_DIR)
/** Bit mask of the SHARED_SPACE field */
#define DICT_TF_MASK_SHARED_SPACE \
  ((~(~0U << DICT_TF_WIDTH_SHARED_SPACE)) << DICT_TF_POS_SHARED_SPACE)

/** Return the value of the COMPACT field */
#define DICT_TF_GET_COMPACT(flags) \
  ((flags & DICT_TF_MASK_COMPACT) >> DICT_TF_POS_COMPACT)
/** Return the value of the ZIP_SSIZE field */
#define DICT_TF_GET_ZIP_SSIZE(flags) \
  ((flags & DICT_TF_MASK_ZIP_SSIZE) >> DICT_TF_POS_ZIP_SSIZE)
/** Return the value of the ATOMIC_BLOBS field */
#define DICT_TF_HAS_ATOMIC_BLOBS(flags) \
  ((flags & DICT_TF_MASK_ATOMIC_BLOBS) >> DICT_TF_POS_ATOMIC_BLOBS)
/** Return the value of the DATA_DIR field */
#define DICT_TF_HAS_DATA_DIR(flags) \
  ((flags & DICT_TF_MASK_DATA_DIR) >> DICT_TF_POS_DATA_DIR)
/** Return the value of the SHARED_SPACE field */
#define DICT_TF_HAS_SHARED_SPACE(flags) \
  ((flags & DICT_TF_MASK_SHARED_SPACE) >> DICT_TF_POS_SHARED_SPACE)
/** Return the contents of the UNUSED bits */
#define DICT_TF_GET_UNUSED(flags) (flags >> DICT_TF_POS_UNUSED)
/* @} */

/** @brief Table Flags set number 2.

These flags will be stored in SYS_TABLES.MIX_LEN.  All unused flags
will be written as 0.  The column may contain garbage for tables
created with old versions of InnoDB that only implemented
ROW_FORMAT=REDUNDANT.  InnoDB engines do not check these flags
for unknown bits in order to protect backward incompatibility. */
/* @{ */
/** Total number of bits in table->flags2. */
#define DICT_TF2_BITS 11
#define DICT_TF2_UNUSED_BIT_MASK (~0U << DICT_TF2_BITS)
#define DICT_TF2_BIT_MASK ~DICT_TF2_UNUSED_BIT_MASK

/** TEMPORARY; TRUE for tables from CREATE TEMPORARY TABLE. */
#define DICT_TF2_TEMPORARY 1

/** The table has an internal defined DOC ID column */
#define DICT_TF2_FTS_HAS_DOC_ID 2

/** The table has an FTS index */
#define DICT_TF2_FTS 4

/** Need to add Doc ID column for FTS index build.
This is a transient bit for index build */
#define DICT_TF2_FTS_ADD_DOC_ID 8

/** This bit is used during table creation to indicate that it will
use its own tablespace instead of the system tablespace. */
#define DICT_TF2_USE_FILE_PER_TABLE 16

/** Set when we discard/detach the tablespace */
#define DICT_TF2_DISCARDED 32

/** Intrinsic table bit
Intrinsic table is table created internally by MySQL modules viz. Optimizer,
FTS, etc.... Intrinsic table has all the properties of the normal table except
it is not created by user and so not visible to end-user. */
#define DICT_TF2_INTRINSIC 128

/** Encryption table bit for innodb_file-per-table only. */
#define DICT_TF2_ENCRYPTION_FILE_PER_TABLE 256

/** FTS AUX hidden table bit. */
#define DICT_TF2_AUX 512

/** Table is opened by resurrected trx during crash recovery. */
#define DICT_TF2_RESURRECT_PREPARED 1024
/* @} */

#define DICT_TF2_FLAG_SET(table, flag) (table->flags2 |= (flag))

#define DICT_TF2_FLAG_IS_SET(table, flag) (table->flags2 & (flag))

#define DICT_TF2_FLAG_UNSET(table, flag) (table->flags2 &= ~(flag))

/** Tables could be chained together with Foreign key constraint. When
first load the parent table, we would load all of its descedents.
This could result in rescursive calls and out of stack error eventually.
DICT_FK_MAX_RECURSIVE_LOAD defines the maximum number of recursive loads,
when exceeded, the child table will not be loaded. It will be loaded when
the foreign constraint check needs to be run. */
#define DICT_FK_MAX_RECURSIVE_LOAD 20

/** Similarly, when tables are chained together with foreign key constraints
with on cascading delete/update clause, delete from parent table could
result in recursive cascading calls. This defines the maximum number of
such cascading deletes/updates allowed. When exceeded, the delete from
parent table will fail, and user has to drop excessive foreign constraint
before proceeds. */
#define FK_MAX_CASCADE_DEL 15

/* REC_ANTELOPE_MAX_INDEX_COL_LEN is measured in bytes and is the maximum
indexed field length (or indexed prefix length) for indexes on tables of
ROW_FORMAT=REDUNDANT and ROW_FORMAT=COMPACT format.
Before we support UTF-8 encodings with mbmaxlen = 4, a UTF-8 character
may take at most 3 bytes.  So the limit was set to 3*256, so that one
can create a column prefix index on 256 characters of a TEXT or VARCHAR
column also in the UTF-8 charset.
This constant MUST NOT BE CHANGED, or the compatibility of InnoDB data
files would be at risk! */
#define REC_ANTELOPE_MAX_INDEX_COL_LEN 768


/** @brief DICT_ANTELOPE_MAX_INDEX_COL_LEN is measured in bytes and
is the maximum indexed column length (or indexed prefix length) in
ROW_FORMAT=REDUNDANT and ROW_FORMAT=COMPACT. Also, in any format,
any fixed-length field that is longer than this will be encoded as
a variable-length field.

It is set to 3*256, so that one can create a column prefix index on
256 characters of a TEXT or VARCHAR column also in the UTF-8
charset. In that charset, a character may take at most 3 bytes.  This
constant MUST NOT BE CHANGED, or the compatibility of InnoDB data
files would be at risk! */
#define DICT_ANTELOPE_MAX_INDEX_COL_LEN REC_ANTELOPE_MAX_INDEX_COL_LEN

/** Find out maximum indexed column length by its table format.
For ROW_FORMAT=REDUNDANT and ROW_FORMAT=COMPACT, the maximum
field length is REC_ANTELOPE_MAX_INDEX_COL_LEN - 1 (767). For
ROW_FORMAT=COMPRESSED and ROW_FORMAT=DYNAMIC, the length could
be REC_VERSION_56_MAX_INDEX_COL_LEN (3072) bytes */
#define DICT_MAX_FIELD_LEN_BY_FORMAT(table)                              \
  (dict_table_has_atomic_blobs(table) ? REC_VERSION_56_MAX_INDEX_COL_LEN \
                                      : REC_ANTELOPE_MAX_INDEX_COL_LEN - 1)

#define DICT_MAX_FIELD_LEN_BY_FORMAT_FLAG(flags)                      \
  (DICT_TF_HAS_ATOMIC_BLOBS(flags) ? REC_VERSION_56_MAX_INDEX_COL_LEN \
                                   : REC_ANTELOPE_MAX_INDEX_COL_LEN - 1)

/** Defines the maximum fixed length column size */
#define DICT_MAX_FIXED_COL_LEN DICT_ANTELOPE_MAX_INDEX_COL_LEN






/** PADDING HEURISTIC BASED ON LINEAR INCREASE OF PADDING TO AVOID
 COMPRESSION FAILURES
 (Note: this is relevant only for compressed indexes)
 GOAL: Avoid compression failures by maintaining information about the
 compressibility of data. If data is not very compressible then leave
 some extra space 'padding' in the uncompressed page making it more
 likely that compression of less than fully packed uncompressed page will
 succeed.

 This padding heuristic works by increasing the pad linearly until the
 desired failure rate is reached. A "round" is a fixed number of
 compression operations.
 After each round, the compression failure rate for that round is
 computed. If the failure rate is too high, then padding is incremented
 by a fixed value, otherwise it's left intact.
 If the compression failure is lower than the desired rate for a fixed
 number of consecutive rounds, then the padding is decreased by a fixed
 value. This is done to prevent overshooting the padding value,
 and to accommodate the possible change in data compressibility. */

/** Number of zip ops in one round. */
#define ZIP_PAD_ROUND_LEN (128)

/** Number of successful rounds after which the padding is decreased */
#define ZIP_PAD_SUCCESSFUL_ROUND_LIMIT (5)

/** Amount by which padding is increased. */
#define ZIP_PAD_INCR (128)





/** The flags for ON_UPDATE and ON_DELETE can be ORed; the default is that
a foreign key constraint is enforced, therefore RESTRICT just means no flag */
/* @{ */
#define DICT_FOREIGN_ON_DELETE_CASCADE 1    /*!< ON DELETE CASCADE */
#define DICT_FOREIGN_ON_DELETE_SET_NULL 2   /*!< ON DELETE SET NULL */
#define DICT_FOREIGN_ON_UPDATE_CASCADE 4    /*!< ON UPDATE CASCADE */
#define DICT_FOREIGN_ON_UPDATE_SET_NULL 8   /*!< ON UPDATE SET NULL */
#define DICT_FOREIGN_ON_DELETE_NO_ACTION 16 /*!< ON DELETE NO ACTION */
#define DICT_FOREIGN_ON_UPDATE_NO_ACTION 32 /*!< ON UPDATE NO ACTION */
/* @} */






#define DICT_HEAP_SIZE                   \
  100 /*!< initial memory heap size when \
      creating a table or index object */

/** SDI version. Written on Page 1 & 2 at FIL_PAGE_FILE_FLUSH_LSN offset. */
const uint32_t SDI_VERSION = 1;

/** Space id of the transaction system page (the system tablespace) */
static const space_id_t TRX_SYS_SPACE = 0;

/** Space id of system tablespace */
const space_id_t SYSTEM_TABLE_SPACE = TRX_SYS_SPACE;

/** Identifies generated InnoDB foreign key names */
static char dict_ibfk[] = "_ibfk_";



/** The number of fields in the nonleaf page of spatial index, except
the page no field. */
#define DICT_INDEX_SPATIAL_NODEPTR_SIZE 1




/** Max number of rollback segments: the number of segment specification slots
in the transaction system array; rollback segment id must fit in one (signed)
byte, therefore 128; each slot is currently 8 bytes in size. If you want
to raise the level to 256 then you will need to fix some assertions that
impose the 7 bit restriction. e.g., mach_write_to_3() */
#define TRX_SYS_N_RSEGS 128

/** Minimum and Maximum number of implicit undo tablespaces.  This kind
of undo tablespace is always created and found in --innodb-undo-directory. */
#define FSP_MIN_UNDO_TABLESPACES 2
#define FSP_MAX_UNDO_TABLESPACES (TRX_SYS_N_RSEGS - 1)
#define FSP_IMPLICIT_UNDO_TABLESPACES 2
#define FSP_MAX_ROLLBACK_SEGMENTS (TRX_SYS_N_RSEGS)



/** The first ID of the redo log pseudo-tablespace */
constexpr space_id_t dict_sys_t_s_log_space_first_id = 0xFFFFFFF0UL;

/** Use maximum UINT value to indicate invalid space ID. */
constexpr space_id_t dict_sys_t_s_invalid_space_id = 0xFFFFFFFF;

/** The data dictionary tablespace ID. */
constexpr space_id_t dict_sys_t_s_space_id = 0xFFFFFFFE;

/** The innodb_temporary tablespace ID. */
constexpr space_id_t dict_sys_t_s_temp_space_id = 0xFFFFFFFD;

/** The number of space IDs dedicated to each undo tablespace */
constexpr space_id_t dict_sys_t_undo_space_id_range = 512;

/** The lowest undo tablespace ID. */
constexpr space_id_t dict_sys_t_s_min_undo_space_id =
    dict_sys_t_s_log_space_first_id - (FSP_MAX_UNDO_TABLESPACES * dict_sys_t_undo_space_id_range);

/** The highest undo  tablespace ID. */
constexpr space_id_t dict_sys_t_s_max_undo_space_id = dict_sys_t_s_log_space_first_id - 1;

/** The first reserved tablespace ID */
constexpr space_id_t dict_sys_t_s_reserved_space_id = dict_sys_t_s_min_undo_space_id;

/** Leave 1K space_ids and start space_ids for temporary
general tablespaces (total 400K space_ids)*/
constexpr space_id_t dict_sys_t_s_max_temp_space_id = dict_sys_t_s_reserved_space_id - 1000;

/** Lowest temporary general space id */
constexpr space_id_t dict_sys_t_s_min_temp_space_id = dict_sys_t_s_reserved_space_id - 1000 - 400000;

/** The dd::Tablespace::id of the dictionary tablespace. */
constexpr dd::Object_id dict_sys_t_s_dd_space_id = 1;

/** The dd::Tablespace::id of innodb_system. */
constexpr dd::Object_id dict_sys_t_s_dd_sys_space_id = 2;

/** The dd::Tablespace::id of innodb_temporary. */
constexpr dd::Object_id dict_sys_t_s_dd_temp_space_id = 3;

