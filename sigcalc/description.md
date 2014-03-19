CA321 - OS Design and Implementation
================================================================================

Assignment 1 - POSIX threads and signal handling
--------------------------------------------------------------------------------

The code is mostly awful but it does work.

Description
--------------------------------------------------------------------------------

The program consists of three main parts:

 - Reader thread.
 - Calculator thread.
 - Main function.

Along with this it has a struct, `sharedData_t` for containing the required
information to be passed to the threads.

### Main Function:

The main function is responsible for:

 - Passing the filename from command line arguments to reader.
 - Creating and Coordinating the threads.

It listens for the following signals:

 - `SIGUSR1` : Responsible for telling reader to read more data or calculator
	       to calculate more data.
 - `SIGUSR2` : Responsible for telling reader or calculator to stop processing.

An integer variable labeled `calculateFlag` is used in order to decide when a
`SIGUSR1` should wake up reader or calculator.

### Reader Thread:

The reader thread is responsible for:

 - Opening and Closing the text file.
 - Reading numbers from the text file and placing them into shared data.
 - Starting the end process when EOF is reached.

It only listens for a `SIGUSR1`.

On recieving a `SIGUSR1` when EOF has not been reached reader will continue
to read Integers from a text file and inform calculator through main that more
data is available for calcuation.

On recieviving a `SIGUSR1` when EOF has been reached the following happens:

 - The file stream is closed.
 - A `SIGUSR2` is sent to main in order to inform calculator that it should end.
 - It waits on calculator to signal back saying it has ended. Then it prints out
   Goodbye.

### Calculator Thread:

The calculator thread is responsible for taking two digits from shared data and
printing their sum.

It listens for:

 - `SIGUSR1` : On recieving this the thread will sum up the available data and
               print it out.
 - `SIGUSR2` : On recieving this the thread will end, sending a signal to main
               in order to inform reader it ended successfully and finally
               printing a goodbye message.
