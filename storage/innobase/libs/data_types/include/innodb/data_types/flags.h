#pragma once

#include <innodb/univ/univ.h>

/*-------------------------------------------*/
/* The 'MAIN TYPE' of a column */
#define DATA_MISSING 0 /* missing column */
#define DATA_VARCHAR                               \
  1 /* character varying of the                    \
    latin1_swedish_ci charset-collation; note      \
    that the MySQL format for this, DATA_BINARY,   \
    DATA_VARMYSQL, is also affected by whether the \
    'precise type' contains                        \
    DATA_MYSQL_TRUE_VARCHAR */
#define DATA_CHAR                                         \
  2                      /* fixed length character of the \
                         latin1_swedish_ci charset-collation */
#define DATA_FIXBINARY 3 /* binary string of fixed length */
#define DATA_BINARY 4    /* binary string */
#define DATA_BLOB                                                        \
  5                      /* binary large object, or a TEXT type;         \
                         if prtype & DATA_BINARY_TYPE == 0, then this is \
                         actually a TEXT column (or a BLOB created       \
                         with < 4.0.14; since column prefix indexes      \
                         came only in 4.0.14, the missing flag in BLOBs  \
                         created before that does not cause any harm) */
#define DATA_INT 6       /* integer: can be any size 1 - 8 bytes */
#define DATA_SYS_CHILD 7 /* address of the child page in node pointer */
#define DATA_SYS 8       /* system column */

/* Data types >= DATA_FLOAT must be compared using the whole field, not as
binary strings */

#define DATA_FLOAT 9
#define DATA_DOUBLE 10
#define DATA_DECIMAL 11  /* decimal number stored as an ASCII string */
#define DATA_VARMYSQL 12 /* any charset varying length char */
#define DATA_MYSQL 13    /* any charset fixed length char */
                         /* NOTE that 4.1.1 used DATA_MYSQL and
                         DATA_VARMYSQL for all character sets, and the
                         charset-collation for tables created with it
                         can also be latin1_swedish_ci */

/* DATA_POINT&DATA_VAR_POINT are for standard geometry datatype 'point' and
DATA_GEOMETRY include all other standard geometry datatypes as described in
OGC standard(line_string, polygon, multi_point, multi_polygon,
multi_line_string, geometry_collection, geometry).
Currently, geometry data is stored in the standard Well-Known Binary(WKB)
format (http://www.opengeospatial.org/standards/sfa).
We use BLOB as underlying datatype for DATA_GEOMETRY and DATA_VAR_POINT
while CHAR for DATA_POINT */
#define DATA_GEOMETRY 14 /* geometry datatype of variable length */
/* The following two are disabled temporarily, we won't create them in
get_innobase_type_from_mysql_type().
TODO: We will enable DATA_POINT/them when we come to the fixed-length POINT
again. */
#define DATA_POINT 15 /* geometry datatype of fixed length POINT */
#define DATA_VAR_POINT                       \
  16 /* geometry datatype of variable length \
     POINT, used when we want to store POINT \
     as BLOB internally */
#define DATA_MTYPE_MAX                        \
  63 /* dtype_store_for_order_and_null_size() \
     requires the values are <= 63 */

#define DATA_MTYPE_CURRENT_MIN DATA_VARCHAR   /* minimum value of mtype */
#define DATA_MTYPE_CURRENT_MAX DATA_VAR_POINT /* maximum value of mtype */
/*-------------------------------------------*/
/* The 'PRECISE TYPE' of a column */
/*
Tables created by a MySQL user have the following convention:

- In the least significant byte in the precise type we store the MySQL type
code (not applicable for system columns).

- In the second least significant byte we OR flags DATA_NOT_NULL,
DATA_UNSIGNED, DATA_BINARY_TYPE.

- In the third least significant byte of the precise type of string types we
store the MySQL charset-collation code. In DATA_BLOB columns created with
< 4.0.14 we do not actually know if it is a BLOB or a TEXT column. Since there
are no indexes on prefixes of BLOB or TEXT columns in < 4.0.14, this is no
problem, though.

Note that versions < 4.1.2 or < 5.0.1 did not store the charset code to the
precise type, since the charset was always the default charset of the MySQL
installation. If the stored charset code is 0 in the system table SYS_COLUMNS
of InnoDB, that means that the default charset of this MySQL installation
should be used.

When loading a table definition from the system tables to the InnoDB data
dictionary cache in main memory, InnoDB versions >= 4.1.2 and >= 5.0.1 check
if the stored charset-collation is 0, and if that is the case and the type is
a non-binary string, replace that 0 by the default charset-collation code of
this MySQL installation. In short, in old tables, the charset-collation code
in the system tables on disk can be 0, but in in-memory data structures
(dtype_t), the charset-collation code is always != 0 for non-binary string
types.

In new tables, in binary string types, the charset-collation code is the
MySQL code for the 'binary charset', that is, != 0.

For binary string types and for DATA_CHAR, DATA_VARCHAR, and for those
DATA_BLOB which are binary or have the charset-collation latin1_swedish_ci,
InnoDB performs all comparisons internally, without resorting to the MySQL
comparison functions. This is to save CPU time.

InnoDB's own internal system tables have different precise types for their
columns, and for them the precise type is usually not used at all.
*/

#define DATA_ENGLISH                                                \
  4                    /* English language character string: this   \
                       is a relic from pre-MySQL time and only used \
                       for InnoDB's own system tables */
#define DATA_ERROR 111 /* another relic from pre-MySQL time */

#define DATA_MYSQL_TYPE_MASK                     \
  255 /* AND with this mask to extract the MySQL \
      type from the precise type */
