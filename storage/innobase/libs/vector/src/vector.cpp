#include <innodb/vector/vector.h>

/********************************************************************
Create a new vector with the given initial size. */
ib_vector_t *ib_vector_create(
    /* out: vector */
    ib_alloc_t *allocator, /* in: vector allocator */
    ulint sizeof_value,    /* in: size of data item */
    ulint size)            /* in: initial size */
{
  ib_vector_t *vec;

  ut_a(size > 0);

  vec = static_cast<ib_vector_t *>(
      allocator->mem_malloc(allocator, sizeof(*vec)));

  vec->used = 0;
  vec->total = size;
  vec->allocator = allocator;
  vec->sizeof_value = sizeof_value;

  vec->data = static_cast<void *>(
      allocator->mem_malloc(allocator, vec->sizeof_value * size));

  return (vec);
}

/********************************************************************
Resize the vector, currently the vector can only grow and we
expand the number of elements it can hold by 2 times. */
void ib_vector_resize(ib_vector_t *vec) /* in: vector */
{
  ulint new_total = vec->total * 2;
  ulint old_size = vec->used * vec->sizeof_value;
  ulint new_size = new_total * vec->sizeof_value;

  vec->data = static_cast<void *>(vec->allocator->mem_resize(
      vec->allocator, vec->data, old_size, new_size));

  vec->total = new_total;
}
