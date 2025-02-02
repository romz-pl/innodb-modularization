#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/Datafile.h>
#include <innodb/tablespace/Tablespace.h>
#include <innodb/tablespace/sys_tablespace_auto_extend_increment.h>
#include <innodb/io/os_offset_t.h>

#ifdef UNIV_HOTBACKUP
#include "srv0srv.h"
#endif



#ifdef UNIV_DEBUG
/** Control if extra debug checks need to be done for temporary tablespace.
Default = true that is disable such checks.
This variable is not exposed to end-user but still kept as variable for
developer to enable it during debug. */
extern bool srv_skip_temp_table_checks_debug;
#endif /* UNIV_DEBUG */

/** Data structure that contains the information about shared tablespaces.
Currently this can be the system tablespace or a temporary table tablespace.
Multi file, shared, system tablespace implementation. */
class SysTablespace : public Tablespace {
 public:
  SysTablespace()
      : m_auto_extend_last_file(),
        m_last_file_size_max(),
        m_created_new_raw(),
        m_is_tablespace_full(false),
        m_sanity_checks_done(false) {
    /* No op */
  }

  ~SysTablespace() { shutdown(); }

  /** Set tablespace full status
  @param[in]	is_full		true if full */
  void set_tablespace_full_status(bool is_full) {
    m_is_tablespace_full = is_full;
  }

  /** Get tablespace full status
  @return true if table is full */
  bool get_tablespace_full_status() { return (m_is_tablespace_full); }

  /** Set sanity check status
  @param[in]	status	true if sanity checks are done */
  void set_sanity_check_status(bool status) { m_sanity_checks_done = status; }

  /** Get sanity check status
  @return true if sanity checks are done */
  bool get_sanity_check_status() { return (m_sanity_checks_done); }

  /** Parse the input params and populate member variables.
  @param	filepath_spec	path to data files
  @param	supports_raw	true if it supports raw devices
  @return true on success parse */
  bool parse_params(const char *filepath_spec, bool supports_raw);

  /** Check the data file specification.
  @param[in]	create_new_db		true if a new database
  is to be created
  @param[in]	min_expected_size	expected tablespace
  size in bytes
  @return DB_SUCCESS if all OK else error code */
  dberr_t check_file_spec(bool create_new_db, ulint min_expected_size);

  /** Free the memory allocated by parse() */
  void shutdown();

  /**
  @return true if a new raw device was created. */
  bool created_new_raw() const { return (m_created_new_raw); }

  /**
  @return auto_extend value setting */
  ulint can_auto_extend_last_file() const { return (m_auto_extend_last_file); }

  /** Set the last file size.
  @param[in]	size	the size to set */
  void set_last_file_size(page_no_t size) {
    ut_ad(!m_files.empty());
    m_files.back().m_size = size;
  }

  /** Get the number of pages in the last data file in the tablespace
  @return the size of the last data file in the array */
  page_no_t last_file_size() const {
    ut_ad(!m_files.empty());
    return (m_files.back().m_size);
  }

  /**
  @return the autoextend increment in pages. */
  page_no_t get_autoextend_increment() const {
    return (sys_tablespace_auto_extend_increment *
            ((1024 * 1024) / UNIV_PAGE_SIZE));
  }

  /** Round the number of bytes in the file to MegaBytes
  and then return the number of pages.
  Note: Only system tablespaces are required to be at least
  1 megabyte.
  @return the number of pages in the file. */
  page_no_t get_pages_from_size(os_offset_t size) {
    return static_cast<page_no_t>(
        ((size / (1024 * 1024)) * ((1024 * 1024) / UNIV_PAGE_SIZE)));
  }

  /**
  @return next increment size */
  page_no_t get_increment() const;

  /** Open or create the data files
  @param[in]  is_temp		whether this is a temporary tablespace
  @param[in]  create_new_db	whether we are creating a new database
  @param[out] sum_new_sizes	sum of sizes of the new files added
  @param[out] flush_lsn		FIL_PAGE_FILE_FLUSH_LSN of first file
  @return DB_SUCCESS or error code */
  dberr_t open_or_create(bool is_temp, bool create_new_db,
                         page_no_t *sum_new_sizes, lsn_t *flush_lsn)
      MY_ATTRIBUTE((warn_unused_result));

 private:
  /** Check the tablespace header for this tablespace.
  @param[out]	flushed_lsn	the value of FIL_PAGE_FILE_FLUSH_LSN
  @return DB_SUCCESS or error code */
  dberr_t read_lsn_and_check_flags(lsn_t *flushed_lsn);

  /** Note that the data file was not found.
  @param[in]	file		data file object
  @param[in]	create_new_db	true if a new instance to be created
  @return DB_SUCCESS or error code */
  dberr_t file_not_found(Datafile &file, bool create_new_db);

  /** Note that the data file was found.
  @param[in,out]	file	data file object */
  void file_found(Datafile &file);

  /** Create a data file.
  @param[in,out]	file	data file object
  @return DB_SUCCESS or error code */
  dberr_t create(Datafile &file);

  /** Create a data file.
  @param[in,out]	file	data file object
  @return DB_SUCCESS or error code */
  dberr_t create_file(Datafile &file);

  /** Open a data file.
  @param[in,out]	file	data file object
  @return DB_SUCCESS or error code */
  dberr_t open_file(Datafile &file);

  /** Set the size of the file.
  @param[in,out]	file	data file object
  @return DB_SUCCESS or error code */
  dberr_t set_size(Datafile &file);

 private:
  /* Put the pointer to the next byte after a valid file name.
  Note that we must step over the ':' in a Windows filepath.
  A Windows path normally looks like C:\ibdata\ibdata1:1G, but
  a Windows raw partition may have a specification like
  \\.\C::1Gnewraw or \\.\PHYSICALDRIVE2:1Gnewraw.
  @param[in]	str		system tablespace file path spec
  @return next character in string after the file name */
  static char *parse_file_name(char *ptr);

  /** Convert a numeric string that optionally ends in upper or lower
  case G, M, or K, rounding off to the nearest number of megabytes.
  Then return the number of pages in the file.
  @param[in,out]	ptr	Pointer to a numeric string
  @return the number of pages in the file. */
  page_no_t parse_units(char *&ptr);

  enum file_status_t {
    FILE_STATUS_VOID = 0,              /** status not set */
    FILE_STATUS_RW_PERMISSION_ERROR,   /** permission error */
    FILE_STATUS_READ_WRITE_ERROR,      /** not readable/writable */
    FILE_STATUS_NOT_REGULAR_FILE_ERROR /** not a regular file */
  };

  /** Verify the size of the physical file
  @param[in]	file	data file object
  @return DB_SUCCESS if OK else error code. */
  dberr_t check_size(Datafile &file);

  /** Check if a file can be opened in the correct mode.
  @param[in,out]	file	data file object
  @param[out]	reason	exact reason if file_status check failed.
  @return DB_SUCCESS or error code. */
  dberr_t check_file_status(const Datafile &file, file_status_t &reason);

  /* DATA MEMBERS */

  /** if true, then we auto-extend the last data file */
  bool m_auto_extend_last_file;

  /** if != 0, this tells the max size auto-extending may increase the
  last data file size */
  page_no_t m_last_file_size_max;

  /** If the following is true we do not allow
  inserts etc. This protects the user from forgetting
  the 'newraw' keyword to my.cnf */
  bool m_created_new_raw;

  /** Tablespace full status */
  bool m_is_tablespace_full;

  /** if false, then sanity checks are still pending */
  bool m_sanity_checks_done;
};