#define DATA_MYSQL_TRUE_VARCHAR          \
  15 /* MySQL type code for the >= 5.0.3 \
     format true VARCHAR */

/* Precise data types for system columns and the length of those columns;
NOTE: the values must run from 0 up in the order given! All codes must
be less than 256 */
#define DATA_ROW_ID 0     /* row id: a 48-bit integer */
#define DATA_ROW_ID_LEN 6 /* stored length for row id */

/** Transaction id: 6 bytes */
constexpr size_t DATA_TRX_ID = 1;


/** Rollback data pointer: 7 bytes */
constexpr size_t DATA_ROLL_PTR = 2;

static_assert(DATA_TRX_ID + 1 == DATA_ROLL_PTR, "DATA_TRX_ID value invalid!");


#define DATA_N_SYS_COLS 3 /* number of system columns defined above */

#define DATA_ITT_N_SYS_COLS 2
/* number of system columns for intrinsic
temporary table */

#define DATA_FTS_DOC_ID 3 /* Used as FTS DOC ID column */

#define DATA_SYS_PRTYPE_MASK 0xF /* mask to extract the above from prtype */

/* Flags ORed to the precise data type */
#define DATA_NOT_NULL                          \
  256 /* this is ORed to the precise type when \
      the column is declared as NOT NULL */
#define DATA_UNSIGNED                          \
  512 /* this id ORed to the precise type when \
      we have an unsigned integer type */
#define DATA_BINARY_TYPE                         \
  1024 /* if the data type is a binary character \
       string, this is ORed to the precise type: \
       this only holds for tables created with   \
       >= MySQL-4.0.14 */
/* #define	DATA_NONLATIN1	2048 This is a relic from < 4.1.2 and < 5.0.1.
                                In earlier versions this was set for some
                                BLOB columns.
*/
#define DATA_GIS_MBR 2048                        /* Used as GIS MBR column */
#define DATA_MBR_LEN SPDIMS * 2 * sizeof(double) /* GIS MBR length*/

#define DATA_LONG_TRUE_VARCHAR                                \
  4096                    /* this is ORed to the precise data \
                  type when the column is true VARCHAR where \
                  MySQL uses 2 bytes to store the data len;  \
                  for shorter VARCHARs MySQL uses only 1 byte */
#define DATA_VIRTUAL 8192 /* Virtual column */

/*-------------------------------------------*/

/* This many bytes we need to store the type information affecting the
alphabetical order for a single field and decide the storage size of an
SQL null*/
#define DATA_ORDER_NULL_TYPE_BUF_SIZE 4
/* In the >= 4.1.x storage format we add 2 bytes more so that we can also
store the charset-collation number; one byte is left unused, though */
#define DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE 6

/* Maximum multi-byte character length in bytes, plus 1 */
#define DATA_MBMAX 5

/* For DATA_POINT of dimension 2, the length of value in btree is always 25,
which is the summary of:
SRID_SIZE(4) + WKB_HEADER_SIZE(1+4) + POINT_DATA_SIZE(8*2).
So the length of physical record or POINT KEYs on btree are 25.
GIS_TODO: When we support multi-dimensions DATA_POINT, we should get the
length from corresponding column or index definition, instead of this MACRO
*/
#define DATA_POINT_LEN 25

/* Pack mbminlen, mbmaxlen to mbminmaxlen. */
#define DATA_MBMINMAXLEN(mbminlen, mbmaxlen) \
  ((mbmaxlen)*DATA_MBMAX + (mbminlen))
/* Get mbminlen from mbminmaxlen. Cast the result of UNIV_EXPECT to ulint
because in GCC it returns a long. */
#define DATA_MBMINLEN(mbminmaxlen) \
  ((ulint)UNIV_EXPECT(((mbminmaxlen) % DATA_MBMAX), 1))
/* Get mbmaxlen from mbminmaxlen. */
#define DATA_MBMAXLEN(mbminmaxlen) ((ulint)((mbminmaxlen) / DATA_MBMAX))

/* For checking if a geom_type is POINT */
#define DATA_POINT_MTYPE(mtype) \
  ((mtype) == DATA_POINT || (mtype) == DATA_VAR_POINT)

/* For checking if mtype is GEOMETRY datatype */
#define DATA_GEOMETRY_MTYPE(mtype) \
  (DATA_POINT_MTYPE(mtype) || (mtype) == DATA_GEOMETRY)

/* For checking if mtype is BLOB or GEOMETRY, since we use BLOB as
the underling datatype of GEOMETRY(not DATA_POINT) data. */
#define DATA_LARGE_MTYPE(mtype)                         \
  ((mtype) == DATA_BLOB || (mtype) == DATA_VAR_POINT || \
   (mtype) == DATA_GEOMETRY)

/* For checking if data type is big length data type. */
#define DATA_BIG_LEN_MTYPE(len, mtype) ((len) > 255 || DATA_LARGE_MTYPE(mtype))

/* For checking if the column is a big length column. */
#define DATA_BIG_COL(col) DATA_BIG_LEN_MTYPE((col)->len, (col)->mtype)

/* For checking if data type is large binary data type. */
#define DATA_LARGE_BINARY(mtype, prtype) \
  ((mtype) == DATA_GEOMETRY ||           \
   ((mtype) == DATA_BLOB && !((prtype)&DATA_BINARY_TYPE)))

/* We now support 15 bits (up to 32767) collation number */
#define MAX_CHAR_COLL_NUM 32767

/* Mask to get the Charset Collation number (0x7fff) */
#define CHAR_COLL_MASK MAX_CHAR_COLL_NUM



#define DATA_MYSQL_BINARY_CHARSET_COLL 63



/* Maximum values for various fields (for non-blob tuples) */
#define REC_MAX_N_FIELDS (1024 - 1)
