#include <innodb/dict_mem/dict_mem_table_col_rename_low.h>

#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_sys.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/memory/mem_heap_alloc.h>
#include <innodb/memory/mem_heap_dup.h>
#include <innodb/memory/mem_heap_get_size.h>

/** Renames a column of a table in the data dictionary cache. */
void dict_mem_table_col_rename_low(
    dict_table_t *table, /*!< in/out: table */
    unsigned i,          /*!< in: column offset corresponding to s */
    const char *to,      /*!< in: new column name */
    const char *s,       /*!< in: pointer to table->col_names */
    bool is_virtual)
/*!< in: if this is a virtual column */
{
  char *t_col_names =
      const_cast<char *>(is_virtual ? table->v_col_names : table->col_names);
  ulint n_col = is_virtual ? table->n_v_def : table->n_def;

  size_t from_len = strlen(s), to_len = strlen(to);

  ut_ad(i < table->n_def || is_virtual);
  ut_ad(i < table->n_v_def || !is_virtual);

  ut_ad(from_len <= NAME_LEN);
  ut_ad(to_len <= NAME_LEN);

  if (from_len == to_len) {
    /* The easy case: simply replace the column name in
    table->col_names. */
    strcpy(const_cast<char *>(s), to);
  } else {
    /* We need to adjust all affected index->field
    pointers, as in dict_index_add_col(). First, copy
    table->col_names. */
    ulint prefix_len = s - t_col_names;

    for (; i < n_col; i++) {
      s += strlen(s) + 1;
    }

    ulint full_len = s - t_col_names;
    char *col_names;

    if (to_len > from_len) {
      ulint table_size_before_rename_col = mem_heap_get_size(table->heap);
      col_names = static_cast<char *>(
          mem_heap_alloc(table->heap, full_len + to_len - from_len));
      ulint table_size_after_rename_col = mem_heap_get_size(table->heap);
      if (table_size_before_rename_col != table_size_after_rename_col) {
        dict_sys->size +=
            table_size_after_rename_col - table_size_before_rename_col;
      }
      memcpy(col_names, t_col_names, prefix_len);
    } else {
      col_names = const_cast<char *>(t_col_names);
    }

    memcpy(col_names + prefix_len, to, to_len);
    memmove(col_names + prefix_len + to_len,
            t_col_names + (prefix_len + from_len),
            full_len - (prefix_len + from_len));

    /* Replace the field names in every index. */
    for (dict_index_t *index = table->first_index(); index != NULL;
         index = index->next()) {
      ulint n_fields = dict_index_get_n_fields(index);

      for (ulint i = 0; i < n_fields; i++) {
        dict_field_t *field = index->get_field(i);

        /* if is_virtual and that in field->col does
        not match, continue */
        if ((!is_virtual) != (!field->col->is_virtual())) {
          continue;
        }

        ulint name_ofs = field->name - t_col_names;
        if (name_ofs <= prefix_len) {
          field->name = col_names + name_ofs;
        } else {
          ut_a(name_ofs < full_len);
          field->name = col_names + name_ofs + to_len - from_len;
        }
      }
    }

    if (is_virtual) {
      table->v_col_names = col_names;
    } else {
      table->col_names = col_names;
    }
  }

  /* Virtual columns are not allowed for foreign key */
  if (is_virtual) {
    return;
  }

  dict_foreign_t *foreign;

  /* Replace the field names in every foreign key constraint. */
  for (dict_foreign_set::iterator it = table->foreign_set.begin();
       it != table->foreign_set.end(); ++it) {
    foreign = *it;

    for (unsigned f = 0; f < foreign->n_fields; f++) {
      /* These can point straight to
      table->col_names, because the foreign key
      constraints will be freed at the same time
      when the table object is freed. */
      foreign->foreign_col_names[f] =
          foreign->foreign_index->get_field(f)->name;
    }
  }

  for (dict_foreign_set::iterator it = table->referenced_set.begin();
       it != table->referenced_set.end(); ++it) {
    foreign = *it;

    for (unsigned f = 0; f < foreign->n_fields; f++) {
      /* foreign->referenced_col_names[] need to be
      copies, because the constraint may become
      orphan when foreign_key_checks=0 and the
      parent table is dropped. */

      const char *col_name = foreign->referenced_index->get_field(f)->name;

      if (strcmp(foreign->referenced_col_names[f], col_name)) {
        char **rc = const_cast<char **>(foreign->referenced_col_names + f);
        size_t col_name_len_1 = strlen(col_name) + 1;

        if (col_name_len_1 <= strlen(*rc) + 1) {
          memcpy(*rc, col_name, col_name_len_1);
        } else {
          *rc = static_cast<char *>(
              mem_heap_dup(foreign->heap, col_name, col_name_len_1));
        }
      }
    }
  }
}
