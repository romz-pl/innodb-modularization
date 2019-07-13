#include <innodb/tablespace/fil_space_open_if_needed.h>

#include <innodb/tablespace/fil_space_get_size.h>
#include <innodb/tablespace/fil_space_t.h>

/** During crash recovery, open a tablespace if it had not been opened
yet, to get valid size and flags.
@param[in,out]	space		Tablespace instance */
void fil_space_open_if_needed(fil_space_t *space) {
  if (space->size == 0) {
    /* Initially, size and flags will be set to 0,
    until the files are opened for the first time.
    fil_space_get_size() will open the file
    and adjust the size and flags. */
    page_no_t size = fil_space_get_size(space->id);

    ut_a(size == space->size);
  }
}
