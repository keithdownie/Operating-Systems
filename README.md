Assignment work for CS 5460: Operating Systems. The primary focus of this class was working with Linux. Below is an overview of the assignments included in this repo.

a2 and a5 are assignments relating to the Linux kernel. a2 was simply in introduction to Linux kernels, where we explored how to locate, load, and unload kernels in the Linux system.

a5 was an exploration into creating these kernels. In this assignment, we made two kernels that were loaded into a Linux system. The first of these, sleepy.c, was used for putting processes to sleep and waking them up in various ways. 

The second kernel was a rootkit to hide in the Linux system. This included modifying the various points, such as the system call table, to stay undetected by the system prevent it from being unloaded. This kernel was part of an effort to show us how careful we need to be when loading kernels from other authors.

a3 and a4 were focused on multi-threaded environments and how to stay consistent in them. These were an exploration into various locking models, such as mfence, atomic operations, and mutexes. 