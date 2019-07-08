#pragma once

#include <innodb/univ/univ.h>

#include <innodb/monitor/monitor_type_t.h>
#include <innodb/monitor/monitor_id_t.h>

/** struct monitor_info describes the basic/static information
about each monitor counter. */
struct monitor_info_t {
  const char *monitor_name;        /*!< Monitor name */
  const char *monitor_module;      /*!< Sub Module the monitor
                                   belongs to */
  const char *monitor_desc;        /*!< Brief desc of monitor counter */
  monitor_type_t monitor_type;     /*!< Type of Monitor Info */
  monitor_id_t monitor_related_id; /*!< Monitor ID of counter that
                                related to this monitor. This is
                                set when the monitor belongs to
                                a "monitor set" */
  monitor_id_t monitor_id;         /*!< Monitor ID as defined in enum
                                   monitor_id_t */
};
