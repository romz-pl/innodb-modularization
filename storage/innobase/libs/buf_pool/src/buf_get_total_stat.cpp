#include <innodb/buf_pool/buf_get_total_stat.h>

#include <innodb/buf_pool/buf_pool_stat_t.h>
#include <innodb/buf_pool/buf_pool_from_array.h>
#include <innodb/buf_pool/buf_pool_t.h>

#include <string.h>

/** Requested number of buffer pool instances */
extern ulong srv_buf_pool_instances;

/** Get total buffer pool statistics. */
void buf_get_total_stat(
    buf_pool_stat_t *tot_stat) /*!< out: buffer pool stats */
{
  ulint i;

  memset(tot_stat, 0, sizeof(*tot_stat));

  for (i = 0; i < srv_buf_pool_instances; i++) {
    buf_pool_stat_t *buf_stat;
    buf_pool_t *buf_pool;

    buf_pool = buf_pool_from_array(i);

    buf_stat = &buf_pool->stat;
    tot_stat->n_page_gets += buf_stat->n_page_gets;
    tot_stat->n_pages_read += buf_stat->n_pages_read;
    tot_stat->n_pages_written += buf_stat->n_pages_written;
    tot_stat->n_pages_created += buf_stat->n_pages_created;
    tot_stat->n_ra_pages_read_rnd += buf_stat->n_ra_pages_read_rnd;
    tot_stat->n_ra_pages_read += buf_stat->n_ra_pages_read;
    tot_stat->n_ra_pages_evicted += buf_stat->n_ra_pages_evicted;
    tot_stat->n_pages_made_young += buf_stat->n_pages_made_young;

    tot_stat->n_pages_not_made_young += buf_stat->n_pages_not_made_young;
  }
}
