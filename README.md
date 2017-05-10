# Dynamic-Load-Balancing
Dynamic Load Balancing has been simulated in a distributed environment, using the truthful payment scheme in order to attain minimum overall expected response time.
The algorithm has been implemented as explained in the paper 'Algorithmic Mechanism Design for Load Balancing in Distributed Systems'.
There is a master process which is elected using the leader election algorithm. The other slave processes send their bids to the master. The aim of this algorithm is to show that when true bids are sent, the overall expected response time is minimized. 

gloabl.cpp executes with minimum overall expected response time. 
global_faulty1.cpp to global_faulty5.cpp are executed with false bids, and hence run with a higher overall expected response time. 
Each line in the file cmd_lines is treated as a job for execution. 

Technologies used : C++, OpenMPI
