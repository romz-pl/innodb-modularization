#include <innodb/cmp/cmp_geometry_field.h>

#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>

/** Innobase uses this function to compare two geometry data fields
 @return	1, 0, -1, if a is greater, equal, less than b, respectively */
int cmp_geometry_field(ulint mtype,           /*!< in: main type */
                              ulint prtype,          /*!< in: precise type */
                              const byte *a,         /*!< in: data field */
                              unsigned int a_length, /*!< in: data field length,
                                                     not UNIV_SQL_NULL */
                              const byte *b,         /*!< in: data field */
                              unsigned int b_length) /*!< in: data field length,
                                                     not UNIV_SQL_NULL */
{
  double x1, x2;
  double y1, y2;

  ut_ad(prtype & DATA_GIS_MBR);

  if (a_length < sizeof(double) || b_length < sizeof(double)) {
    return (0);
  }

  /* Try to compare mbr left lower corner (xmin, ymin) */
  x1 = mach_double_read(a);
  x2 = mach_double_read(b);
  y1 = mach_double_read(a + sizeof(double) * SPDIMS);
  y2 = mach_double_read(b + sizeof(double) * SPDIMS);

  if (x1 > x2) {
    return (1);
  } else if (x2 > x1) {
    return (-1);
  }

  if (y1 > y2) {
    return (1);
  } else if (y2 > y1) {
    return (-1);
  }

  /* left lower corner (xmin, ymin) overlaps, now right upper corner */
  x1 = mach_double_read(a + sizeof(double));
  x2 = mach_double_read(b + sizeof(double));
  y1 = mach_double_read(a + sizeof(double) * SPDIMS + sizeof(double));
  y2 = mach_double_read(b + sizeof(double) * SPDIMS + sizeof(double));

  if (x1 > x2) {
    return (1);
  } else if (x2 > x1) {
    return (-1);
  }

  if (y1 > y2) {
    return (1);
  } else if (y2 > y1) {
    return (-1);
  }

  return (0);
}

