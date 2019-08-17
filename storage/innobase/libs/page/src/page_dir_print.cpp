#include <innodb/page/page_dir_print.h>


#ifndef UNIV_HOTBACKUP
#ifdef UNIV_BTR_PRINT

/** This is used to print the contents of the directory for
 debugging purposes. */
void page_dir_print(page_t *page, /*!< in: index page */
                    ulint pr_n)   /*!< in: print n first and n last entries */
{
  ulint n;
  ulint i;
  page_dir_slot_t *slot;

  n = page_dir_get_n_slots(page);

  fprintf(stderr,
          "--------------------------------\n"
          "PAGE DIRECTORY\n"
          "Page address %p\n"
          "Directory stack top at offs: %lu; number of slots: %lu\n",
          page, (ulong)page_offset(page_dir_get_nth_slot(page, n - 1)),
          (ulong)n);
  for (i = 0; i < n; i++) {
    slot = page_dir_get_nth_slot(page, i);
    if ((i == pr_n) && (i < n - pr_n)) {
      fputs("    ...   \n", stderr);
    }
    if ((i < pr_n) || (i >= n - pr_n)) {
      fprintf(stderr,
              "Contents of slot: %lu: n_owned: %lu,"
              " rec offs: %lu\n",
              (ulong)i, (ulong)page_dir_slot_get_n_owned(slot),
              (ulong)page_offset(page_dir_slot_get_rec(slot)));
    }
  }
  fprintf(stderr,
          "Total of %lu records\n"
          "--------------------------------\n",
          (ulong)(PAGE_HEAP_NO_USER_LOW + page_get_n_recs(page)));
}

#endif                               /* UNIV_BTR_PRINT */
#endif                               /* !UNIV_HOTBACKUP */
