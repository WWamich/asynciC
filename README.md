# asynciC
asynci is a library for asynchronous task execution using threads in C.  It provides an interface for creating, managing, and synchronizing tasks, as well as for asynchronous I/O waiting using select.
Main functions:

  asyncTask — creates and executes an asynchronous task in a separate thread.
  asyncTaskAfter — performs a task after completing another one.
  asyncTaskAfterAll — executes a task after completing all tasks from the array.
  asyncAfterMultiplex — waits for I/O readiness on the file descriptor, allowing asynchronous tasks to continue.
  Features:

  Multitasking support through the use of POSIX threads (pthread).
  Synchronization of tasks using mutexes and conditional variables.
  Waiting for I/O events using select for asynchronous operations.
