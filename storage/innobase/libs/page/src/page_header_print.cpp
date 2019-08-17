#include <innodb/page/page_header_print.h>


#ifndef UNIV_HOTBACKUP
#ifdef UNIV_BTR_PRINT

/** Prints the info in a page header. */
void page_header_print(const page_t *page) {
  fprintf(stderr,
          "--------------------------------\n"
          "PAGE HEADER INFO\n"
          "Page address %p, n records %lu (%s)\n"
          "n dir slots %lu, heap top %lu\n"
          "Page n heap %lu, free %lu, garbage %lu\n"
          "Page last insert %lu, direction %lu, n direction %lu\n",
          page, (ulong)page_header_get_field(page, PAGE_N_RECS),
          page_is_comp(page) ? "compact format" : "original format",
          (ulong)page_header_get_field(page, PAGE_N_DIR_SLOTS),
          (ulong)page_header_get_field(page, PAGE_HEAP_TOP),
          (ulong)page_dir_get_n_heap(page),
          (ulong)page_header_get_field(page, PAGE_FREE),
          (ulong)page_header_get_field(page, PAGE_GARBAGE),
          (ulong)page_header_get_field(page, PAGE_LAST_INSERT),
          (ulong)page_header_get_field(page, PAGE_DIRECTION),
          (ulong)page_header_get_field(page, PAGE_N_DIRECTION));
}



#endif                               /* UNIV_BTR_PRINT */
#endif                               /* !UNIV_HOTBACKUP */
