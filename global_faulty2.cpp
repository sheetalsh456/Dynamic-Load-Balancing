#include <iostream>
#include <algorithm>
#include <math.h>
#include <cstdlib>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/status.hpp>
#define ELECTED_TAG 2
using namespace boost::mpi;
using namespace std;
environment env;
communicator world;
// the processing rates of the 4 processors of the system
float rates[4]={1700,1700,1700,1700};
float bids[4],total_arr_rate,load[4],cost[4],total_cost,profit[4],pay[4],total_profit=0.0;
int slave_bids;
float total_load=0.0;
int nprocs;
void calculate_overall_execution_time()
{
	int i;
	float execution_time=0.0;
	for(i=0;i<slave_bids;i++)
	{
		execution_time += (load[i]/(rates[i+1]-load[i]));
	}
	printf("\nOverall expected execution time : %f\n",(execution_time/total_arr_rate)*1000);
}
void master()
{
	int i;
	sort(bids,bids+3);
	float c,bids_total = 0.0,sqrt_bids=0.0;
	for(i=0;i<slave_bids;i++)
	{
		bids_total += 1.0/bids[i];
		sqrt_bids += sqrt(1.0/bids[i]);
	}
	c = (bids_total - total_arr_rate)/(sqrt_bids);
	int n = slave_bids;
	while(c > sqrt(1.0/bids[slave_bids-1]))
	{
		load[n]=0.0;
		n--;
		for(i=0;i<n;i++)
		{
			bids_total += 1.0/bids[i];
			sqrt_bids += sqrt(1.0/bids[i]);
		}
		c = (bids_total - total_arr_rate)/(sqrt_bids);
	}
	for(i=0;i<slave_bids;i++)
	{
		load[i] = 1.0/bids[i] - c * sqrt(1.0/bids[i]);
		MPI_Send(&load[i],1,MPI_FLOAT,i+1,0,world);
	}
	calculate_overall_execution_time();
	printf("Total load : %f\n",total_arr_rate);
}
int main(int argc, char const *argv[])
{
	int no_of_jobs=0,world_rank;
	
	MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	if (nprocs < 2) {
        	printf ("Number of processors < 2\nExiting\n");
        	exit (1);
    	}
    slave_bids = nprocs-1;
    // Leader election algorithm
	bool terminated=false;
	int inmsg[2];	
	int msg[2];
	msg[0]=world_rank;
	msg[1]=0;
	MPI_Status Stat;
	int min=world_rank; //initially the minimum ID is node's own ID
	int lcounter=0;	//local counter of received messages (for protocol)
	int lmc=0;		//local counter of send and received messages (for analysis)
	int dummy=1;
	int leader;
	//send my ID with counter to the right
	MPI_Send(&msg, 2, MPI_INT, (world_rank+1)%nprocs, 1, MPI_COMM_WORLD);
	lmc++; //first message sent	

	while (!terminated){
		//receive ID with counter from left
		MPI_Recv(&inmsg, 2, MPI_INT, (world_rank-1)%nprocs, 1, MPI_COMM_WORLD, &Stat);
		lmc++;
		lcounter++;	//increment local counter
		if (min>inmsg[0]) min=inmsg[0];	//update minimum value if needed
		inmsg[1]++;	//increment the counter before forwarding

		if (inmsg[0]==world_rank && lcounter==(inmsg[1])) {
			terminated=true;
		}
		//forward received ID
		MPI_Send(&inmsg, 2, MPI_INT, (world_rank+1)%nprocs, 1, MPI_COMM_WORLD);		
	}
	leader = min;
	FILE *fp;
	// The file cmd_lines contains the jobs to be executed. 
	fp = fopen("cmd_lines","r");
    if(fp == NULL)
    {
    	printf("\nCannot open file");
    	return 0;
    }
    char c;
    for(c = getc(fp); c!=EOF ; c=getc(fp))
    {
    	if(c=='\n')
    		no_of_jobs++;
    }
    // This indicates the total number of lines in cmd_lines file, to be executed. Each line is treated as a job.
    total_arr_rate = no_of_jobs;
    fclose(fp);
    // indicates a slave process
    if(world_rank!=leader)
	{    	
		float proc_load;
		// Sending bids to slaves
		MPI_Send(&rates[world_rank],1,MPI_FLOAT,leader,0,world);
		// Receiving load from master
		MPI_Recv(&proc_load,1,MPI_FLOAT,leader,0,world,MPI_STATUS_IGNORE);
		printf("Load received by  processor %d = %f\n",world_rank,proc_load);
		load[world_rank-1]=proc_load;
		total_load+=load[world_rank-1];
	}
	// indicates the master process
	else if (world_rank == leader)
	{
		bids[0]=0.097;
		bids[1]=0.563;
		bids[2]=0.456;
		master();
	}
}

