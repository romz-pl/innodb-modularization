#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_no_t.h>
#include <innodb/log_sn/lsn_t.h>

#include <map>
#include <mutex>
#include <memory>


#ifndef UNIV_HOTBACKUP

/** It is a environment for tests of redo log. It contains a mock, which
replaces real buffer pool during the test. */
class Log_test {
 public:
  typedef page_no_t Key;
  typedef int64_t Value;

  struct Page {
    Key key;
    Value value;
    lsn_t oldest_modification;
    lsn_t newest_modification;
  };

  typedef std::map<Key, Page> Pages;

  class Sync_point {
   public:
    virtual void sync() = 0;

    virtual ~Sync_point() {}
  };

  enum class Options {
    VALIDATE_RECENT_CLOSED = 1,
    VALIDATE_RECENT_WRITTEN = 2
  };

  typedef std::map<std::string, std::unique_ptr<Sync_point>> Sync_points;

  /** Calculates oldest_modification of the earliest added dirty page
  during the test in log0log-t. It is basically a replacement for the
  log_buf_get_oldest_modification_approx() during the test.
  @return oldest_modification lsn */
  lsn_t oldest_modification_approx() const;

  void add_dirty_page(const Page &page);

  void fsync_written_pages();

  void purge(lsn_t max_dirty_page_age);

  byte *create_mlog_rec(byte *rec, Key key, Value value);

  byte *parse_mlog_rec(byte *begin, byte *end);

  const Pages &flushed() const;

  const Pages &recovered() const;

  void sync_point(const std::string &sync_point_name);

  void register_sync_point_handler(
      const std::string &sync_point_name,
      std::unique_ptr<Sync_point> &&sync_point_handler);

  bool enabled(Options option) const;

  void set_enabled(Options option, bool enabled);

  int flush_every() const;

  void set_flush_every(int flush_every);

  int verbosity() const;

  void set_verbosity(int level);

 private:
  void recovered_reset(Key key, lsn_t oldest_modification,
                       lsn_t newest_modification);

  void recovered_add(Key key, Value value, lsn_t oldest_modification,
                     lsn_t newest_modification);

  mutable std::mutex m_mutex;
  mutable std::mutex m_purge_mutex;
  std::map<lsn_t, Page> m_buf;
  Pages m_written;
  Pages m_flushed;
  Pages m_recovered;
  Sync_points m_sync_points;
  uint64_t m_options_enabled = 0;
  int m_verbosity = 0;
  int m_flush_every = 10;
};

#endif /* !UNIV_HOTBACKUP */
