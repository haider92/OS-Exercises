CA321 - OS Design and Implementation
================================================================================

Assignment 2 - Hotel Wakeup call system
--------------------------------------------------------------------------------

 - Ian Duffy, 11356066
 - Darren Brogan, 11424362
 - Peter Morgan, 11314976

This project is our own work. We have not recieved assistance beyond what is
normal, and we have cited any sources from which we have borrowed. We have
not given a copy of our work, or a part of our work, to anyone. We are aware
that copying or giving a copy may have serious consequences.

Description
--------------------------------------------------------------------------------

The program consists of three main parts:

 - Main function.
 - LinkedList data structure.
 - Guest thread.
 - Waiter thread.

Along with this it has a struct, `sharedData_t` for containing the required
information to be passed to the threads.

### Main Function:

The main function is responsible for:

 - Creating the shared data and passing it into the threads.
 - Listening for `SIGINT`, canceling the threads and destroying the mutex.

### LinkedList datastructure

The LinkedList datastructure contains nodes with two fields, an expiry time and
a room number. The nodes are sorted based of their expiry time.

### Guest Thread:

The guest thread is responsible for adding items to the LinkedList. 
The items are added in sorted order based of their expiry time.

If the newly added item is the head of the list it will prompt the waiter
thread to update the time that it waits for.

Its cancelation point is after the mutex unlock. Therefore there is no point
in unlocking the mutex within the cleanup handler as it is already done.

### Waiter thread Thread:

The waiter thread is responsible for executing the wakeup calls. While no 
wakeup calls exist it will sleep. If a wakeup call exists it uses
`pthread_cond_timedwait` to sleep until the expiry time is reached.

When the expiry time is reached the wakeup call is removed from the LinkedList
and its details are printed out.

The cancelation points for this thread are at `pthread_cond_wait` and
`pthread_cond_timedwait`, at these points the mutex isn't unlocked. For this
reason the cleanup handler is responsible for unlocking the mutex. Along with
this the cleanup handler is responsible for destroying the allocated memory
to all pending jobs.
