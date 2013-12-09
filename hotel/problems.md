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

Problems
--------------------------------------------------------------------------------

As far as we are aware no problems exist in relation to:
 - Busy waiting
 - Deadlocks
 - Failure to clean up

We have tested our solution extensively across different environments. In
order to increase portability we included errno.h instead of statically
defining `ETIMEDOUT`, we noticed that this value differed on OSX, BSD and Linux.

We have ran our solution through valgrind, it reported that all memory was being
freed successfully. For the below report we replaced sigwait with a sleep of 20
seconds and reduced the sleep and expiry time max random value.

$ valgrind --tool=memcheck --track-origins=yes --leak-check=full ./a.out 

==16220== Memcheck, a memory error detector
==16220== Copyright (C) 2002-2012, and GNU GPL'd, by Julian Seward et al.
==16220== Using Valgrind-3.8.1 and LibVEX; rerun with -h for copyright info
==16220== Command: ./a.out
==16220== 
==16220== Syscall param rt_sigprocmask(set) points to uninitialised byte(s)
==16220==    at 0x507EB60: sigprocmask (in /lib64/libc-2.12.so)
==16220==    by 0x401201: main (in /home/duffy/a.out)
==16220==  Address 0x7ff000360 is on thread 1's stack
==16220==  Uninitialised value was created by a stack allocation
==16220==    at 0x40115B: main (in /home/duffy/a.out)
==16220== 
Registering:	4384 Mon Dec  9 13:22:26 2013

Registering:	1916 Mon Dec  9 13:22:24 2013

Registering:	387 Mon Dec  9 13:22:24 2013

Registering:	1422 Mon Dec  9 13:22:25 2013

Wake up:	1916 Mon Dec  9 13:22:24 2013

Expired alarms:	1
Pending alarms:	3

Wake up:	387 Mon Dec  9 13:22:24 2013

Expired alarms:	2
Pending alarms:	2

Registering:	3691 Mon Dec  9 13:22:33 2013

Wake up:	1422 Mon Dec  9 13:22:25 2013

Expired alarms:	3
Pending alarms:	2

Registering:	3927 Mon Dec  9 13:22:25 2013

Registering:	4173 Mon Dec  9 13:22:31 2013

Wake up:	3927 Mon Dec  9 13:22:25 2013

Expired alarms:	4
Pending alarms:	3

Wake up:	4384 Mon Dec  9 13:22:26 2013

Expired alarms:	5
Pending alarms:	2

Registering:	369 Mon Dec  9 13:22:33 2013

Registering:	783 Mon Dec  9 13:22:27 2013

Registering:	124 Mon Dec  9 13:22:34 2013

Wake up:	783 Mon Dec  9 13:22:27 2013

Expired alarms:	6
Pending alarms:	4

Registering:	3930 Mon Dec  9 13:22:30 2013

Registering:	3059 Mon Dec  9 13:22:37 2013

Registering:	1394 Mon Dec  9 13:22:35 2013

Wake up:	3930 Mon Dec  9 13:22:30 2013

Expired alarms:	7
Pending alarms:	6

Registering:	3043 Mon Dec  9 13:22:39 2013

Wake up:	4173 Mon Dec  9 13:22:31 2013

Expired alarms:	8
Pending alarms:	6

Registering:	4422 Mon Dec  9 13:22:40 2013

Registering:	3538 Mon Dec  9 13:22:39 2013

Registering:	3316 Mon Dec  9 13:22:31 2013

Wake up:	3316 Mon Dec  9 13:22:31 2013

Expired alarms:	9
Pending alarms:	8

Registering:	3527 Mon Dec  9 13:22:33 2013

Registering:	4957 Mon Dec  9 13:22:35 2013

Registering:	4171 Mon Dec  9 13:22:38 2013

Wake up:	369 Mon Dec  9 13:22:33 2013

Expired alarms:	10
Pending alarms:	10

Wake up:	3527 Mon Dec  9 13:22:33 2013

Expired alarms:	11
Pending alarms:	9

Wake up:	3691 Mon Dec  9 13:22:33 2013

Expired alarms:	12
Pending alarms:	8

Registering:	2306 Mon Dec  9 13:22:38 2013

Registering:	1328 Mon Dec  9 13:22:39 2013

Wake up:	124 Mon Dec  9 13:22:34 2013

Expired alarms:	13
Pending alarms:	9

Registering:	847 Mon Dec  9 13:22:43 2013

Wake up:	4957 Mon Dec  9 13:22:35 2013

Expired alarms:	14
Pending alarms:	9

Wake up:	1394 Mon Dec  9 13:22:35 2013

Expired alarms:	15
Pending alarms:	8

Registering:	858 Mon Dec  9 13:22:39 2013

Registering:	4583 Mon Dec  9 13:22:41 2013

Registering:	3368 Mon Dec  9 13:22:40 2013

Registering:	4044 Mon Dec  9 13:22:36 2013

Wake up:	4044 Mon Dec  9 13:22:36 2013

Expired alarms:	16
Pending alarms:	11

Wake up:	3059 Mon Dec  9 13:22:37 2013

Expired alarms:	17
Pending alarms:	10

Registering:	1809 Mon Dec  9 13:22:43 2013

Registering:	789 Mon Dec  9 13:22:41 2013

Wake up:	2306 Mon Dec  9 13:22:38 2013

Expired alarms:	18
Pending alarms:	11

Wake up:	4171 Mon Dec  9 13:22:38 2013

Expired alarms:	19
Pending alarms:	10

Registering:	2652 Mon Dec  9 13:22:42 2013

Wake up:	858 Mon Dec  9 13:22:39 2013

Expired alarms:	20
Pending alarms:	10

Wake up:	1328 Mon Dec  9 13:22:39 2013

Expired alarms:	21
Pending alarms:	9

Wake up:	3538 Mon Dec  9 13:22:39 2013

Expired alarms:	22
Pending alarms:	8

Wake up:	3043 Mon Dec  9 13:22:39 2013

Expired alarms:	23
Pending alarms:	7

Registering:	4933 Mon Dec  9 13:22:39 2013

Registering:	3369 Mon Dec  9 13:22:48 2013

Registering:	1227 Mon Dec  9 13:22:45 2013

Registering:	2540 Mon Dec  9 13:22:44 2013

Registering:	1435 Mon Dec  9 13:22:47 2013

Wake up:	4933 Mon Dec  9 13:22:39 2013

Expired alarms:	24
Pending alarms:	11

Wake up:	3368 Mon Dec  9 13:22:40 2013

Expired alarms:	25
Pending alarms:	10

Wake up:	4422 Mon Dec  9 13:22:40 2013

Expired alarms:	26
Pending alarms:	9

The guest thread is cleaning up...
The guest thread says goodbye.
The waiter thread is cleaning up...
The waiter thread says goodbye
Pending: 0
==16220== 
==16220== HEAP SUMMARY:
==16220==     in use at exit: 0 bytes in 0 blocks
==16220==   total heap usage: 110 allocs, 110 frees, 6,848 bytes allocated
==16220== 
==16220== All heap blocks were freed -- no leaks are possible
==16220== 
==16220== For counts of detected and suppressed errors, rerun with: -v
==16220== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 6 from 6)
