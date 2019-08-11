Background redo log threads
===========================


@page PAGE_INNODB_REDO_LOG_THREADS Background redo log threads

 Three background log threads are responsible for writes of new data to disk:

 -# [Log writer](@ref sect_redo_log_writer) - writes from the log buffer or
 write-ahead buffer to OS buffers.

 -# [Log flusher](@ref sect_redo_log_flusher) - writes from OS buffers to disk
 (fsyncs).

 -# [Log write_notifier](@ref sect_redo_log_write_notifier) - notifies user
 threads about completed writes to disk (when write_lsn is advanced).

 -# [Log flush_notifier](@ref sect_redo_log_flush_notifier) - notifies user
 threads about completed fsyncs (when flushed_to_disk_lsn is advanced).

 Two background log threads are responsible for checkpoints (reclaiming space
 in log files):

 -# [Log closer](@ref sect_redo_log_closer) - tracks up to which lsn all
 dirty pages have been added to flush lists (wrt. oldest_modification).

 -# [Log checkpointer](@ref sect_redo_log_checkpointer) - determines
 @ref subsect_redo_log_available_for_checkpoint_lsn and writes checkpoints.


 @section sect_redo_log_writer Thread: log writer

 This thread is responsible for writing data from the log buffer to disk
 (to the log files). However, it's not responsible for doing fsync() calls.
 It copies data to system buffers. It is the log flusher thread, which is
 responsible for doing fsync().

 There are following points that need to be addressed by the log writer thread:

 -# %Find out how much data is ready in the log buffer, which is concurrently
    filled in by multiple user threads.

   In the log recent written buffer, user threads set links for every finished
   write to the log buffer. Each such link is represented as a number of bytes
   written, starting from a _start_lsn_. The link is stored in the slot assigned
   to the _start_lsn_ of the write.

   The log writer thread tracks links in the recent written buffer, traversing
   a connected path created by the links. It stops when it encounters a missing
   outgoing link. In such case the next fragment of the log buffer is still
   being written (or the maximum assigned lsn was reached).

   It also stops as soon as it has traversed by more than 4kB, in which case
   it is enough for a next write (unless we decided again to do fsyncs from
   inside the log writer thread). After traversing links and clearing slots
   occupied by the links (in the recent written buffer), the log writer thread
   updates @ref subsect_redo_log_buf_ready_for_write_lsn.

   @diafile storage/innobase/log/recent_written_buffer.dia "Example of links in
 the recent written buffer"

   @note The log buffer has no holes up to the _log.buf_ready_for_write_lsn_
   (all concurrent writes for smaller lsn have been finished).

   If there were no links to traverse, _log.buf_ready_for_write_lsn_ was not
   advanced and the log writer thread needs to wait. In such case it first
   uses spin delay and afterwards switches to wait on the _writer_event_.


 -# Prepare log blocks for writing - update their headers and footers.

   The log writer thread detects completed log blocks in the log buffer.
   Such log blocks will not receive any more writes. Hence their headers
   and footers could be easily updated (e.g. checksum is calculated).

   @diafile storage/innobase/log/log_writer_complete_blocks.dia "Complete blocks
 are detected and written"

   If any complete blocks were detected, they are written directly from
   the log buffer (after updating headers and footers). Afterwards the
   log writer thread retries the previous step before making next decisions.
   For each write consisting of one or more complete blocks, the
   _MONITOR_LOG_FULL_BLOCK_WRITES_ is incremented by one.

   @note There is a special case - when write-ahead is required, data needs
   to be copied to the write-ahead buffer and the last incomplete block could
   also be copied and written. For details read below and check the next point.

   The special case is also for the last, incomplete log block. Note that
   @ref subsect_redo_log_buf_ready_for_write_lsn could be in the middle of
   such block. In such case, next writes are likely incoming to the log block.

   @diafile storage/innobase/log/log_writer_incomplete_block.dia "Incomplete
 block is copied"

   For performance reasons we often need to write the last incomplete block.
   That's because it turned out, that we should try to reclaim user threads
   as soon as possible, allowing them to handle next transactions and provide
   next data.

   In such case:
     - the last log block is first copied to the dedicated buffer, up to the
       @ref subsect_redo_log_buf_ready_for_write_lsn,
     - the remaining part of the block in the dedicated buffer is filled in
       with _0x00_ bytes,
     - header fields are updated,
     - checksum is calculated and stored in the block's footer,
     - the block is written from the dedicated buffer,
     - the _MONITOR_LOG_PARTIAL_BLOCK_WRITES_ is incremented by one.

       @note The write-ahead buffer is used as the dedicated buffer for writes
       of the last incomplete block. That's because, whenever we needed a next
       write-ahead (even for complete blocks), we possibly can also write the
       last incomplete block during the write-ahead. The monitor counters for
       full/partial block writes are incremented before the logic related to
       writing ahead is applied. Hence the counter of partial block writes is
       not incremented if a full block write was possible (in which case only
       requirement for write-ahead could be the reason of writing the incomplete
       block).

   @remarks The log writer thread never updates
   [first_rec_group](@ref a_redo_log_block_first_rec_group) field.
   It has to be set by user threads before the block is allowed to be written.
   That's because only user threads know where are the boundaries between
   groups of log records. The user thread which has written data ending at
   lsn which needs to be pointed as _first_rec_group_, is the one responsible
   for setting the field. User thread which has written exactly up to the end
   of log block, is considered ending at lsn after the header of the next log
   block. That's because after such write, the log writer is allowed to write
   the next empty log block (_buf_ready_for_write_lsn_ points then to such lsn).
   The _first_rec_group_ field is updated before the link is added to the log
   recent written buffer.


 -# Avoid read-on-write issue.

   The log writer thread is also responsible for writing ahead to avoid
   the read-on-write problem. It tracks up to which point the write ahead
   has been done. When a write would go further:

     - If we were trying to write more than size of single write-ahead
       region, we limit the write to completed write-ahead sized regions,
       and postpone writing the last fragment for later (retrying with the
       first step and updating the _buf_ready_for_write_lsn_).

       @note If we needed to write complete regions of write-ahead bytes,
       they are ready in the log buffer and could be written directly from
       there. Such writes would not cause read-on-write problem, because
       size of the writes is divisible by write-ahead region.

     - Else, we copy data to special write-ahead buffer, from which
       we could safely write the whole single write-ahead sized region.
       After copying the data, the write-ahead buffer is completed with
       _0x00_ bytes.

       @note The write-ahead buffer is also used for copying the last
       incomplete log block, which was described in the previous point.


 -# Update write_lsn.

   After doing single write (single fil_io()), the log writer thread updates
   @ref subsect_redo_log_write_lsn and fallbacks to its main loop. That's
   because a lot more data could be prepared in meantime, as the write operation
   could take significant time.

   That's why the general rule is that after doing fil_io(), we need to update
   @ref subsect_redo_log_buf_ready_for_write_lsn before making next decisions
   on how much to write within next fil_io() call.


 -# Notify [log writer_notifier thread](@ref sect_redo_log_write_notifier)
   using os_event_set on the _write_notifier_event_.

   @see @ref sect_redo_log_waiting_for_writer


 -# Notify [log flusher thread](@ref sect_redo_log_flusher) using os_event_set()
   on the _flusher_event_.


 @section sect_redo_log_flusher Thread: log flusher

 The log flusher thread is responsible for doing fsync() of the log files.

 When the fsync() calls are finished, the log flusher thread updates the
 @ref subsect_redo_log_flushed_to_disk_lsn and notifies the
 [log flush_notifier thread](@ref sect_redo_log_flush_notifier) using
 os_event_set() on the _flush_notifier_event_.

 @remarks
 Small optimization has been applied - if there was only a single log block
 flushed since the previous flush, then the log flusher thread notifies user
 threads directly (instead of notifying the log flush_notifier thread).
 Impact of the optimization turned out to be positive for some scenarios and
 negative for other, so further investigation is required. However, because
 the change seems to make sense from logical point of view, it has been
 preserved.

 If the log flusher thread detects that none of the conditions is satisfied,
 it simply waits and retries the checks. After initial spin delay, it waits
 on the _flusher_event_.


 @section sect_redo_log_flush_notifier Thread: log flush_notifier

 The log flush_notifier thread is responsible for notifying all user threads
 that are waiting for @ref subsect_redo_log_flushed_to_disk_lsn >= lsn, when
 the condition is satisfied.

 @remarks
 It also notifies when it is very likely to be satisfied (lsn values are
 within the same log block). It is allowed to make mistakes and it is
 responsibility of the notified user threads to ensure, that
 the _flushed_to_disk_lsn_ is advanced sufficiently.

 The log flush_notifier thread waits for the advanced _flushed_to_disk_lsn_
 in loop, using os_event_wait_time_low() on the _flush_notifier_event_.
 When it gets notified by the [log flusher](@ref sect_redo_log_flusher),
 it ensures that the _flushed_to_disk_lsn_ has been advanced (single new
 byte is enough though).

 It notifies user threads waiting on all events between (inclusive):
   - event for a block with the previous value of _flushed_to_disk_lsn_,
   - event for a block containing the new value of _flushed_to_disk_lsn_.

 Events are assigned per blocks in the circular array of events using mapping:

         event_slot = (lsn-1) / OS_FILE_LOG_BLOCK_SIZE % S

 where S is size of the array (number of slots with events). Each slot has
 single event, which groups all user threads waiting for flush up to any lsn
 within the same log block (or log block with number greater by S*i).

 @diafile storage/innobase/log/log_notifier_notifications.dia "Notifications
 executed on slots"

 Internal mutex in event is used, to avoid missed notifications (these would
 be worse than the false notifications).

 However, there is also maximum timeout defined for the waiting on the event.
 After the timeout was reached (default: 1ms), the _flushed_to_disk_lsn_ is
 re-checked in the user thread (just in case).

 @note Because flushes are possible for @ref subsect_redo_log_write_lsn set in
 the middle of log block, it is likely that the same slot for the same block
 will be notified multiple times in a row. We tried delaying notifications for
 the last block, but the results were only worse then. It turned out that
 latency is extremely important here.

 @see @ref sect_redo_log_waiting_for_flusher


 @section sect_redo_log_write_notifier Thread: log write_notifier

 The log write_notifier thread is responsible for notifying all user threads
 that are waiting for @ref subsect_redo_log_write_lsn >= lsn, when the condition
 is satisfied.

 @remarks
 It also notifies when it is very likely to be satisfied (lsn values are
 within the same log block). It is allowed to make mistakes and it is
 responsibility of the notified user threads to ensure, that the _write_lsn_
 is advanced sufficiently.

 The log write_notifier thread waits for the advanced _write_lsn_ in loop,
 using os_event_wait_time_low() on the _write_notifier_event_.
 When it gets notified (by the [log writer](@ref sect_redo_log_writer)),
 it ensures that the _write_lsn_ has been advanced (single new byte is enough).
 Then it notifies user threads waiting on all events between (inclusive):
   - event for a block with the previous value of _write_lsn_,
   - event for a block containing the new value of _write_lsn_.

 Events are assigned per blocks in the circular array of events using mapping:

         event_slot = (lsn-1) / OS_FILE_LOG_BLOCK_SIZE % S

 where S is size of the array (number of slots with events). Each slot has
 single event, which groups all user threads waiting for write up to any lsn
 within the same log block (or log block with number greater by S*i).

 Internal mutex in event is used, to avoid missed notifications (these would
 be worse than the false notifications).

 However, there is also maximum timeout defined for the waiting on the event.
 After the timeout was reached (default: 1ms), the _write_lsn_ is re-checked
 in the user thread (just in case).

 @note Because writes are possible for @ref subsect_redo_log_write_lsn set in
 the middle of log block, it is likely that the same slot for the same block
 will be notified multiple times in a row.

 @see @ref sect_redo_log_waiting_for_writer


 @section sect_redo_log_closer Thread: log closer

 The log closer thread is responsible for tracking up to which lsn, all
 dirty pages have already been added to flush lists. It traverses links
 in the log recent closed buffer, following a connected path, which is
 created by the links. The traversed links are removed and afterwards
 the @ref subsect_redo_log_buf_dirty_pages_added_up_to_lsn is updated.

 Links are stored inside slots in a ring buffer. When link is removed,
 the related slot becomes empty. Later it is reused for link pointing
 from larger lsn value.

 The log checkpointer thread must not write a checkpoint for lsn larger
 than _buf_dirty_pages_added_up_to_lsn_. That is because some user thread
 might be in state where it is just after writing to the log buffer, but
 before adding its dirty pages to flush lists. The dirty pages could have
 modifications protected by log records, which start at lsn, which would
 be logically deleted by such checkpoint.


 @section sect_redo_log_checkpointer Thread: log checkpointer

 The log checkpointer thread is responsible for:

 -# Checking if a checkpoint write is required (to decrease checkpoint age
 before it gets too big).

 -# Checking if synchronous flush of dirty pages should be forced on page
 cleaner threads, because of space in redo log or age of the oldest page.

 -# Writing checkpoints (it's the only thread allowed to do it!).

 This thread has been introduced at the very end. It was not required for
 the performance, but it makes the design more consistent after we have
 introduced other log threads. That's because user threads are not doing
 any writes to the log files themselves then. Previously they were writing
 checkpoints when needed, which required synchronization between them.

 The log checkpointer thread updates log.available_for_checkpoint_lsn,
 which is calculated as:

         min(log.buf_dirty_pages_added_up_to_lsn, max(0, oldest_lsn - L))

 where:
   - oldest_lsn = min(oldest modification of the earliest page from each
                      flush list),
   - L is a number of slots in the log recent closed buffer.

 The special case is when there is no dirty page in flush lists - then it's
 basically set to the _log.buf_dirty_pages_added_up_to_lsn_.

 @note Note that previously, all user threads were trying to calculate this
 lsn concurrently, causing contention on flush_list mutex, which is required
 to read the _oldest_modification_ of the earliest added page. Now the lsn
 is updated in single thread.


 @section sect_redo_log_waiting_for_writer Waiting until log has been written to
 disk

 User has to wait until the [log writer thread](@ref sect_redo_log_writer)
 has written data from the log buffer to disk for lsn >= _end_lsn_ of log range
 used by the user, which is true when:

         write_lsn >= end_lsn

 The @ref subsect_redo_log_write_lsn is updated by the log writer thread.

 The waiting is solved using array of events. The user thread waiting for
 a given lsn, waits using the event at position:

         slot = (end_lsn - 1) / OS_FILE_LOG_BLOCK_SIZE % S

 where _S_ is number of entries in the array. Therefore the event corresponds
 to log block which contains the _end_lsn_.

 The [log write_notifier thread](@ref sect_redo_log_write_notifier) tracks how
 the @ref subsect_redo_log_write_lsn is advanced and notifies user threads for
 consecutive slots.

 @remarks
 When the _write_lsn_ is in the middle of log block, all user threads waiting
 for lsn values within the whole block are notified. When user thread is
 notified, it checks if the current value of the _write_lsn_ is sufficient and
 retries waiting if not. To avoid missed notifications, event's internal mutex
 is used.


 @section sect_redo_log_waiting_for_flusher Waiting until log has been flushed
 to disk

 If a user need to assure the log persistence in case of crash (e.g. on COMMIT
 of a transaction), he has to wait until [log flusher](@ref
 sect_redo_log_flusher) has flushed log files to disk for lsn >= _end_lsn_ of
 log range used by the user, which is true when:

         flushed_to_disk_lsn >= end_lsn

 The @ref subsect_redo_log_flushed_to_disk_lsn is updated by the log flusher
 thread.

 The waiting is solved using array of events. The user thread waiting for
 a given lsn, waits using the event at position:

         slot = (end_lsn - 1) / OS_FILE_LOG_BLOCK_SIZE % S

 where _S_ is number of entries in the array. Therefore the event corresponds
 to log block which contains the _end_lsn_.

 The [log flush_notifier thread](@ref sect_redo_log_flush_notifier) tracks how
 the
 @ref subsect_redo_log_flushed_to_disk_lsn is advanced and notifies user
 threads for consecutive slots.

 @remarks
 When the _flushed_to_disk_lsn_ is in the middle of log block, all
 user threads waiting for lsn values within the whole block are notified.
 When user thread is notified, it checks if the current value of the
 _flushed_to_disk_lsn_ is sufficient and retries waiting if not.
 To avoid missed notifications, event's internal mutex is used.


 @page PAGE_INNODB_REDO_LOG_FORMAT Format of redo log

 @section sect_redo_log_format_overview Overview

 Redo log contains multiple log files, each has the same format. Consecutive
 files have data for consecutive ranges of lsn values. When a file ends at
 _end_lsn_, the next log file begins at the _end_lsn_. There is a fixed number
 of log files, they are re-used in circular manner. That is, for the last
 log file, the first log file is a successor.

 @note A single big file would remain fully cached for some of file systems,
 even if only a small fragment of the file is being modified. Hence multiple
 log files are used to make evictions always possible. Keep in mind though
 that log files are used in circular manner (lsn modulo size of log files,
 when size is calculated except the log file headers).

 The default log file names are: _ib_logfile0_, _ib_logfile1_, ... The maximum
 allowed number of log files is 100. The special file name _ib_logfile101_ is
 used when new log files are created and it is used instead of _ib_logfile0_
 until all the files are ready. Afterwards the _ib_logfile101_ is atomically
 renamed to _ib_logfile0_ and files are considered successfully created then.

 @section sect_redo_log_format_file Log file format

 @subsection subsect_redo_log_format_header Header of log file

 %Log file starts with a header of _LOG_FILE_HDR_SIZE_ bytes. It contains:

   - Initial block of _OS_FILE_LOG_BLOCK_SIZE_ (512) bytes, which has:

     - Binding of an offset within the file to the lsn value.

       This binding allows to map any lsn value which is represented
       within the file to corresponding lsn value.

   - Format of redo log - remains the same as before the patch.

   - Checksum of the block.

   - Two checkpoint blocks - _LOG_CHECKPOINT_1_ and _LOG_CHECKPOINT_2_.

     Each checkpoint block contains _OS_FILE_LOG_BLOCK_SIZE_ bytes:

       - _checkpoint_lsn_ - lsn to start recovery at.

         @note In earlier versions than 8.0, checkpoint_lsn pointed
         directly to the beginning of the first log record group,
         which should be recovered (but still the related page could
         have been flushed). However since 8.0 this value might point
         to some byte inside a log record. In such case, recovery is
         supposed to skip the group of log records which contains
         the checkpoint lsn (and start at the beginning of the next).
         We cannot easily determine beginning of the next group.
         There are two cases:

         - block with _checkpoint_lsn_ has no beginning of group at all
           (first_rec_group = 0) - then we search forward for the first
           block that has non-zero first_rec_group and there we have
           the next group's start,

         - block with _checkpoint_lsn_ has one or more groups of records
           starting inside the block - then we start parsing at the first
           group that starts in the block and keep parsing consecutive
           groups until we passed checkpoint_lsn; we don't apply these
           groups of records (we must not because of fil renames); after
           we passed checkpoint_lsn, the next group that starts is the
           one we were looking for to start recovery at; it is possible
           that the next group begins in the next block (if there was no
           more groups starting after checkpoint_lsn within the block)

       - _checkpoint_no_ - checkpoint number - when checkpoint is
         being written, a next checkpoint number is assigned.

       - _log.buf_size_ - size of the log buffer when the checkpoint
         write was started.

         It remains a mystery, why do we need that. It's neither used
         by the recovery, nor required for MEB. Some rumours say that
         maybe it could be useful for auto-config external tools to
         detect what configuration of MySQL should be used.

         @note
         Note that size of the log buffer could be decreased in runtime,
         after writing the checkpoint (which was not the case, when this
         field was being introduced).

     There are two checkpoint headers, because they are updated alternately.
     In case of crash in the middle of any such update, the alternate header
     would remain valid (so it's the same reason for which double write buffer
     is used for pages).

     @remarks
     Each log file has its own header. However checkpoints are read only from
     the first log file (_ib_logfile0_) during recovery.

 @subsection subsect_redo_log_format_blocks Log blocks

 After the header, there are consecutive log blocks. Each log block has the same
 format and consists of _OS_FILE_LOG_BLOCK_SIZE_ bytes (512). These bytes are
 enumerated by lsn values.

 @note Bytes used by [headers of log files](@ref subsect_redo_log_format_header)
 are NOT included in lsn sequence.

 Each log block contains:
   - header - _LOG_BLOCK_HDR_SIZE_ bytes (12):

     - @anchor a_redo_log_block_hdr_no hdr_no

       This is a block number. Consecutive blocks have consecutive numbers.
       Hence this is basically lsn divided by _OS_FILE_LOG_BLOCK_SIZE_.
       However it is also wrapped at 1G (due to limited size of the field).
       It should be possible to wrap it at 2G (only the single flush bit is
       reserved as the highest bit) but for historical reasons it is 1G.

     - @anchor a_redo_log_block_flush_bit flush_bit

       This is a single bit stored as the highest bit of hdr_no. The bit is
       skipped when calculating block number.

       It is set for the first block of multiple blocks written in a single
       call to fil_io().

       It was supposed to help to filter out writes which were not atomic.
       When the flush bit is read from disk, it means that up to this lsn,
       all previous log records have been fully written from the log buffer
       to OS buffers. That's because previous calls to fil_io() had to be
       finished, before a fil_io() call for current block was started.

       The wrong assumption was that we can trust those log records then.
       Note, we have no guarantee that order of writes is preserved by disk
       controller. That's why only after fsync() call is finished, one could
       be sure, that data is fully written (up to the write_lsn at which
       fsync() was started).

       During recovery, when the flush bit is encountered, *contiguous_lsn
       is updated, but then the updated lsn seems unused...

       It seems that there is no real benefit from the flush bit at all,
       and even in 5.7 it was completely ignored during the recovery.

     - @anchor a_redo_log_block_data_len data_len

       Number of bytes within the log block. Possible values:

         - _0_ - this is an empty block (end the recovery).

         - _OS_FILE_LOG_BLOCK_SIZE_ - this is a full block.

         - value within [_LOG_BLOCK_HDR_SIZE_,
           _OS_FILE_LOG_BLOCK_SIZE_ - _LOG_BLOCK_TRL_SIZE_),
           which means that this is the last block and it is an
           incomplete block.

           This could be then considered an offset, which points
           to the end of the data within the block. This value
           includes _LOG_BLOCK_HDR_SIZE_ bytes of the header.

     - @anchor a_redo_log_block_first_rec_group first_rec_group

       Offset within the log block to the beginning of the first group
       of log records that starts within the block or 0 if none starts.
       This offset includes _LOG_BLOCK_HDR_SIZE_ bytes of the header.

     - @anchor a_redo_log_block_checkpoint_no checkpoint_no

       Checkpoint number of a next checkpoint write. Set by the log
       writer thread just before a write starts for the block.

       It could be used during recovery to detect that we have read
       old block of redo log (tail) because of the wrapped log files.

   - data part - bytes up to [data_len](@ref a_redo_log_block_data_len) byte.

     Actual data bytes are followed by _0x00_ if the block is incomplete.

     @note Bytes within this fragment of the block, are enumerated by _sn_
     sequence (whereas bytes of header and trailer are NOT). This is the
     only difference between _sn_ and _lsn_ sequences (_lsn_ enumerates
     also bytes of header and trailer).

   - trailer - _LOG_BLOCK_TRL_SIZE_ bytes (4):

     - checksum

       Algorithm used for the checksum depends on the configuration.
       Note that there is a potential problem if a crash happened just
       after switching to "checksums enabled". During recovery some log
       blocks would have checksum = LOG_NO_CHECKSUM_MAGIC and some would
       have a valid checksum. Then recovery with enabled checksums would
       point problems for the blocks without valid checksum. User would
       have to disable checksums for the recovery then.



 @remarks
 All fields except [first_rec_group](@ref a_redo_log_block_first_rec_group)
 are updated by the [log writer thread](@ref sect_redo_log_writer) just before
 writing the block.

