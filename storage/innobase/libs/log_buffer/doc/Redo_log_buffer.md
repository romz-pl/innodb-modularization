Redo log buffer
===============



 @page PAGE_INNODB_REDO_LOG_BUF Redo log buffer

 When mtr commits, data has to be moved from internal buffer of the mtr
 to the redo log buffer. For a better concurrency, procedure for writing
 to the log buffer consists of following steps:
 -# @ref sect_redo_log_buf_reserve
 -# @ref sect_redo_log_buf_write
 -# @ref sect_redo_log_buf_add_links_to_recent_written

 Afterwards pages modified during the mtr, need to be added to flush lists.
 Because there is no longer a mutex protecting order in which dirty pages
 are added to flush lists, additional mechanism is required to ensure that
 lsn available for checkpoint is determined properly. Hence the procedure
 consists of following steps:
 -# @ref sect_redo_log_mark_dirty_pages
 -# @ref sect_redo_log_add_dirty_pages
 -# @ref sect_redo_log_add_link_to_recent_closed

 Then the [log closer thread](@ref sect_redo_log_closer) advances lsn up
 to which lsn values might be available for checkpoint safely (up to which
 all dirty pages have been added to flush lists).
 [Read more about reclaiming space...](@ref sect_redo_log_reclaim_space)

 @section sect_redo_log_buf_reserve Reservation of space in the redo

 Range of lsn values is reserved for a provided number of data bytes.
 The reserved range will directly address space for the data in both
 the log buffer and the log files.

 Procedure used to reserve the range of lsn values:

 -# Acquiring shared access to the redo log (through sharded rw_lock)

 -# Increase the global number of reserved data bytes (@ref subsect_redo_log_sn)
    by number of data bytes we need to write.

    This is performed by an atomic fetch_add operation:

         start_sn = log.sn.fetch_add(len)
         end_sn = start_sn + len

    where _len_ is number of data bytes we need to write.

    Then range of sn values is translated to range of lsn values:

         start_lsn = log_translate_sn_to_lsn(start_sn)
         end_lsn = log_translate_sn_to_lsn(end_sn)

    The required translations are performed by simple calculations, because:

         lsn = sn / LOG_BLOCK_DATA_SIZE * OS_FILE_LOG_BLOCK_SIZE
                 + sn % LOG_BLOCK_DATA_SIZE
                 + LOG_BLOCK_HDR_SIZE

 -# Wait until the reserved range corresponds to free space in the log buffer.

   In this step we could be forced to wait for the
   [log writer thread](@ref sect_redo_log_writer),
   which reclaims space in the log buffer by writing data to system buffers.

   The user thread waits until the reserved range of lsn values maps to free
   space in the log buffer, which is true when:

         end_lsn - log.write_lsn <= log.buf_size

   @remarks
   The waiting is performed by a call to log_write_up_to(end_lsn -
 log.buf_size), which has a loop with short sleeps. We assume that it is
 unlikely that the waiting is actually needed. The
 _MONITOR_LOG_ON_BUFFER_SPACE_* counters track number of iterations spent in the
 waiting loop. If they are not nearby 0, DBA should try to increase the size of
 the log buffer.

   @note The log writer thread could be waiting on the write syscall, but it
   also could be waiting for other user threads, which need to complete writing
   their data to the log buffer for smaller sn values! Hopefully these user
   threads have not been scheduled out. If we controlled scheduling (e.g. if
   we have used fiber-based approach), we could avoid such problem.

 -# Wait until the reserved range corresponds to free space in the log files.

   In this step we could be forced to wait for page cleaner threads or the
   [log checkpointer thread](@ref sect_redo_log_checkpointer) until it made
   a next checkpoint.

   The user thread waits until the reserved range of lsn values maps to free
   space in the log files, which is true when:

         end_lsn - log.last_checkpoint_lsn <= redo lsn capacity

   @remarks
   The waiting is performed by a loop with progressive sleeps.
   The _MONITOR_LOG_ON_FILE_SPACE_* counters track number of iterations
   spent in the waiting loop. If they are not nearby 0, DBA should try to
   use more page cleaner threads, increase size of the log files or ask for
   better storage device.

   This mechanism could lead to a __deadlock__, because the user thread waiting
   during commit of mtr, keeps the dirty pages locked, which makes it impossible
   to flush them. Now, if these pages have very old modifications, it could be
   impossible to move checkpoint further without flushing them. In such case the
   log checkpointer thread will be unable to
   [reclaim space in the log files](@ref sect_redo_log_reclaim_space).

   To avoid such problems, user threads call log_free_check() from time to time,
   when they don't keep any latches. They try to do it at least every 4 modified
   pages and if they detected that there is not much free space in the log
 files, they wait until the free space is reclaimed (but without holding
 latches!).

   @note Note that multiple user threads could check the free space without
   holding latches and then proceed with writes. Therefore this mechanism only
   works because the minimum required free space is based on assumptions:
     - maximum number of such concurrent user threads is limited,
     - maximum size of write between two checks within a thread is limited.

   This mechanism does not provide safety when concurrency is not limited!
   In such case we only do the best effort but the deadlock is still possible
   in theory.

 @see log_buffer_reserve()


 @section sect_redo_log_buf_write Copying data to the reserved space

 After a range of lsn values has been reserved, the data is copied to the log
 buffer's fragment related to the range of lsn values.

 The log buffer is a ring buffer, directly addressed by lsn values, which means
 that there is no need for shifting of data in the log buffer. Byte for a given
 lsn is stored at lsn modulo size of the buffer. It is then easier to reach
 higher concurrency with such the log buffer, because shifting would require
 an exclusive access.

 @note However, when writing the wrapped fragment of the log buffer to disk,
 extra IO operation could happen (because we need to copy two disjoint areas
 of memory). First of all, it's a rare case so it shouldn't matter at all.
 Also note that the wrapped fragment results only in additional write to
 system buffer, so still number of real IO operations could stay the same.

 Writes to different ranges of lsn values happen concurrently without any
 synchronization. Each user thread writes its own sequence of log records
 to the log buffer, copying them from the internal buffer of the mtr, leaving
 holes for headers and footers of consecutive log blocks.

 @note There is some hidden synchronization when multiple user threads write to
 the same memory cache line. That happens when they write to the same 64 bytes,
 because they have reserved small consecutive ranges of lsn values. Fortunately
 each mtr takes in average more than few bytes, which limits number of such user
 threads that meet within a cache line.

 When mtr_commit() finishes writing the group of log records, it is responsible
 for updating the _first_rec_group_ field in the header of the block to which
 _end_lsn_ belongs, unless it is the same block to which _start_lsn_ belongs
 (in which case user ending at _start_lsn_ is responsible for the update).

 @see log_buffer_write()


 @section sect_redo_log_buf_add_links_to_recent_written Adding links to the
 recent written buffer

 Fragment of the log buffer, which is close to current lsn, is very likely being
 written concurrently by multiple user threads. There is no restriction on order
 in which such concurrent writes might be finished. Each user thread which has
 finished writing, proceeds further without waiting for any other user threads.

 @diafile storage/innobase/log/user_thread_writes_to_buffer.dia "One of many
 concurrent writes"

 @note Note that when a user thread has finished writing, still some other user
 threads could be writing their data for smaller lsn values. It is still fine,
 because it is the [log writer thread](@ref sect_redo_log_writer) that needs to
 ensure, that it writes only complete fragments of the log buffer. For that we
 need information about the finished writes.

 The log recent written buffer is responsible for tracking which of concurrent
 writes to the log buffer, have been finished. It allows the log writer thread
 to update @ref subsect_redo_log_buf_ready_for_write_lsn, which allows to find
 the next complete fragment of the log buffer to write. It is enough to track
 only recent writes, because we know that up to _log.buf_ready_for_write_lsn_,
 all writes have been finished. Hence this lsn value defines the beginning of
 lsn range represented by the recent written buffer in a given time. The recent
 written buffer is a ring buffer, directly addressed by lsn value. When there
 is no space in the buffer, user thread needs to wait.

 @note Size of the log recent written buffer is limited, so concurrency might
 be limited if the recent written buffer is too small and user threads start
 to wait for each other then (indirectly by waiting for the space reclaimed
 in the recent written buffer by the log writer thread).

 Let us describe the procedure used for adding the links.

 Suppose, user thread has just written some of mtr's log records to a range
 of lsn values _tmp_start_lsn_ .. _tmp_end_lsn_, then:

 -# User thread waits for free space in the recent written buffer, until:

         tmp_end_lsn - log.buf_ready_for_write_lsn <= S

    where _S_ is a number of slots in the log recent_written buffer.

 -# User thread adds the link by setting value of slot for _tmp_start_lsn_:

         to_advance = tmp_end_lsn - tmp_start_lsn
         log.recent_written[tmp_start_lsn % S] = to_advance

    The value gives information about how much to advance lsn when traversing
    the link.

    @note Note that possibly _tmp_end_lsn_ < _end_lsn_. In such case, next write
    of log records in the mtr will start at _tmp_end_lsn_. After all the log
    records are written, the _tmp_end_lsn_ should become equal to the _end_lsn_
    of the reservation (we must not reserve more bytes than we write).

 The log writer thread follows path created by the added links, updates
 @ref subsect_redo_log_buf_ready_for_write_lsn and clears the links, allowing
 to reuse them (for lsn larger by _S_).

 Before the link is added, release barrier is required, to avoid compile time
 or memory reordering of writes to the log buffer and the recent written buffer.
 It is extremely important to ensure, that write to the log buffer will precede
 write to the recent written buffer.

 The same will apply to reads in the log writer thread, so then the log writer
 thread will be sure, that after reading the link from the recent written buffer
 it will read the proper data from the log buffer's fragment related to the
 link.

 Copying data and adding links is performed in loop for consecutive log records
 within the group of log records in the mtr.

 @note Note that until some user thread finished writing all the log records,
 any log records which have been written to the log buffer for larger lsn
 (by other user threads), cannot be written to disk. The log writer thread
 will stop at the missing links in the log recent written buffer and wait.
 It follows connected links only.

 @see log_buffer_write_completed()


 @section sect_redo_log_mark_dirty_pages Marking pages as dirty

 Range of lsn values _start_lsn_ .. _end_lsn_, acquired during the reservation
 of space, represents the whole group of log records. It is used to mark all
 the pages in the mtr as dirty.

 @note During recovery the whole mtr needs to be recovered or skipped at all.
 Hence we don't need more detailed ranges of lsn values when marking pages.

 Each page modified in the mtr is locked and its _oldest_modification_ is
 checked to see if this is the first modification or the page had already been
 modified when its modification in this mtr started.

 Page, which was modified the first time, will have updated:
   - _oldest_modification_ = _start_lsn_,
   - _newest_modification_ = _end_lsn_,

 and will be added to the flush list for corresponding buffer pool (buffer pools
 are sharded by page_id).

 For other pages, only _newest_modification_ field is updated (with _end_lsn_).

 @note Note that some pages could already be modified earlier (in a previous
 mtr) and still unflushed. Such pages would have _oldest_modification_ != 0
 during this phase and they would belong already to flush lists. Hence it is
 enough to update their _newest_modification_.


 @section sect_redo_log_add_dirty_pages Adding dirty pages to flush lists

 After writes of all log records in a mtr_commit() have been finished, dirty
 pages have to be moved to flush lists. Hopefully, after some time the pages
 will become flushed and space in the log files could be reclaimed.

 The procedure for adding pages to flush lists:

 -# Wait for the recent closed buffer covering _end_lsn_.

    Before moving the pages, user thread waits until there is free space for
    a link pointing from _start_lsn_ to _end_lsn_ in the recent closed buffer.
    The free space is available when:

         end_lsn - log.buf_dirty_pages_added_up_to_lsn < L

    where _L_ is a number of slots in the log recent closed buffer.

    This way we have guarantee, that the maximum delay in flush lists is limited
    by _L_. That's because we disallow adding dirty page with too high lsn value
    until pages with smaller lsn values (smaller by more than _L_), have been
    added!

 -# Add the dirty pages to corresponding flush lists.

    During this step pages are locked and marked as dirty as described in
    @ref sect_redo_log_mark_dirty_pages.

    Multiple user threads could perform this step in any order of them.
    Hence order of dirty pages in a flush list, is not the same as order by
    their oldest modification lsn.

    @diafile storage/innobase/log/relaxed_order_of_dirty_pages.dia "Relaxed
 order of dirty pages"

 @note Note that still the @ref subsect_redo_log_buf_dirty_pages_added_up_to_lsn
 cannot be advanced further than to _start_lsn_. That's because the link from
 _start_lsn_ to _end_lsn_, has still not been added at this stage.

 @see log_buffer_write_completed_before_dirty_pages_added()


 @section sect_redo_log_add_link_to_recent_closed Adding link to the log recent
 closed buffer

 After all the dirty pages have been added to flush lists, a link pointing from
 _start_lsn_ to _end_lsn_ is added to the log recent closed buffer.

 This is performed by user thread, by setting value of slot for start_lsn:

         log.recent_closed[start_lsn % L] = end_lsn - start_lsn

 where _L_ is size of the log recent closed buffer. The value gives information
 about how much to advance lsn when traversing the link.

 The [log closer thread](@ref sect_redo_log_closer) is responsible for reseting
 the entry in _log.recent_closed_ to 0, which must happen before the slot might
 be reused for larger lsn values (larger by _L_, _2L_, ...). Afterwards the log
 closer thread advances @ref subsect_redo_log_buf_dirty_pages_added_up_to_lsn,
 allowing user threads, waiting for free space in the log recent closed buffer,
 to proceed.

 @note Note that the increased value of _log.buf_dirty_pages_added_up_to_lsn_
 might possibly allow a newer checkpoint.

 @see log_buffer_write_completed_and_dirty_pages_added()

 After the link is added, the shared-access for log buffer is released.
 This possibly allows any thread waiting for an exclussive access to proceed.


 @section sect_redo_log_reclaim_space Reclaiming space in redo log

 Recall that recovery always starts at the last written checkpoint lsn.
 Therefore @ref subsect_redo_log_last_checkpoint_lsn defines the beginning of
 the log files. Because size of the log files is fixed, it is easy to determine
 if a given range of lsn values corresponds to free space in the log files or
 not (in which case it would overwrite tail of the redo log for smaller lsns).

 Space in the log files is reclaimed by writing a checkpoint for a higher lsn.
 This could be possible when more dirty pages have been flushed. The checkpoint
 cannot be written for higher lsn than the _oldest_modification_ of any of the
 dirty pages (otherwise we would have lost modifications for that page in case
 of crash). It is [log checkpointer thread](@ref sect_redo_log_checkpointer),
 which calculates safe lsn for a next checkpoint
 (@ref subsect_redo_log_available_for_checkpoint_lsn) and writes the checkpoint.
 User threads doing writes to the log buffer, no longer hold mutex, which would
 disallow to determine such lsn and write checkpoint mean while.

 Suppose user thread has just finished writing to the log buffer, and it is just
 before adding the corresponding dirty pages to flush lists, but suddenly became
 scheduled out. Now, the log checkpointer thread comes in and tries to determine
 lsn available for a next checkpoint. If we allowed the thread to take minimum
 _oldest_modification_ of dirty pages in flush lists and write checkpoint at
 that lsn value, we would logically erase all log records for smaller lsn
 values. However the dirty pages, which the user thread was trying to add to
 flush lists, could have smaller value of _oldest_modification_. Then log
 records protecting the modifications would be logically erased and in case of
 crash we would not be able to recover the pages.

 That's why we need to protect from doing checkpoint at such lsn value, which
 would logically erase the just written data to the redo log, until the related
 dirty pages have been added to flush lists.

 The [log closer thread](@ref sect_redo_log_closer) tracks up to which lsn the
 log checkpointer thread might trust that all dirty pages have been added -
 so called @ref subsect_redo_log_buf_dirty_pages_added_up_to_lsn. Any attempts
 to make checkpoint at higher value are limited to this lsn.

 When user thread has added all the dirty pages related to _start_lsn_ ..
 _end_lsn_, it creates link in the log recent closed buffer, pointing from
 _start_lsn_ to _end_lsn_. The log closer thread tracks the links in the recent
 closed buffer, clears the slots (so they could be safely reused) and updates
 the @ref subsect_redo_log_buf_dirty_pages_added_up_to_lsn, reclaiming space
 in the recent closed buffer and potentially allowing to advance checkpoint
 further.

 Order of pages added to flush lists became relaxed so we also cannot rely
 directly on the lsn of the earliest added page to a given flush list.
 It is not guaranteed that it has the minimum _oldest_modification_ anymore.
 However it is guaranteed that it has _oldest_modification_ not higher than
 the minimum by more than _L_. Hence we subtract _L_ from its value and use
 that as lsn available for checkpoint according to the given flush list.
 For more details
 [read about adding dirty pages](@ref sect_redo_log_add_dirty_pages).

 @note Note there are two reasons for which lsn available for checkpoint could
 be updated:
   - because @ref subsect_redo_log_buf_dirty_pages_added_up_to_lsn was updated,
   - because the earliest added dirty page in one of flush lists became flushed.


