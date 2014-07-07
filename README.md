# memory pool allocator and garbage collector

## memory pool
This is largely based on the memory pool allocator of Eli Bendersky(http://eli.thegreenplace.net/2008/10/17/memmgr-a-fixed-pool-memory-allocator/).

## garbage collector
The garbage collector is based on the mark & sweep idea. However, the memory is allocated through pointers of pointers, to allow
for objects to be moved in the memory pool, and eliminate fragmentation.
