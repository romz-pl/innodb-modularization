#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/persistent_type_t.h>
#include <innodb/allocator/ut_allocator.h>

#include <map>

class Persister;
class PersistentTableMetadata;

/** Container of persisters used in the system. Currently we don't need
to protect this object since we only initialize it at very beginning and
destroy it in the end. During the server running, we only get the persisters */
class Persisters {
  typedef std::map<
      persistent_type_t, Persister *, std::less<persistent_type_t>,
      ut_allocator<std::pair<const persistent_type_t, Persister *>>>
      persisters_t;

 public:
  /** Constructor */
  Persisters() : m_persisters() {}

  /** Destructor */
  ~Persisters();

  /** Get the persister object with specified type
  @param[in]	type	persister type
  @return Persister object required or NULL if not found */
  Persister *get(persistent_type_t type) const;

  /** Add a specified persister of type, we will allocate the Persister
  if there is no such persister exist, otherwise do nothing and return
  the existing one
  @param[in]	type	persister type
  @return the persister of type */
  Persister *add(persistent_type_t type);

  /** Remove a specified persister of type, we will free the Persister
  @param[in]	type	persister type */
  void remove(persistent_type_t type);

  /** Serialize the metadata to a buffer
  @param[in]	metadata	metadata to serialize
  @param[out]	buffer		buffer to store the serialized metadata
  @return the length of serialized metadata */
  size_t write(PersistentTableMetadata &metadata, byte *buffer);

 private:
  /** A map to store all persisters needed */
  persisters_t m_persisters;
};
