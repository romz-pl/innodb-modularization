#include <innodb/clone/fix_cloned_tables.h>

#include <innodb/logger/error.h>
#include <innodb/logger/info.h>
#include <innodb/pfs/PFS_NOT_INSTRUMENTED.h>
#include <innodb/thread/os_thread_create.h>

#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dictionary.h"
#include "sql/dd/impl/utils.h"
#include "sql/dd/types/schema.h"
#include "sql/dd/types/table.h"
#include "sql/mysqld.h"
#include "sql/sql_class.h"
#include "sql/sql_plugin.h"
#include "sql/sql_thd_internal_api.h"
#include "sql/strfunc.h"

using DD_schemas = std::vector<const dd::Schema *>;
using Const_schema_iter = DD_schemas::const_iterator;

using DD_tables = std::vector<const dd::Table *>;
using Const_table_iter = DD_tables::const_iterator;

using Releaser = dd::cache::Dictionary_client::Auto_releaser;

#include <atomic>

namespace {

/** Fix schema and table for non-InnoDB SE */
class Fixup_tables {
 public:
  /** Constructor */
  Fixup_tables() : m_num_tasks() { m_num_errors.store(0); }

  /** Fix tables for which data is not cloned.
  @param[in,out]	thd	current	THD
  @param[in]		tables	tables from DD
  @return true if error */
  bool fix(THD *thd, const DD_tables &tables) {
    set_num_tasks(tables.size());

    using namespace std::placeholders;

    auto fixup_function =
        std::bind(&Fixup_tables::fix_tables, this, thd, _1, _2, _3);
    par_for(PFS_NOT_INSTRUMENTED, tables, get_num_tasks(), fixup_function);

    return (failed());
  }

  /** Fix schema for which data directory is not created.
  @param[in]	schemas	schemas from DD
  @return true if error */
  bool fix(const DD_schemas &schemas) {
    set_num_tasks(schemas.size());

    using namespace std::placeholders;

    auto fixup_function =
        std::bind(&Fixup_tables::fix_schemas, this, _1, _2, _3);
    par_for(PFS_NOT_INSTRUMENTED, schemas, get_num_tasks(), fixup_function);

    return (failed());
  }

 private:
  /** @return number of tasks. */
  size_t get_num_tasks() const { return (m_num_tasks); }

  /** Calculate and set number of new tasks to spawn.
  @param[in]	num_entries	number of entries to handle */
  void set_num_tasks(size_t num_entries) {
    /* Have one task for every 50,000 entries. */
    m_num_tasks = num_entries / 50000;

#ifdef UNIV_DEBUG
    /* Test operation in newly spawned thread. */
    if (m_num_tasks == 0) {
      ++m_num_tasks;
    }
#endif /* UNIV_DEBUG */

    /* Don't go beyond 8 threads for now. */
    if (m_num_tasks > 8) {
      m_num_tasks = 8;
    }
    m_num_errors.store(0);
  }

  /** @return true, if any thread has failed. */
  bool failed() const { return (m_num_errors.load() != 0); }

  /** Check and create empty table if table data is not there.
  @param[in,out]	thd		current	THD
  @param[in]		begin		first element in current slice
  @param[in]		end		last element in current slice
  @param[in]		thread_number	current thread number. */
  void fix_tables(THD *thd, const Const_table_iter &begin,
                  const Const_table_iter &end, size_t thread_number);

  /** Check and create schema directory if not there.
  @param[in]	begin		first element in current slice
  @param[in]	end		last element in current slice
  @param[in]	thread_number	current thread number. */
  void fix_schemas(const Const_schema_iter &begin, const Const_schema_iter &end,
                   size_t thread_number);

 private:
  /** Number of tasks failed. */
  std::atomic_size_t m_num_errors;

