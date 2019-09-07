#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>
#include <innodb/univ/rec_t.h>

struct mtr_t;
struct dict_index_t;
struct dtuple_t;
namespace lob{ struct undo_vers_t; }

/** Build a previous version of a clustered index record. The caller must hold
a latch on the index page of the clustered index record.
If the vrow passed to this function is not null, then this function will store
information about virtual columns from the requested version in vrow, unless the
change did not affect any secondary index nor ordering field of clustered index
(the change has UPD_NODE_NO_ORD_CHANGE flag) in which case the requested
information can not be reconstructed from undo log, and the caller may assume
that the (virtual) columns of secondary index have the same values they have in
the more recent version (the one `rec` comes from).
Equivalently, if the vrow is not returned, it is either because it was not
requested, or not available due to UPD_NODE_NO_ORD_CHANGE.
Obviously vrow is also not set in case rec is the oldest version in history,
in which case we also set old_vers to NULL.
@param[in]	index_rec	clustered index record in the index tree
@param[in]	index_mtr	mtr which contains the latch to index_rec page
                                and purge_view
@param[in]	rec		version of a clustered index record
@param[in]	index		clustered index
@param[in,out]	offsets		rec_get_offsets(rec, index)
@param[in]	heap		memory heap from which the memory needed is
                                allocated
@param[out]	old_vers	previous version, or NULL if rec is the first
                                inserted version, or if history data has been
                                deleted
@param[in]	v_heap		memory heap used to create vrow dtuple if it is
                                not yet created. This heap diffs from "heap"
                                above in that it could be
                                prebuilt->old_vers_heap for selection
@param[out]	vrow		virtual column info, if any
@param[in]	v_status	status determine if it is going into this
                                function by purge thread or not. And if we read
                                "after image" of undo log has been rebuilt
@param[in]	lob_undo	LOB undo information.
@retval true if previous version was built, or if it was an insert or the table
has been rebuilt
@retval false if the previous version is earlier than purge_view, or being
purged, which means that it may have been removed */
bool trx_undo_prev_version_build(const rec_t *index_rec, mtr_t *index_mtr,
                                 const rec_t *rec, const dict_index_t *index,
                                 ulint *offsets, mem_heap_t *heap,
                                 rec_t **old_vers, mem_heap_t *v_heap,
                                 const dtuple_t **vrow, ulint v_status,
                                 lob::undo_vers_t *lob_undo);
