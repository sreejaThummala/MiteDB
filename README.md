MiteDB
======


Summary
=======

A simple key value store which provides
    consistency
    failure recovery
    fast reads and writes. 
  
Design and implementation
=========================

These three priorities have conflicting optimizations. For instance, the data can be consistent if the server always reads from and writes to disk, but this will cause slow read and write speeds. On the other hand, we can have fast reads and writes by implementing cache on the client side, but doing so will eliminate consistency because updates from other clients will not be detected. Therefore, we need to balance between consistencies, failure recovery, and fast reads and writes. The server needs to eventually write every key-value pair to disk. Since disk accesses are very expensive, 
we implement a B-tree to index the data stored into the disk. B-trees are balanced trees that are optimized for situations when part or all of the tree must be maintained in secondary storage such as a magnetic disk. Since disk accesses are slow, b-tree tries to minimize the number of disk accesses. 

Although b-tree is implemented to index data in the disk, the reads and writes are still not fast enough. 
Therefore, we implemented a Least Recently Used (LRU) cache in the memory of the server to improve reads of 
recent keys. The server will first attempt the cache for all reads because cache reads has magnitudes of performance 
over disk reads. We did not depend on the cache for the writes because cache does not persist to disk. Therefore, the 
server will still perform writes on disk for the sake of consistency and failure recovery if the server were to crash.
Writing to append-only log is less expensive than writing to b-tree which requires random reads and writes . So, our server writes to the append only log first, and then a separate thread writes to b-tree.

When the number of records in logs increase to a MAX_RECORD_COUNT, a new log is created based on 
the timestamp, and the old log file is queued in a shared queue. The elements of the shared queue are removed and 
processed by the indexer thread. The indexer thread removes one element from the shared queue to write to b-tree. 
When all the log records are written to the b-tree, the thread blocks access and deletes the log. If the number of logs 
increased to 5, then writes every log to b-tree.

For a get() operation, the server tries to search the cache for the key. If present, it returns the value. If not, it 
searches the logs. If key is present in the logs, it is returned. Otherwise it searches the underlying b-tree. If still key is not found, it sends an error response.

The consistency is maintained in our design as logs and the records in logs are searched in Last In First Out 
basis. The recovery failure is attained as everything is stored in the secondary storage in the form of .log files (append only log files) and .dat file (data blocks of b-tree). When the database is restarted, the system initializes the b-tree using the .log files and the .dat file. Hence, any data written by the server is present in the b-tree for use after it is bootstrapped.
