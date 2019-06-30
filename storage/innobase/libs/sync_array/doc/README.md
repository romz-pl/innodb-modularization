
WAIT ARRAY (The wait array for synchronization primitives)
==========================================================

The wait array consists of cells each of which has an an event object created
for it. The threads waiting for a mutex, for example, can reserve a cell
in the array and suspend themselves to wait for the event to become signaled.
When using the wait array, remember to make sure that some thread holding
the synchronization object will eventually know that there is a waiter in
the array and signal the object, to prevent infinite wait.  Why we chose
to implement a wait array? First, to make mutexes fast, we had to code
our own implementation of them, which only in usually uncommon cases
resorts to using slow operating system primitives. Then we had the choice of
assigning a unique OS event for each mutex, which would be simpler, or
using a global wait array. In some operating systems, the global wait
array solution is more efficient and flexible, because we can do with
a very small number of OS events, say 200. In NT 3.51, allocating events
seems to be a quadratic algorithm, because 10 000 events are created fast,
but 100 000 events takes a couple of minutes to create.

As of 5.0.30 the above mentioned design is changed. Since now OS can handle
millions of wait events efficiently, we no longer have this concept of each
cell of wait array having one event.  Instead, now the event that a thread
wants to wait on is embedded in the wait object (mutex or rw_lock). We still
keep the global wait array for the sake of diagnostics and also to avoid
infinite wait The error_monitor thread scans the global wait array to signal
any waiting threads who have missed the signal.