  /** Number of tasks. */
  size_t m_num_tasks;
};

void Fixup_tables::fix_tables(THD *thd, const Const_table_iter &begin,
                              const Const_table_iter &end,
                              size_t thread_number) {
  ib::info(ER_IB_MSG_151) << "Clone: Fix Tables count: " << (end - begin)
                          << " task: " << thread_number;

  bool thread_created = false;

  /* For newly spawned threads, create server THD */
  if (thread_number != get_num_tasks()) {
    my_thread_init();
    thd = create_thd(false, true, true, PSI_NOT_INSTRUMENTED);
    /* No MDL locks and privilege check during initialization phase. */
    thd->system_thread = SYSTEM_THREAD_DD_INITIALIZE;
    thd->security_context()->skip_grants();
    thread_created = true;
  }

  for (auto it = begin; it != end; ++it) {
    const auto &table = *it;
    auto se = ha_resolve_by_name_raw(thd, lex_cstring_handle(table->engine()));

    auto se_type = ha_legacy_type(se ? plugin_data<handlerton *>(se) : nullptr);

    plugin_unlock(thd, se);

    if (se_type != DB_TYPE_MYISAM && se_type != DB_TYPE_CSV_DB) {
      continue;
    }

    auto dc = dd::get_dd_client(thd);
    Releaser releaser(dc);

    const dd::Schema *table_schema = nullptr;

    if (dc->acquire(table->schema_id(), &table_schema)) {
      ++m_num_errors;
      break;
    }

    const auto schema_name = table_schema->name().c_str();
    const auto table_name = table->name().c_str();

    char sql_stmt[FN_LEN + FN_LEN + 64];

    snprintf(sql_stmt, sizeof(sql_stmt), "TRUNCATE TABLE %s.%s", schema_name,
             table_name);

    if (dd::execute_query(thd, sql_stmt)) {
      ++m_num_errors;
      ib::error(ER_IB_MSG_150)
          << "Clone: Failed to fix Table: " << schema_name << "." << table_name
          << " task: " << thread_number;
      break;
    }

    ib::info(ER_IB_MSG_151) << "Clone: Fixed Table: " << schema_name << "."
                            << table_name << " task: " << thread_number;
  }

  if (thread_created) {
    thd->system_thread = SYSTEM_THREAD_BACKGROUND;
    destroy_thd(thd);
    my_thread_end();
  }
}

void Fixup_tables::fix_schemas(const Const_schema_iter &begin,
                               const Const_schema_iter &end,
                               size_t thread_number) {
  ib::info(ER_IB_MSG_151) << "Clone Fix [Schema] count: " << (end - begin)
                          << " task: " << thread_number;
  for (auto it = begin; it != end; ++it) {
    const auto &schema = *it;

    const auto schema_dir = schema->name().c_str();

    MY_STAT stat_info;

    if (mysql_file_stat(key_file_misc, schema_dir, &stat_info, MYF(0)) !=
        nullptr) {
      /* Schema directory exists */
      continue;
    }

    if (0 == strcmp(schema_dir, "information_schema")) {
      /* Information schema has no directory */
      continue;
    }

    if (my_mkdir(schema_dir, 0777, MYF(0)) < 0) {
      ib::error(ER_IB_MSG_150)
          << "Clone: Failed to create schema directory: " << schema_dir
          << " task: " << thread_number;
      ++m_num_errors;
      break;
    }

    ib::info(ER_IB_MSG_151)
        << "Clone: Fixed Schema: " << schema_dir << " task: " << thread_number;
  }
}
} /* namespace */



bool fix_cloned_tables(THD *thd) {
  auto dc = dd::get_dd_client(thd);
  Releaser releaser(dc);

  DD_schemas schemas;
  if (dc->fetch_global_components(&schemas)) {
    return (true);
  }

  Fixup_tables clone_fixup;

  ib::info(ER_IB_MSG_151) << "Clone Fixup: check and create schema directory";

  if (clone_fixup.fix(schemas)) {
    return (true);
  }

  DD_tables tables;
  if (dc->fetch_global_components(&tables)) {
    return (true);
  }

  ib::info(ER_IB_MSG_151) << "Clone Fixup: create empty MyIsam and CSV tables";

  if (clone_fixup.fix(thd, tables)) {
    return (true);
  }

  ib::info(ER_IB_MSG_151) << "Clone Fixup: finished successfully";

  return (false);
}
