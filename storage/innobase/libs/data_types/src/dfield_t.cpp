#include <innodb/data_types/dfield_t.h>

#include <innodb/disk/flags.h>
#include <innodb/memory/mem_heap_alloc.h>

/** Create a deep copy of this object
@param[in]	heap	the memory heap in which the clone will be
                        created.

@return	the cloned object. */
dfield_t *dfield_t::clone(mem_heap_t *heap) {
  const ulint size = len == UNIV_SQL_NULL ? 0 : len;
  dfield_t *obj =
      static_cast<dfield_t *>(mem_heap_alloc(heap, sizeof(dfield_t) + size));

  obj->ext = ext;
  obj->len = len;
  obj->type = type;
  obj->spatial_status = spatial_status;

  if (len != UNIV_SQL_NULL) {
    obj->data = obj + 1;
    memcpy(obj->data, data, len);
  } else {
    obj->data = 0;
  }

  return (obj);
}

byte *dfield_t::blobref() const {
  ut_ad(ext);

  return (static_cast<byte *>(data) + len - BTR_EXTERN_FIELD_REF_SIZE);
}
