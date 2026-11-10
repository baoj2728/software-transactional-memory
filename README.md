# Software Transactional Memory
Dylan Bao (jiayuanb) and Samuel Zhang (samuelz2)

C++ Software Transactional Memory Runtime

## Summary
   
We plan to implement a software transactional memory (STM) library in C++ that allows multiple threads to perform atomic transactions on shared memory without explicitly managing locks. We will run our implementation on multicore CPUs and compare its performance against traditional locking methods. We also hope to explore ways to improve performance under high contention

## Background

In parallel programs, multiple threads often need to access and modify the same memory locations. To prevent race conditions, programmers typically use locks to synchronize these accesses. However, locking has several drawbacks. Coarse-grained locks can limit parallelism by preventing threads from executing at the same time, even when they are accessing unrelated data. Fine-grained locks allow more concurrency but are harder to implement correctly and can introduce deadlocks.

Transactional memory provides another way to handle synchronization. Instead of explicitly acquiring and releasing locks, programmers group memory operations into transactions that should execute atomically. The system is responsible for detecting conflicts between transactions and making sure that their changes are only visible when the entire transaction successfully commits. If a conflict occurs, a transaction can be aborted and retried.

Transactional memory can be implemented through hardware or software. Hardware transactional memory uses processor features such as cache coherence to track memory accesses and detect conflicts. Software transactional memory instead uses software data structures and atomic operations to provide similar functionality.

For our project, we plan to implement an STM library in C++ based on the TL2 algorithm. Each transaction will maintain a read set and a write set to track which memory locations it accesses. Writes will be buffered locally until the transaction commits, while versioned locks will be used to detect conflicts and validate reads. This should allow independent transactions to execute concurrently without requiring a global lock.

## The Challenge

One of the main challenges will be ensuring that transactions execute correctly when multiple threads are accessing the same memory. Transactions need to appear atomic, meaning that other threads should never observe partially completed updates. We will need to carefully coordinate how transactions read shared data, validate their accesses, and commit their changes.

Another challenge is detecting conflicts efficiently. Our implementation will need to track the memory locations that each transaction reads and writes. When two transactions access the same location and at least one modifies it, the system needs to determine whether a transaction should be aborted. Detecting these conflicts too frequently may introduce unnecessary overhead, but detecting them too late could waste computation on transactions that eventually need to restart.

We also expect performance to depend heavily on the workload. For example, transactions that access mostly independent memory locations should be able to run in parallel. However, if many threads repeatedly modify the same locations, the number of transaction aborts may increase significantly. Larger transactions may also be more likely to encounter conflicts since they access more memory and take longer to execute.

Finally, STM introduces its own synchronization costs. Tracking read and write sets, maintaining version information, and validating transactions all require additional computation and memory accesses. We want to investigate when these costs are worth the additional concurrency compared to traditional locks.

## Resources

We plan to implement the STM library from scratch using C++17/20 and the standard threading and atomic libraries. We will use the multicore CPU machines available in the GHC labs for development and benchmarking.

Our implementation will be based on the ideas presented in Lecture 18 on Transactional Memory, along with the paper Transactional Locking II by Dice, Shalev, and Shavit. We will use these as references for designing the transaction management and conflict detection mechanisms.

We do not expect to need any specialized hardware since our implementation will be entirely software-based.

## Goals and Deliverables

### What We Plan to Achieve

Our main goal is to build a working STM library that supports atomic transactions across multiple shared memory locations.

The library should support beginning a transaction, reading and writing transactional variables, committing changes, and aborting or retrying transactions when conflicts occur. We plan to implement lazy versioning, optimistic conflict detection, and versioned locks based on TL2.

We will also implement a set of multithreaded applications to test the correctness and performance of our library. Our initial benchmarks will include shared counters and bank account transfers, where multiple threads repeatedly access and modify shared data.

To evaluate performance, we will compare our STM implementation against coarse-grained and fine-grained locking implementations of the same workloads. We plan to measure execution time, transaction throughput, abort rates, and speedup as we increase the number of threads.

We will also vary the size of transactions, the proportion of reads and writes, and the amount of contention to better understand where STM performs well and where it struggles.

### What We Hope to Achieve

If we have enough time, we hope to extend our STM framework to support additional versioning and conflict detection strategies. In particular, we are interested in exploring the four software transactional memory approaches discussed in lecture.

1) Lazy versioning with optimistic reads and writes, based on Sun TL2
2) Lazy versioning with optimistic reads and pessimistic writes, based on MS OSTM
3) Eager versioning with optimistic reads and pessimistic writes, based on Intel STM
4) Eager versioning with pessimistic reads and writes, based on Intel STM

Our initial implementation will focus on the first approach. If time permits, we would extend our framework to support the remaining configurations and compare their performance under different workloads. We are particularly interested in how eager and lazy versioning affect commit and abort overhead, and whether pessimistic conflict detection performs better than optimistic detection under high contention.

We also hope to explore adaptive contention management, where the runtime adjusts how transactions retry based on the frequency of conflicts. This could help reduce wasted work when many threads repeatedly access the same shared memory locations.

Since implementing all four approaches may be too ambitious for the project timeline, we will prioritize completing and benchmarking our initial STM implementation before working on these extensions.

### Final Deliverables

By the end of the project, we plan to have a working C++ STM library, correctness tests, several benchmark applications, and performance comparisons against traditional locking. We also hope to include at least one optimization to our initial implementation if time permits.

## Platform Choice

We chose C++ on multicore CPUs because STM requires direct control over shared memory and synchronization. C++ provides atomic operations and threading primitives that we can use to implement our own transaction management system without relying on hardware transactional memory.

Multicore CPUs are also a natural platform for this project since multiple threads can access the same shared address space. This will allow us to test how well our implementation scales with the number of threads and how contention affects performance.

## Schedule

| Week | Dates | Planned Work |
|---|---|---|
| **Week 1** | Nov 9–15 | Finalize the STM library design and transaction API. Implement basic transaction data structures, including read and write sets, and set up initial correctness tests. |
| **Week 2** | Nov 16–22 | Implement transactional reads and writes, versioned locks, conflict detection, and commit and abort operations. Aim to have a working STM implementation that supports simple multithreaded workloads. |
| **Milestone** | **Nov 23** | Demonstrate the initial STM implementation and present preliminary correctness and performance results. |
| **Week 3** | Nov 23–29 | Test the implementation and fix concurrency issues. Implement coarse-grained and fine-grained locking baselines and begin benchmarking shared counters and bank transfers. |
| **Week 4** | Nov 30–Dec 6 | Investigate performance bottlenecks and implement adaptive contention management. Compare retry strategies under varying contention levels and, if time permits, explore conflict detection granularity. |
| **Week 5** | Dec 7–11 | Run final benchmarks and analyze performance across different thread counts, transaction sizes, and contention levels. Complete documentation, the final report, and the poster. |
