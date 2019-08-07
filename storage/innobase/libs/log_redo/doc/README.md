Redo log system
===============

Redo log system - provides durability for unflushed modifications
to contents of data pages.

This file covers general maintenance, including:
-# Allocation and deallocation of the redo log data structures.
-# Initialization and shutdown of the redo log.
-# Start / stop for the log background threads.
-# Runtime updates of server variables.
-# Extending size of the redo log buffers.
-# Locking redo position (for replication).

Code responsible for writing to redo log could be found in log0buf.cc,
log0write.cc, and log0log.ic. The log writer, flusher, write notifier,
flush notifier, and closer threads are implemented in log0write.cc.

Code responsible for checkpoints could be found in log0chkp.cc.
The log checkpointer thread is implemented there.




@page PAGE_INNODB_REDO_LOG Innodb redo log

@section sect_redo_log_general General idea of redo log

The redo log is a write ahead log of changes applied to contents of data pages.
It provides durability for all changes applied to the pages. In case of crash,
it is used to recover modifications to pages that were modified but have not
been flushed to disk.

@note In case of clean shutdown, the redo log should be logically empty.
This means that after the checkpoint lsn there should be no records to apply.
However the log files still could contain some old data (which is not used
during the recovery process).

Every change to content of a data page must be done through a mini transaction
(so called mtr - mtr_t), which in mtr_commit() writes all its log records
to the redo log.

@remarks Normally these changes are performed using the mlog_write_ulint()
or similar function. In some page-level operations, only a code number of
a c-function and its parameters are written to the redo log, to reduce the
size of the redo log. You should not add parameters to such functions
(e.g. trx_undo_header_create(), trx_undo_insert_header_reuse()).
You should not add functionality which can either change when compared to older
versions, or which is dependent on data outside of the page being modified.
Therefore all functions must implement self-contained page transformation
and it should be unchanged if you don't have very essential reasons to change
the log semantics or format.

Single mtr can cover changes to multiple pages. In case of crash, either the
whole set of changes from a given mtr is recovered or none of the changes.

During life time of a mtr, a log of changes is collected inside an internal
buffer of the mtr. It contains multiple log records, which describe changes
applied to possibly different modified pages. When the mtr is committed, all
the log records are written to the log buffer within a single group of the log
records. Procedure:

-# Total number of data bytes of log records is calculated.
-# Space for the log records is reserved. Range of lsn values is assigned for
   a group of log records.
-# %Log records are written to the reserved space in the log buffer.
-# Modified pages are marked as dirty and moved to flush lists.
   All the dirty pages are marked with the same range of lsn values.
-# Reserved space is closed.

Background threads are responsible for writing of new changes in the log buffer
to the log files. User threads that require durability for the logged records,
have to wait until the log gets flushed up to the required point.

During recovery only complete groups of log records are recovered and applied.
Example given, if we had rotation in a tree, which resulted in changes to three
nodes (pages), we have a guarantee, that either the whole rotation is recovered
or nothing, so we will not end up with a tree that has incorrect structure.

Consecutive bytes written to the redo log are enumerated by the lsn values.
Every single byte written to the log buffer corresponds to current lsn
increased by one.

Data in the redo log is structured in consecutive blocks of 512 bytes
(_OS_FILE_LOG_BLOCK_SIZE_). Each block contains a header of 12 bytes
(_LOG_BLOCK_HDR_SIZE_) and a footer of 4 bytes (_LOG_BLOCK_TRL_SIZE_).
These extra bytes are also enumerated by lsn values. Whenever we refer to
data bytes, we mean actual bytes of log records - not bytes of headers and
footers of log blocks. The sequence of enumerated data bytes, is called the
sn values. All headers and footers of log blocks are added within the log
buffer, where data is actually stored in proper redo format.

When a user transaction commits, extra mtr is committed (related to undo log),
and then user thread waits until the redo log is flushed up to the point,
where log records of that mtr end.

