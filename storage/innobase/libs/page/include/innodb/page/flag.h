#pragma once

#include <innodb/univ/univ.h>

/* @defgroup fsp_flags InnoDB Tablespace Flag Constants @{ */

/** Width of the POST_ANTELOPE flag */
#define FSP_FLAGS_WIDTH_POST_ANTELOPE 1


/** Number of flag bits used to indicate the tablespace zip page size */
#define FSP_FLAGS_WIDTH_ZIP_SSIZE 4


/** Width of the ATOMIC_BLOBS flag.  The ability to break up a long
column into an in-record prefix and an externally stored part is available
to ROW_FORMAT=REDUNDANT and ROW_FORMAT=COMPACT. */
#define FSP_FLAGS_WIDTH_ATOMIC_BLOBS 1


/** Number of flag bits used to indicate the tablespace page size */
#define FSP_FLAGS_WIDTH_PAGE_SSIZE 4


/** Width of the DATA_DIR flag.  This flag indicates that the tablespace
is found in a remote location, not the default data directory. */
#define FSP_FLAGS_WIDTH_DATA_DIR 1


/** Width of the SHARED flag.  This flag indicates that the tablespace
was created with CREATE TABLESPACE and can be shared by multiple tables. */
#define FSP_FLAGS_WIDTH_SHARED 1


/** Width of the TEMPORARY flag.  This flag indicates that the tablespace
is a temporary tablespace and everything in it is temporary, meaning that
it is for a single client and should be deleted upon startup if it exists. */
#define FSP_FLAGS_WIDTH_TEMPORARY 1


/** Width of the encryption flag.  This flag indicates that the tablespace
is a tablespace with encryption. */
#define FSP_FLAGS_WIDTH_ENCRYPTION 1


/** Width of the SDI flag.  This flag indicates the presence of
tablespace dictionary.*/
#define FSP_FLAGS_WIDTH_SDI 1


/** Width of all the currently known tablespace flags */
#define FSP_FLAGS_WIDTH                                        \
  (FSP_FLAGS_WIDTH_POST_ANTELOPE + FSP_FLAGS_WIDTH_ZIP_SSIZE + \
   FSP_FLAGS_WIDTH_ATOMIC_BLOBS + FSP_FLAGS_WIDTH_PAGE_SSIZE + \
   FSP_FLAGS_WIDTH_DATA_DIR + FSP_FLAGS_WIDTH_SHARED +         \
   FSP_FLAGS_WIDTH_TEMPORARY + FSP_FLAGS_WIDTH_ENCRYPTION +    \
   FSP_FLAGS_WIDTH_SDI)


/** A mask of all the known/used bits in tablespace flags */
#define FSP_FLAGS_MASK (~(~0U << FSP_FLAGS_WIDTH))


/** Zero relative shift position of the POST_ANTELOPE field */
#define FSP_FLAGS_POS_POST_ANTELOPE 0


/** Zero relative shift position of the ZIP_SSIZE field */
#define FSP_FLAGS_POS_ZIP_SSIZE \
  (FSP_FLAGS_POS_POST_ANTELOPE + FSP_FLAGS_WIDTH_POST_ANTELOPE)


/** Zero relative shift position of the ATOMIC_BLOBS field */
#define FSP_FLAGS_POS_ATOMIC_BLOBS \
  (FSP_FLAGS_POS_ZIP_SSIZE + FSP_FLAGS_WIDTH_ZIP_SSIZE)


/** Zero relative shift position of the PAGE_SSIZE field */
#define FSP_FLAGS_POS_PAGE_SSIZE \
  (FSP_FLAGS_POS_ATOMIC_BLOBS + FSP_FLAGS_WIDTH_ATOMIC_BLOBS)


/** Zero relative shift position of the start of the DATA_DIR bit */
#define FSP_FLAGS_POS_DATA_DIR \
  (FSP_FLAGS_POS_PAGE_SSIZE + FSP_FLAGS_WIDTH_PAGE_SSIZE)


/** Zero relative shift position of the start of the SHARED bit */
#define FSP_FLAGS_POS_SHARED (FSP_FLAGS_POS_DATA_DIR + FSP_FLAGS_WIDTH_DATA_DIR)


/** Zero relative shift position of the start of the TEMPORARY bit */
#define FSP_FLAGS_POS_TEMPORARY (FSP_FLAGS_POS_SHARED + FSP_FLAGS_WIDTH_SHARED)


/** Zero relative shift position of the start of the ENCRYPTION bit */
#define FSP_FLAGS_POS_ENCRYPTION \
  (FSP_FLAGS_POS_TEMPORARY + FSP_FLAGS_WIDTH_TEMPORARY)


/** Zero relative shift position of the start of the SDI bits */
#define FSP_FLAGS_POS_SDI \
  (FSP_FLAGS_POS_ENCRYPTION + FSP_FLAGS_WIDTH_ENCRYPTION)


/** Zero relative shift position of the start of the UNUSED bits */
#define FSP_FLAGS_POS_UNUSED (FSP_FLAGS_POS_SDI + FSP_FLAGS_WIDTH_SDI)


/** Bit mask of the POST_ANTELOPE field */
#define FSP_FLAGS_MASK_POST_ANTELOPE \
  ((~(~0U << FSP_FLAGS_WIDTH_POST_ANTELOPE)) << FSP_FLAGS_POS_POST_ANTELOPE)


/** Bit mask of the ZIP_SSIZE field */
#define FSP_FLAGS_MASK_ZIP_SSIZE \
  ((~(~0U << FSP_FLAGS_WIDTH_ZIP_SSIZE)) << FSP_FLAGS_POS_ZIP_SSIZE)


