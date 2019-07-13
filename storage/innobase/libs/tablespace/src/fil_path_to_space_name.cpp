#include <innodb/tablespace/fil_path_to_space_name.h>

#include <innodb/io/Fil_path.h>
#include <innodb/allocator/mem_strdupl.h>

/** Convert a file name to a tablespace name. Strip the file name
prefix and suffix, leaving only databasename/tablename.
@param[in]	filename	directory/databasename/tablename.ibd
@return database/tablename string, to be freed with ut_free() */
char *fil_path_to_space_name(const char *filename) {
  std::string path{filename};
  auto pos = path.find_last_of(Fil_path::SEPARATOR);

  ut_a(pos != std::string::npos && !Fil_path::is_separator(path.back()));

  std::string db_name = path.substr(0, pos);
  std::string space_name = path.substr(pos + 1, path.length());

  /* If it is a path such as a/b/c.ibd, ignore everything before 'b'. */
  pos = db_name.find_last_of(Fil_path::SEPARATOR);

  if (pos != std::string::npos) {
    db_name = db_name.substr(pos + 1);
  }

  char *name;

  if (Fil_path::has_suffix(IBD, space_name)) {
    /* fil_space_t::name always uses '/' . */

    path = db_name;
    path.push_back('/');

    /* Strip the ".ibd" suffix. */
    path.append(space_name.substr(0, space_name.length() - 4));

    name = mem_strdupl(path.c_str(), path.length());

  } else {
    /* Must have an "undo" prefix. */
    ut_ad(space_name.find("undo") == 0);

    name = mem_strdupl(space_name.c_str(), space_name.length());
  }

  return (name);
}
