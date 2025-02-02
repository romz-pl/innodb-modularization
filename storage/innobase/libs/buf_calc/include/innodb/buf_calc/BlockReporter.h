#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_size_t.h>
#include <innodb/page/page_id_t.h>
#include <innodb/buf_calc/srv_checksum_algorithm_t.h>

/** Class to print checksums to log file. */
class BlockReporter {
 public:
  /** Constructor
  @param[in]	check_lsn	check lsn of the page with the
                                  current lsn (only in recovery)
  @param[in]	read_buf	buffer holding the page
  @param[in]	page_size	page size
  @param[in]	skip_checksum	skip checksum verification */
  BlockReporter(bool check_lsn, const byte *read_buf,
                const page_size_t &page_size, bool skip_checksum)
      : m_check_lsn(check_lsn),
        m_read_buf(read_buf),
        m_page_size(page_size),
        m_skip_checksum(skip_checksum) {}

  virtual ~BlockReporter() {}
  BlockReporter(const BlockReporter &) = default;

  /** Checks if a page is corrupt.
  @retval	true	if page is corrupt
  @retval	false	if page is not corrupt */
  bool MY_ATTRIBUTE((warn_unused_result)) is_corrupted() const;

  /** Print message if page is empty.
  @param[in]	empty		true if page is empty */
  virtual inline void report_empty_page(bool empty) const {}

  /** Print crc32 checksum and the checksum fields in page.
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer
  @param[in]	crc32		Calculated crc32 checksum
  @param[in]	algo		Current checksum algorithm */
  virtual inline void print_strict_crc32(ulint checksum_field1,
                                         ulint checksum_field2, uint32_t crc32,
                                         srv_checksum_algorithm_t algo) const {}

  /** Print innodb checksum and the checksum fields in page.
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer */
  virtual inline void print_strict_innodb(ulint checksum_field1,
                                          ulint checksum_field2) const {}

  /** Print none checksum and the checksum fields in page.
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer
  @param[in]	algo		Current checksum algorithm */
  virtual inline void print_strict_none(ulint checksum_field1,
                                        ulint checksum_field2,
                                        srv_checksum_algorithm_t algo) const {}

  /** Print innodb checksum value stored in page trailer.
  @param[in]	old_checksum	checksum value according to old style
  @param[in]	new_checksum	checksum value according to new style
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer
  @param[in]	algo		current checksum algorithm */
  virtual inline void print_innodb_checksum(
      ulint old_checksum, ulint new_checksum, ulint checksum_field1,
      ulint checksum_field2, srv_checksum_algorithm_t algo) const {}

  /** Print the message that checksum mismatch happened in
  page header. */
  virtual inline void print_innodb_fail() const {}

  /** Print both new-style, old-style & crc32 checksum values.
  @param[in]	checksum_field1	Checksum in page header
  @param[in]	checksum_field2	Checksum in page trailer */
  virtual inline void print_crc32_checksum(ulint checksum_field1,
                                           ulint checksum_field2) const {}

  /** Print a message that crc32 check failed. */
  virtual inline void print_crc32_fail() const {}

  /** Print a message that none check failed. */
  virtual inline void print_none_fail() const {}

  /** Print checksum values on a compressed page.
  @param[in]	calc	the calculated checksum value
  @param[in]	stored	the stored checksum in header. */
  virtual inline void print_compressed_checksum(ib_uint32_t calc,
                                                ib_uint32_t stored) const {}

  /** Verify a compressed page's checksum.
  @retval		true		if stored checksum is valid
  according to the value of srv_checksum_algorithm
  @retval		false		if stored schecksum is not valid
  according to the value of srv_checksum_algorithm */
  bool verify_zip_checksum() const;

  /** Calculate the compressed page checksum. This variant
  should be used when only the page_size_t is unknown and
  only physical page_size of compressed page is available.
  @param[in]	read_buf		buffer holding the page
  @param[in]	phys_page_size		physical page size
  @param[in]	algo			checksum algorithm to use
  @param[in]	use_legacy_big_endian	only used if algo is
  SRV_CHECKSUM_ALGORITHM_CRC32 or SRV_CHECKSUM_ALGORITHM_STRICT_CRC32 -
  if true then use big endian byteorder when converting byte strings to
  integers.
  @return page checksum */
  uint32_t calc_zip_checksum(const byte *read_buf, ulint phys_page_size,
                             srv_checksum_algorithm_t algo,
                             bool use_legacy_big_endian = false) const;

  /** Calculate the compressed page checksum.
  @param[in]	algo			checksum algorithm to use
  @param[in]	use_legacy_big_endian	only used if algo is
  SRV_CHECKSUM_ALGORITHM_CRC32 or SRV_CHECKSUM_ALGORITHM_STRICT_CRC32 -
  if true then use big endian byteorder when converting byte strings to
  integers.
  @return page checksum */
  uint32_t calc_zip_checksum(srv_checksum_algorithm_t algo,
                             bool use_legacy_big_endian = false) const;

 private:
  /** Checks if the page is in innodb checksum format.
  @param[in]	checksum_field1	new checksum field
  @param[in]	checksum_field2	old checksum field
  @param[in]	algo		current checksum algorithm
  @return true if the page is in innodb checksum format. */
  bool is_checksum_valid_innodb(ulint checksum_field1, ulint checksum_field2,
                                const srv_checksum_algorithm_t algo) const;

  /** Checks if the page is in none checksum format.
  @param[in]	checksum_field1	new checksum field
  @param[in]	checksum_field2	old checksum field
  @param[in]	algo		current checksum algorithm
  @return true if the page is in none checksum format. */
  bool is_checksum_valid_none(ulint checksum_field1, ulint checksum_field2,
                              const srv_checksum_algorithm_t algo) const;

  /** Checks if the page is in crc32 checksum format.
  @param[in]	checksum_field1		new checksum field
  @param[in]	checksum_field2		old checksum field
  @param[in]	algo			current checksum algorithm
  @param[in]	use_legacy_big_endian	big endian algorithm
  @return true if the page is in crc32 checksum format. */
  bool is_checksum_valid_crc32(ulint checksum_field1, ulint checksum_field2,
                               const srv_checksum_algorithm_t algo,
                               bool use_legacy_big_endian) const;

  /** Issue a warning when the checksum that is stored in the page is
  valid, but different than the global setting innodb_checksum_algorithm.
  @param[in]	curr_algo	current checksum algorithm
  @param[in]	page_checksum	page valid checksum
  @param[in]	page_id		page identifier */
  void page_warn_strict_checksum(srv_checksum_algorithm_t curr_algo,
                                 srv_checksum_algorithm_t page_checksum,
                                 const page_id_t &page_id) const;

 protected:
  /** If true, do a LSN check during innodb recovery. */
  bool m_check_lsn;
  /** Buffer holding the page. */
  const byte *m_read_buf;
  /** Page size. */
  const page_size_t &m_page_size;
  /** Skip checksum verification but compare only data. */
  bool m_skip_checksum;
};
