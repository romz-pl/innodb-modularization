#include <innodb/lock_priv/lock_t.h>

#include <innodb/lock_types/lock_mode_string.h>

#include <sstream>
#include <ostream>

/** Convert the member 'type_mode' into a human readable string.
@return human readable string */
std::string lock_t::type_mode_string() const {
  std::ostringstream sout;
  sout << type_string();
  sout << " | " << lock_mode_string(mode());

  if (is_record_not_gap()) {
    sout << " | LOCK_REC_NOT_GAP";
  }

  if (is_waiting()) {
    sout << " | LOCK_WAIT";
  }

  if (is_gap()) {
    sout << " | LOCK_GAP";
  }

  if (is_insert_intention()) {
    sout << " | LOCK_INSERT_INTENTION";
  }
  return (sout.str());
}


std::ostream &lock_t::print(std::ostream &out) const {
  out << "[lock_t: type_mode=" << type_mode << "(" << type_mode_string() << ")";

  if (is_record_lock()) {
    out << rec_lock;
  } else {
    out << tab_lock;
  }

  out << "]";
  return (out);
}