/** Bit mask of the ATOMIC_BLOBS field */
#define FSP_FLAGS_MASK_ATOMIC_BLOBS \
  ((~(~0U << FSP_FLAGS_WIDTH_ATOMIC_BLOBS)) << FSP_FLAGS_POS_ATOMIC_BLOBS)


/** Bit mask of the PAGE_SSIZE field */
#define FSP_FLAGS_MASK_PAGE_SSIZE \
  ((~(~0U << FSP_FLAGS_WIDTH_PAGE_SSIZE)) << FSP_FLAGS_POS_PAGE_SSIZE)


/** Bit mask of the DATA_DIR field */
#define FSP_FLAGS_MASK_DATA_DIR \
  ((~(~0U << FSP_FLAGS_WIDTH_DATA_DIR)) << FSP_FLAGS_POS_DATA_DIR)


/** Bit mask of the SHARED field */
#define FSP_FLAGS_MASK_SHARED \
  ((~(~0U << FSP_FLAGS_WIDTH_SHARED)) << FSP_FLAGS_POS_SHARED)


/** Bit mask of the TEMPORARY field */
#define FSP_FLAGS_MASK_TEMPORARY \
  ((~(~0U << FSP_FLAGS_WIDTH_TEMPORARY)) << FSP_FLAGS_POS_TEMPORARY)


/** Bit mask of the ENCRYPTION field */
#define FSP_FLAGS_MASK_ENCRYPTION \
  ((~(~0U << FSP_FLAGS_WIDTH_ENCRYPTION)) << FSP_FLAGS_POS_ENCRYPTION)


/** Bit mask of the SDI field */
#define FSP_FLAGS_MASK_SDI \
  ((~(~0U << FSP_FLAGS_WIDTH_SDI)) << FSP_FLAGS_POS_SDI)


/** Return the value of the POST_ANTELOPE field */
#define FSP_FLAGS_GET_POST_ANTELOPE(flags) \
  ((flags & FSP_FLAGS_MASK_POST_ANTELOPE) >> FSP_FLAGS_POS_POST_ANTELOPE)


/** Return the value of the ZIP_SSIZE field */
#define FSP_FLAGS_GET_ZIP_SSIZE(flags) \
  ((flags & FSP_FLAGS_MASK_ZIP_SSIZE) >> FSP_FLAGS_POS_ZIP_SSIZE)


/** Return the value of the ATOMIC_BLOBS field */
#define FSP_FLAGS_HAS_ATOMIC_BLOBS(flags) \
  ((flags & FSP_FLAGS_MASK_ATOMIC_BLOBS) >> FSP_FLAGS_POS_ATOMIC_BLOBS)


/** Return the value of the PAGE_SSIZE field */
#define FSP_FLAGS_GET_PAGE_SSIZE(flags) \
  ((flags & FSP_FLAGS_MASK_PAGE_SSIZE) >> FSP_FLAGS_POS_PAGE_SSIZE)


/** Return the value of the DATA_DIR field */
#define FSP_FLAGS_HAS_DATA_DIR(flags) \
  ((flags & FSP_FLAGS_MASK_DATA_DIR) >> FSP_FLAGS_POS_DATA_DIR)


/** Return the contents of the SHARED field */
#define FSP_FLAGS_GET_SHARED(flags) \
  ((flags & FSP_FLAGS_MASK_SHARED) >> FSP_FLAGS_POS_SHARED)


/** Return the contents of the TEMPORARY field */
#define FSP_FLAGS_GET_TEMPORARY(flags) \
  ((flags & FSP_FLAGS_MASK_TEMPORARY) >> FSP_FLAGS_POS_TEMPORARY)


/** Return the contents of the ENCRYPTION field */
#define FSP_FLAGS_GET_ENCRYPTION(flags) \
  ((flags & FSP_FLAGS_MASK_ENCRYPTION) >> FSP_FLAGS_POS_ENCRYPTION)


/** Return the value of the SDI field */
#define FSP_FLAGS_HAS_SDI(flags) \
  ((flags & FSP_FLAGS_MASK_SDI) >> FSP_FLAGS_POS_SDI)


/** Return the contents of the UNUSED bits */
#define FSP_FLAGS_GET_UNUSED(flags) (flags >> FSP_FLAGS_POS_UNUSED)


/** Return true if flags are not set */
#define FSP_FLAGS_ARE_NOT_SET(flags) ((flags & FSP_FLAGS_MASK) == 0)


/** Set ENCRYPTION bit in tablespace flags */
UNIV_INLINE void fsp_flags_set_encryption(uint32_t &flags) {
  flags |= FSP_FLAGS_MASK_ENCRYPTION;
}


/** Set ENCRYPTION bit in tablespace flags */
UNIV_INLINE void fsp_flags_unset_encryption(uint32_t &flags) {
  flags &= ~FSP_FLAGS_MASK_ENCRYPTION;
}


/** Set SDI Index bit in tablespace flags */
UNIV_INLINE void fsp_flags_set_sdi(uint32_t &flags) {
  flags |= FSP_FLAGS_MASK_SDI;
}


/** Set SDI Index bit in tablespace flags */
UNIV_INLINE void fsp_flags_unset_sdi(uint32_t &flags) {
  flags &= ~FSP_FLAGS_MASK_SDI;
}


/** Use an alias in the code for FSP_FLAGS_GET_SHARED() */
#define fsp_is_shared_tablespace FSP_FLAGS_GET_SHARED


/* @} */
