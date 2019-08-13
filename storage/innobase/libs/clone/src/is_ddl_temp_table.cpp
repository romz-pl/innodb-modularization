#include <innodb/clone/is_ddl_temp_table.h>

#include <innodb/tablespace/fil_node_t.h>
#include <innodb/dict_types/flags.h>

/** Check if the tablespace file is temporary file created by DDL
@param[in]	node	file node
@return true if created by DDL */
bool is_ddl_temp_table(fil_node_t *node) {
  const char *name_ptr;

  name_ptr = strrchr(node->name, OS_PATH_SEPARATOR);

  if (name_ptr == nullptr) {
    name_ptr = node->name;
  } else {
    name_ptr++;
  }

  /* Check if it is a temporary table created by DDL. This is work
  around to identify concurrent DDL till server provides MDL lock
  for blocking DDL. */
  if (strncmp(name_ptr, TEMP_FILE_PREFIX, TEMP_FILE_PREFIX_LENGTH) == 0) {
    return (true);
  }

  return (false);
}
