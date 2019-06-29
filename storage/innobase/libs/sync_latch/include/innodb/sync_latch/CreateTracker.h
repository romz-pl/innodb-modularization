#pragma once

#include <innodb/univ/univ.h>
#include <innodb/assert/assert.h>
#include <innodb/allocator/ut_allocator.h>
#include <innodb/sync_os/OSMutex.h>
#include <innodb/sync_latch/sync_basename.h>

#include <string>
#include <map>

/** Track mutex file creation name and line number. This is to avoid storing
{ const char* name; uint16_t line; } in every instance. This results in the
sizeof(Mutex) > 64. We use a lookup table to store it separately. Fetching
the values is very rare, only required for diagnostic purposes. And, we
don't create/destroy mutexes that frequently. */
struct CreateTracker {
  /** Constructor */
  CreateTracker() UNIV_NOTHROW { m_mutex.init(); }

  /** Destructor */
  ~CreateTracker() UNIV_NOTHROW {
    ut_d(m_files.empty());

    m_mutex.destroy();
  }

  /** Register where the latch was created
  @param[in]	ptr		Latch instance
  @param[in]	filename	Where created
  @param[in]	line		Line number in filename */
  void register_latch(const void *ptr, const char *filename,
                      uint16_t line) UNIV_NOTHROW {
    m_mutex.enter();

    Files::iterator lb = m_files.lower_bound(ptr);

    ut_ad(lb == m_files.end() || m_files.key_comp()(ptr, lb->first));

    typedef Files::value_type value_type;

    m_files.insert(lb, value_type(ptr, File(filename, line)));

    m_mutex.exit();
  }

  /** Deregister a latch - when it is destroyed
  @param[in]	ptr		Latch instance being destroyed */
  void deregister_latch(const void *ptr) UNIV_NOTHROW {
    m_mutex.enter();

    Files::iterator lb = m_files.lower_bound(ptr);

    ut_ad(lb != m_files.end() && !(m_files.key_comp()(ptr, lb->first)));

    m_files.erase(lb);

    m_mutex.exit();
  }

  /** Get the create string, format is "name:line"
  @param[in]	ptr		Latch instance
  @return the create string or "" if not found */
  std::string get(const void *ptr) UNIV_NOTHROW {
    m_mutex.enter();

    std::string created;

    Files::iterator lb = m_files.lower_bound(ptr);

    if (lb != m_files.end() && !(m_files.key_comp()(ptr, lb->first))) {
      std::ostringstream msg;

      msg << lb->second.m_name << ":" << lb->second.m_line;

      created = msg.str();
    }

    m_mutex.exit();

    return (created);
  }

 private:
  /** For tracking the filename and line number */
  struct File {
    /** Constructor */
    File() UNIV_NOTHROW : m_name(), m_line() {}

    /** Constructor
    @param[in]	name		Filename where created
    @param[in]	line		Line number where created */
    File(const char *name, uint16_t line) UNIV_NOTHROW
        : m_name(sync_basename(name)),
          m_line(line) {
      /* No op */
    }

    /** Filename where created */
    std::string m_name;

    /** Line number where created */
    uint16_t m_line;
  };

  /** Map the mutex instance to where it was created */
  typedef std::map<const void *, File, std::less<const void *>,
                   ut_allocator<std::pair<const void *const, File>>>
      Files;

  typedef OSMutex Mutex;

  /** Mutex protecting m_files */
  Mutex m_mutex;

  /** Track the latch creation */
  Files m_files;
};
