/** @file include/ut0new.h
 Instrumented memory allocator.

 Created May 26, 2014 Vasil Dimov
 *******************************************************/

/** Dynamic memory allocation within InnoDB guidelines.
All dynamic (heap) memory allocations (malloc(3), strdup(3), etc, "new",
various std:: containers that allocate memory internally), that are done
within InnoDB are instrumented. This means that InnoDB uses a custom set
of functions for allocating memory, rather than calling e.g. "new" directly.

Here follows a cheat sheet on what InnoDB functions to use whenever a
standard one would have been used.

Creating new objects with "new":
--------------------------------
Standard:
  new expression
  or
  new(std::nothrow) expression
InnoDB, default instrumentation:
  UT_NEW_NOKEY(expression)
InnoDB, custom instrumentation, preferred:
  UT_NEW(expression, key)

Destroying objects, created with "new":
---------------------------------------
Standard:
  delete ptr
InnoDB:
  UT_DELETE(ptr)

Creating new arrays with "new[]":
---------------------------------
Standard:
  new type[num]
  or
  new(std::nothrow) type[num]
InnoDB, default instrumentation:
  UT_NEW_ARRAY_NOKEY(type, num)
InnoDB, custom instrumentation, preferred:
  UT_NEW_ARRAY(type, num, key)

Destroying arrays, created with "new[]":
----------------------------------------
Standard:
  delete[] ptr
InnoDB:
  UT_DELETE_ARRAY(ptr)

Declaring a type with a std:: container, e.g. std::vector:
----------------------------------------------------------
Standard:
  std::vector<t>
InnoDB:
  std::vector<t, ut_allocator<t> >

Declaring objects of some std:: type:
-------------------------------------
Standard:
  std::vector<t> v
InnoDB, default instrumentation:
  std::vector<t, ut_allocator<t> > v
InnoDB, custom instrumentation, preferred:
  std::vector<t, ut_allocator<t> > v(ut_allocator<t>(key))

Raw block allocation (as usual in C++, consider whether using "new" would
not be more appropriate):
-------------------------------------------------------------------------
Standard:
  malloc(num)
InnoDB, default instrumentation:
  ut_malloc_nokey(num)
InnoDB, custom instrumentation, preferred:
  ut_malloc(num, key)

Raw block resize:
-----------------
Standard:
  realloc(ptr, new_size)
InnoDB:
  ut_realloc(ptr, new_size)

Raw block deallocation:
-----------------------
Standard:
  free(ptr)
InnoDB:
  ut_free(ptr)

Note: the expression passed to UT_NEW() or UT_NEW_NOKEY() must always end
with (), thus:
Standard:
  new int
InnoDB:
  UT_NEW_NOKEY(int())
*/
