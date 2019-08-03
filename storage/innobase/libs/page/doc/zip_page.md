The format of compressed pages is as follows
============================================

The header and trailer of the uncompressed pages, excluding the page
directory in the trailer, are copied as is to the header and trailer
of the compressed page.

At the end of the compressed page, there is a dense page directory
pointing to every user record contained on the page, including deleted
records on the free list.  The dense directory is indexed in the
collation order, i.e., in the order in which the record list is
linked on the uncompressed page.  The infimum and supremum records are
excluded.  The two most significant bits of the entries are allocated
for the delete-mark and an n_owned flag indicating the last record in
a chain of records pointed to from the sparse page directory on the
uncompressed page.

The data between PAGE_ZIP_START and the last page directory entry will
be written in compressed format, starting at offset PAGE_DATA.
Infimum and supremum records are not stored.  We exclude the
REC_N_NEW_EXTRA_BYTES in every record header.  These can be recovered
from the dense page directory stored at the end of the compressed
page.

The fields node_ptr (in non-leaf B-tree nodes; level>0), trx_id and
roll_ptr (in leaf B-tree nodes; level=0), and BLOB pointers of
externally stored columns are stored separately, in ascending order of
heap_no and column index, starting backwards from the dense page
directory.

The compressed data stream may be followed by a modification log
covering the compressed portion of the page, as follows.

MODIFICATION LOG ENTRY FORMAT
- write record:
  - (heap_no - 1) << 1 (1..2 bytes)
  - extra bytes backwards
  - data bytes
- clear record:
  - (heap_no - 1) << 1 | 1 (1..2 bytes)

The integer values are stored in a variable-length format:
- 0xxxxxxx: 0..127
- 1xxxxxxx xxxxxxxx: 0..32767

The end of the modification log is marked by a 0 byte.

In summary, the compressed page looks like this:

(1) Uncompressed page header (PAGE_DATA bytes)
(2) Compressed index information
(3) Compressed page data
(4) Page modification log (page_zip->m_start..page_zip->m_end)
(5) Empty zero-filled space
(6) BLOB pointers (on leaf pages)
  - BTR_EXTERN_FIELD_REF_SIZE for each externally stored column
  - in descending collation order
(7) Uncompressed columns of user records, n_dense * uncompressed_size bytes,
  - indexed by heap_no
  - DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN for leaf pages of clustered indexes
  - REC_NODE_PTR_SIZE for non-leaf pages
  - 0 otherwise
(8) dense page directory, stored backwards
  - n_dense = n_heap - 2
  - existing records in ascending collation order
  - deleted records (free list) in link order
