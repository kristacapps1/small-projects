# small-projects
#################################
     Small Projects README
#################################
Included are:

mgetweb.cpp
  - C++ project to download a file from the internet using multiple threads sending HTTP requests through TCP/IP connection.

postfix_evaluator.s
  - A project in MIPS to evaluate any postfix expression. Includes list data structure implementation and stack to evaluate expression

Credit-Debit-java-threads-project
  - includes two files
    > SafeAccount.java
      - includes implementation for a balance account object with synchronized debit and credit operations
    > tester.java
      - tests the SafeAccount

time_file_objects.cpp
  - simple c++ project to time the efficiency of unnamed pipe,named pipe(fifo),and, using multiproccessing, message queue

multithreaded_copy_file_local.cpp
  - simple c++ project to use multiple threads for efficiency to copy all files from one local directory to another

###################################  
Build And Run
##################################

Included is a Makefile to build mgetweb, time_file_objects, and multithreaded_copy_file_local

Running:  

time_file_objects: 
  - requires a test file to time the delay in copying through file objects 
    > Usage: ./time_file_objects testFile.txt  

multithreaded_copy_file_local:  
  - requires a directory with files to copy, a destination directory, and the number of threads. 
    > Usage: ./multithreaded_copy_file_local testdir1 testdir2 30

mgetweb:
  - TBD 