When a dirty page is being flushed, a thread doing the flush, first needs to
wait until the redo log gets flushed up to the newest modification of the page.
Afterwards the page might be flushed. In case of crash, we might end up with
the newest version of the page and without any earlier versions of the page.
Then other pages, which potentially have not been flushed before the crash,
need to be recovered to that version. This applies to:
        * pages modified within the same group of log records,
        * and pages modified within any earlier group of log records.

@section sect_redo_log_architecture Architecture of redo log

@subsection subsect_redo_log_data_layers Data layers

Redo log consists of following data layers:

-# %Log files (typically 4 - 32 GB) - physical redo files that reside on
   the disk.

-# %Log buffer (64 MB by default) - groups data to write to log files,
   formats data in proper way: include headers/footers of log blocks,
   calculates checksums, maintains boundaries of record groups.

-# %Log recent written buffer (e.g. 4MB) - tracks recent writes to the
   log buffer. Allows to have concurrent writes to the log buffer and tracks
   up to which lsn all such writes have been already finished.

-# %Log recent closed buffer (e.g. 4MB) - tracks for which recent writes,
   corresponding dirty pages have been already added to the flush lists.
   Allows to relax order in which dirty pages have to be added to the flush
   lists and tracks up to which lsn, all dirty pages have been added.
   This is required to not make checkpoint at lsn which is larger than
   oldest_modification of some dirty page, which still has not been added
   to the flush list (because user thread was scheduled out).

-# %Log write ahead buffer (e.g. 4kB) - used to write ahead more bytes
   to the redo files, to avoid read-on-write problem. This buffer is also
   used when we need to write an incomplete log block, which might
   concurrently be receiving even more data from next user threads. In such
   case we first copy the incomplete block to the write ahead buffer.

@subsection subsect_redo_log_general_rules General rules

-# User threads write their redo data only to the log buffer.

-# User threads write concurrently to the log buffer, without synchronization
   between each other.

-# The log recent written buffer is maintained to track concurrent writes.

-# Background log threads write and flush the log buffer to disk.

-# User threads do not touch log files. Background log threads are the only
   allowed to touch the log files.

-# User threads wait for the background threads when they need flushed redo.

-# %Events per log block are exposed by redo log for users interested in waiting
   for the flushed redo.

-# Users can see up to which point log has been written / flushed.

-# User threads need to wait if there is no space in the log buffer.

   @diafile storage/innobase/log/arch_writing.dia "Writing to the redo log"

-# User threads add dirty pages to flush lists in the relaxed order.

-# Order in which user threads reserve ranges of lsn values, order in which
   they write to the log buffer, and order in which they add dirty pages to
   flush lists, could all be three completely different orders.

-# User threads do not write checkpoints (are not allowed to touch log files).

-# Checkpoint is automatically written from time to time by a background thread.

-# User threads can request a forced write of checkpoint and wait.

-# User threads need to wait if there is no space in the log files.

   @diafile storage/innobase/log/arch_deleting.dia "Reclaiming space in the redo
log"

-# Well thought out and tested set of _MONITOR_ counters is maintained and
   documented.

-# All settings are configurable through server variables, but the new server
   variables are hidden unless a special _EXPERIMENTAL_ mode has been defined
   when running cmake.

-# All the new buffers could be resized dynamically during runtime. In practice,
   only size of the log buffer is accessible without the _EXPERIMENTAL_ mode.

   @note
   This is a functional change - the log buffer could be resized dynamically
   by users (also decreased).

@section sect_redo_log_lsn_values Glossary of lsn values

Different fragments of head of the redo log are tracked by different values:
  - @ref subsect_redo_log_write_lsn,
  - @ref subsect_redo_log_buf_ready_for_write_lsn,
  - @ref subsect_redo_log_sn.

Different fragments of the redo log's tail are tracked by different values:
  - @ref subsect_redo_log_buf_dirty_pages_added_up_to_lsn,
  - @ref subsect_redo_log_available_for_checkpoint_lsn,
  - @ref subsect_redo_log_last_checkpoint_lsn.

@subsection subsect_redo_log_write_lsn log.write_lsn

Up to this lsn we have written all data to log files. It's the beginning of
the unwritten log buffer. Older bytes in the buffer are not required and might
be overwritten in cyclic manner for lsn values larger by _log.buf_size_.

