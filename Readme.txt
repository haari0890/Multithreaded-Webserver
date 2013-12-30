MYHTTPD Multithreaded Webserver:

Steps to run the server:
1) To compile the C program type in the MAKE keyword from that directory in the terminal.
2) To execute the server program type in ./myhttpd followed by the arguments. Eg: ./myhttpd -s SJF -d -t 15 -p 8000
3) Get statement and the head statements can be executed from another terminal or client after connecting the user. A filename follows the get/head keyword.
4) For Debugger mode start the server with the "-D" flag as the arguments. This will accept only one request from the client and exits the server program.
5) For help menu type "-H" as arguments from the terminal while executing the program.
6) The first client's IP address is not being recorded. The sub sebsequent requesting client's IP address are recorded.
7) Default Setting- 4 Threads, FCFS policy, 60 second waiting time, Port number: 8080, No Logging.
8) To enable logging to the file use -l argument followed by the path of the logging file name.
