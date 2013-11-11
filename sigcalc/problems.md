CA321 - OS Design and Implementation
================================================================================

Assignment 1 - POSIX threads and signal handling
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

Many problems were incountered before coming up with our final solution.

We started off with a program that functioned as expected however it relied on
the reader thread and calculator thread talking directly to each other.

We eventually changed this over so all coordination was done through main. This
was done using signal handlers and making the threads global data. We also used
4 different signals. It took us a long time to get to this stage and we hit many
deadlocks on the way.

Eventually we ended up with our final solution which you wil see in `sigcalc.c`.
We believe this solution is functions as expected and is structured in a good
manor. However, we would agree that the solution could possibly be cleaner.
Specifically in relation to the following points:

 - The file name is passed in the struct to both threads. Should we of created
   a seperate struct so the calculator thread didn't get unrequired information.

 - The result of pthread_self() from main is stored within our struct which is
   also passed to all the threads. We looked at using `kill(0, SIGNAL)` and 
   `raise(signal)` for doing this but experienced unexpected functionality on
   some runs. Using pthread_kill and sending the signals directly to main seemed
   to be the safest option.

 - From the brief it was unclear if reader had to wait for calculator to end. It
   was implemented to do this based on the screencast of an example run.

 - It may of been prudent to use thread cleanup handlers instead of setting
   conditions within the while loops to end and let the thread end naturally.

 - As stated in the brief we make usage of usleep. However, usleep is obsolete
   according to POSIX.1-2001.

 - fscanf may have portability issues. The application was tested on: OpenSuSE,
   CentOS, Ubuntu and OSX. No issues were highlighted.