Value is updated by: [log writer thread](@ref sect_redo_log_writer).

@subsection subsect_redo_log_buf_ready_for_write_lsn log.buf_ready_for_write_lsn

Up to this lsn, all concurrent writes to log buffer have been finished.
We don't need older part of the log recent-written buffer.

It obviously holds:

        log.buf_ready_for_write_lsn >= log.write_lsn

Value is updated by: [log writer thread](@ref sect_redo_log_writer).

@subsection subsect_redo_log_flushed_to_disk_lsn log.flushed_to_disk_lsn

Up to this lsn, we have written and flushed data to log files.

It obviously holds:

        log.flushed_to_disk_lsn <= log.write_lsn

Value is updated by: [log flusher thread](@ref sect_redo_log_flusher).

@subsection subsect_redo_log_sn log.sn

Corresponds to current lsn. Maximum assigned sn value (enumerates only
data bytes).

It obviously holds:

        log.sn >= log_translate_lsn_to_sn(log.buf_ready_for_write_lsn)

Value is updated by: user threads during reservation of space.

@subsection subsect_redo_log_buf_dirty_pages_added_up_to_lsn
log.buf_dirty_pages_added_up_to_lsn

Up to this lsn user threads have added all dirty pages to flush lists.

The redo log records are allowed to be deleted not further than up to this lsn.
That's because there could be a page with _oldest_modification_ smaller than
the minimum _oldest_modification_ available in flush lists. Note that such page
is just about to be added to flush list by a user thread, but there is no mutex
protecting access to the minimum _oldest_modification_, which would be acquired
by the user thread before writing to redo log. Hence for any lsn greater than
_buf_dirty_pages_added_up_to_lsn_ we cannot trust that flush lists are complete
and minimum calculated value (or its approximation) is valid.

@note
Note that we do not delete redo log records physically, but we still can delete
them logically by doing checkpoint at given lsn.

It holds (unless the log writer thread misses an update of the
@ref subsect_redo_log_buf_ready_for_write_lsn):

        log.buf_dirty_pages_added_up_to_lsn <= log.buf_ready_for_write_lsn.

Value is updated by: [log closer thread](@ref sect_redo_log_closer).

@subsection subsect_redo_log_available_for_checkpoint_lsn
log.available_for_checkpoint_lsn

Up to this lsn all dirty pages have been flushed to disk. However, this value
is not guaranteed to be the maximum such value. As insertion order to flush
lists is relaxed, the buf_pool_get_oldest_modification_approx() returns
modification time of some page that was inserted the earliest, it doesn't
have to be the oldest modification though. However, the maximum difference
between the first page in flush list, and one with the oldest modification
lsn is limited by the number of entries in the log recent closed buffer.

That's why from result of buf_pool_get_oldest_modification_approx() size of
the log recent closed buffer is subtracted. The result is used to update the
lsn available for a next checkpoint.

This has impact on the redo format, because the checkpoint_lsn can now point
to the middle of some group of log records (even to the middle of a single
log record). Log files with such checkpoint are not recoverable by older
versions of InnoDB by default.

Value is updated by:
[log checkpointer thread](@ref sect_redo_log_checkpointer).

@see @ref sect_redo_log_add_dirty_pages

@subsection subsect_redo_log_last_checkpoint_lsn log.last_checkpoint_lsn

Up to this lsn all dirty pages have been flushed to disk and the lsn value
has been flushed to header of the first log file (_ib_logfile0_).

The lsn value points to place where recovery is supposed to start. Data bytes
for smaller lsn values are not required and might be overwritten (log files
are circular). One could consider them logically deleted.

Value is updated by:
[log checkpointer thread](@ref sect_redo_log_checkpointer).

It holds:

        log.last_checkpoint_lsn
        <= log.available_for_checkpoint_lsn
        <= log.buf_dirty_pages_added_up_to_lsn.


Read more about redo log details:
- @subpage PAGE_INNODB_REDO_LOG_BUF
- @subpage PAGE_INNODB_REDO_LOG_THREADS
- @subpage PAGE_INNODB_REDO_LOG_FORMAT

