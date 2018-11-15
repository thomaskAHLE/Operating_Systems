#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "prioque.h"

#define DEBUG_MODE 0

typedef struct {
	unsigned long cpu_burst;
	unsigned long io_burst;
	int repeat;
} ProcessBehavior;

typedef  struct {
	unsigned long arrival_time;
	unsigned long return_from_IO_time;
	unsigned long current_cpu_burst;
	unsigned long total_cpu;
	unsigned long total_IO;
	unsigned long remaining_quantum;
	int pid;
	int current_queue_level;
	int good;
	int bad;
	int executing;
	Queue behaviors;
} Process;

//Global Variables
Queue toBeExecuted, ioQueue, mlfbQueue, finishedQueue;
Queue *tbeQ = &toBeExecuted, *ioQ = &ioQueue, *mlfbQ =&mlfbQueue, *finQ = &finishedQueue ;
unsigned long  mlfbQ_time = 0;
unsigned long  null_cycles = 0;
unsigned long quantums[3] = {10,30,100};
int bad[3] = {1,2,3};
int good[3] = {3,2,1};
unsigned long preemption_count = 0lu;
void init_process(Process *p)
{
	p->pid =0;
	p->arrival_time = 0lu;
	p->return_from_IO_time = 0lu;
	p->remaining_quantum = 0lu;
	p->current_queue_level = 1;
	p->total_cpu = 0lu;
	p->total_IO = 0lu;
	p->good = 0;
	p->bad = 0;
	p->executing = 0;
	init_queue(&(p->behaviors), sizeof(ProcessBehavior),1,0,0);
}

//prints processes -for testing purposes only
void print_process(Process * p)
{
	rewind_queue(&(p->behaviors));
	printf("Behavior Length = %d \n", queue_length(&(p->behaviors)));
	ProcessBehavior * pb_ptr;
	while(!end_of_queue(&(p->behaviors)))
	{
		pb_ptr = (ProcessBehavior *) pointer_to_current(&(p->behaviors));
		printf("Behavior for process %d with charge against cpy %lu cputime: %lu ioTime %lu and repeat %d \n", p->pid,p->remaining_quantum,  pb_ptr->cpu_burst, pb_ptr->io_burst, pb_ptr->repeat);
		next_element(&(p->behaviors));
	}
	rewind_queue(&(p->behaviors));
}

void print_processes (Queue * q)
{
	rewind_queue(q);
	Process p;
	printf("Queue Length:%d, \n", queue_length(q));
	ProcessBehavior pb;
	while(! end_of_queue(q))
	{
		peek_at_current(q, &p);
		printf("Process with pid: %d will start at time %lu \n", p.pid, p.arrival_time);
		print_process(&p);
		next_element(q);
	}	
	rewind_queue(q);
}

//compare functions
int compareArrivalTime(void * p1, void * p2)
{
	return ((Process*)p1)->arrival_time < ((Process*)p2)->arrival_time;
}

int compareIOReturnTime( void *p1, void * p2)
{
	return ((Process*)p1)->return_from_IO_time < ((Process*)p2)->return_from_IO_time;
}

// initializing queues
void init_queues()
{
	init_queue(tbeQ, sizeof(Process),1, compareArrivalTime, 0);
	init_queue(ioQ, sizeof(Process), 1, compareIOReturnTime, 0);
	init_queue(mlfbQ, sizeof(Process), 1,0, 0);
	init_queue(finQ, sizeof(Process), 1, 0, 0);
}

//destroys all queues
void destroy_queues()
{
	destroy_queue(tbeQ);
	destroy_queue(ioQ);
	destroy_queue(mlfbQ);
	destroy_queue(finQ);
}

//end of program printing and destroying process behavior queues - tested - works.
void total_CPU_Print()
{
	rewind_queue(finQ);
	printf("Scheduler shutdown at time %lu \n", mlfbQ_time);
	printf("Total CPU usage for all processes scheduled: \n");
	printf("Process <<null>>: \t %lu time units.\n", null_cycles);
	Process *p;
	while(!end_of_queue(finQ))
	{
		p = (Process *) pointer_to_current(finQ);
		printf("Process %d: \t %lu time units.\n", p->pid, p->total_cpu);
		destroy_queue(&(p->behaviors));
		next_element(finQ);
	}
}

void refresh_cpu_burst( Process *p)
{
	ProcessBehavior * pb = pointer_to_current(&(p->behaviors));
	p->current_cpu_burst = pb->cpu_burst;
}

// parses the file and puts it into toBeExecutedQueue -tested -  works
void parseFile(char* fileName)
{
	FILE* fp = fopen(fileName, "r");
	if(!fp)
	{
		fputs("File could not be opened\n", stderr);
		return;
	}
	int previousPID = -1;
	char buf [1000];
	Process p;
	int ready_to_add = 0;
	while(fgets(buf, 1000,fp))
	{
		unsigned long arrival_time;
		int pid;
		unsigned long run_time;
		unsigned long io_time;
		int repeat;
		sscanf(buf,"%lu %d %lu %lu %d", &arrival_time, &pid, &run_time, &io_time, &repeat);
		if( previousPID != pid)
		{
			if(ready_to_add)
			{
				add_to_queue(tbeQ,&p, p.arrival_time);
			}
			init_process(&p);
			p.arrival_time = arrival_time;
			p.pid = pid;
			previousPID = pid;
			ready_to_add = 1;
		}
		ProcessBehavior pb;
		pb.cpu_burst = run_time;
		pb.io_burst = io_time;
		pb.repeat = repeat;
		add_to_queue(&(p.behaviors), &pb, 0);
	
	}
	add_to_queue(tbeQ, &p, p.arrival_time);
	fclose(fp);
}

void read_process_descriptions()
{
	Process p;
	ProcessBehavior b;
	int pid = 0, first =1;
	unsigned long arrival;
	init_process(&p);
	arrival = 0lu;
	while(scanf("%lu", &arrival) != EOF)
	{
		scanf("%d %lu %lu %d", &pid, &b.cpu_burst, &b.io_burst,&b.repeat);
		if(!first && p.pid != pid) 
		{
			add_to_queue(tbeQ, &p, p.arrival_time);
			init_process(&p);
		}
		p.pid = pid;
		p.arrival_time = arrival;
		first = 0;
		add_to_queue(&(p.behaviors),&b,1);
	}
	add_to_queue(tbeQ, &p, p.arrival_time);
}

//removes current behavior and adds a new one if there is one  -semi-tested- needs some changes
void  next_behavior(Process * p)
{
	int len = queue_length(&(p->behaviors));
	if(DEBUG_MODE)
	{
		printf("Trying to get next behavior for process %d behavior queue length %d \n", p->pid,len);
	}
	if(len)
	{
		delete_current(&(p->behaviors));
		if( len  > 1)
		{
			rewind_queue(&(p->behaviors));
			refresh_cpu_burst(p);	
			p->remaining_quantum = quantums[p->current_queue_level -1];			
		} 
	}
}

void preempt_process()
{
	Process p;
	remove_from_front(mlfbQ, &p);
	if(DEBUG_MODE)
	{
		printf("Preempted process %d \n", p.pid);
	}
	p.executing = 0;
	p.remaining_quantum = quantums[p.current_queue_level -1];
	printf("QUEUED: Process %d queued at level %d at time %lu \n", p.pid, p.current_queue_level, mlfbQ_time);
	add_to_queue(mlfbQ, &p, p.current_queue_level);
}

void add_new_process(Process *p, Process *p_current, int current_exists)
{
	p->remaining_quantum = quantums[p->current_queue_level -1];
	printf("CREATE: Process %d entered the ready queue at time %lu\n",p->pid, mlfbQ_time);	
	if(DEBUG_MODE)
	{
		printf("new process with behavior length %d \n",queue_length(&(p->behaviors)));
	}
	if(current_exists && p_current->executing && p_current->current_queue_level > 1)
	{
		preempt_process();
	}
	if(DEBUG_MODE)
	{
		printf("current level: %d \n", p->current_queue_level);
	}
	refresh_cpu_burst(p);
	if(DEBUG_MODE)
	{
		printf("New cpu_burst %lu \n ", p->current_cpu_burst);
	}
	add_to_queue(mlfbQ, p, p->current_queue_level);	
}

void return_from_IO (Process *p_io, Process *p_current, int current_exists)
{
	if(DEBUG_MODE)
	{
		printf("Process %d returned from I/O remaining behaviors %d \n",p_io->pid,queue_length(&(p_io->behaviors )));
	}
	if( current_exists && p_current->executing && p_io->current_queue_level <  p_current->current_queue_level)
	{
		preempt_process();
	}
	if(queue_length(&(p_io->behaviors)) > 1)
	{
		ProcessBehavior * pb = pointer_to_current(&(p_io->behaviors));
		if(pb->repeat == 0)
		{
			next_behavior(p_io);
		}
	}
	add_to_queue(mlfbQ, p_io, p_io->current_queue_level);
}

void check_for_return_from_IO()
{
	if(!empty_queue(ioQ) && current_priority(ioQ) == mlfbQ_time)
	{
		Process p_io;
		Process* p_current;
		int current_exists = 0;
		int added = 0;
		if(!empty_queue(mlfbQ))
		{
			p_current = pointer_to_current(mlfbQ);
			current_exists = 1;
		}	
		while(!empty_queue(ioQ) && current_priority(ioQ) == mlfbQ_time)
		{
			remove_from_front(ioQ, &p_io);
			return_from_IO(&p_io, p_current, current_exists);	
		}
	}
}
void check_for_new()
{
	if(! empty_queue(tbeQ) && current_priority(tbeQ) == mlfbQ_time)
	{	
		Process p_new;
		Process*  p_current;
		int current_exists =0;
		if(! empty_queue(mlfbQ))
		{
			current_exists = 1;
			p_current = pointer_to_current(mlfbQ); 
		}
		while( !empty_queue(tbeQ) && current_priority(tbeQ) == mlfbQ_time)
		{	
	    	remove_from_front(tbeQ,&p_new);
			add_new_process(&p_new, p_current, current_exists);
		}	
	}
}

void check_for_promotion(Process *p)
{
	if(p->good < 3)
	{
		p->good +=1;
	}

	if(p->current_queue_level > 1 && p->good >= good[ p->current_queue_level - 1])
	{
		if(DEBUG_MODE)
		{
			printf("Process %d promoted to level %d \n", p->pid, p->current_queue_level );
		}
		p->current_queue_level -=1;
		p->bad = 0;
		p->good = 0;
	}
}

void check_for_demotion(Process *p)
{
	if(p->bad < 3)
	{
		p->bad += 1;
	}
	if(DEBUG_MODE)
	{
		printf("Process %d bad = %d \n", p->pid, p->bad);
		printf("bad for quantum level = %d\n", bad[p->current_queue_level-1]);
	}
	if(p->current_queue_level < 3 && p->bad >= bad[p->current_queue_level -1])
	{
		p->current_queue_level += 1;
		if(DEBUG_MODE)
		{
			printf("Process %d demoted to level %d \n", p->pid, p->current_queue_level);
		}
		p->good = 0;
		p->bad = 0;
	}
}

void cpu_burst_finished(Process *p)
{
	p->executing = 0;
	if(0 < p->remaining_quantum )
	{
		check_for_promotion(p);
	}
	ProcessBehavior *pb = pointer_to_current(&(p->behaviors));
	if(pb->repeat > 0 )
	{
		p->bad = 0;
		p->return_from_IO_time = mlfbQ_time + pb->io_burst;
		pb->repeat -= 1;
		p->total_IO += pb->io_burst;
		refresh_cpu_burst(p);
		p->remaining_quantum = quantums[p->current_queue_level -1];
		printf("I/O: Process %d blocked for I/O at time %lu  \n", p->pid, mlfbQ_time);
		add_to_queue(ioQ, p, p->return_from_IO_time);
	}
	else
	{
		printf("FINISHED: Process %d finished at time %lu \n", p->pid, mlfbQ_time);
		add_to_queue(finQ, p, p->arrival_time);

	}
}

void execute()
{
	if(! empty_queue(mlfbQ))
	{
		Process*  p;
		p = pointer_to_current(mlfbQ);
		if(!(p->executing))
		{
			p->executing = 1;
			printf("RUN: Process %d started execution from level %d at time %lu; Wants to execute for %lu \n", p->pid, p->current_queue_level,mlfbQ_time, p->current_cpu_burst);
			if(DEBUG_MODE)
			{
				printf("current cpu burst %lu \n", p->current_cpu_burst);
			}
		}
		p->current_cpu_burst -= 1lu;
		p->remaining_quantum -= 1lu;
		p->total_cpu += 1lu;
		if(DEBUG_MODE)
		{
			printf("current cpu burst %lu \n", p->current_cpu_burst);
			printf("CPU charge against quantum %lu \n", p->remaining_quantum);
		}
		if( p->remaining_quantum == 0 )
		{
			preemption_count += 1lu;
			check_for_demotion(p);
			if(p->current_cpu_burst  > 0)
			{	
				preempt_process();
			}
		}
		if(p->current_cpu_burst == 0)
		{	
			Process p_finished;
			remove_from_front(mlfbQ, &p_finished);
			cpu_burst_finished(&p_finished);
		}
		
	 	
	}	
	else
	{
		null_cycles++;
	}
}


int main()
{
	init_queues();
//	parseFile("file2.txt");

	read_process_descriptions();
//	print_processes(tbeQ);
	while(! empty_queue(tbeQ) || !empty_queue(mlfbQ) || !empty_queue(ioQ) )
	{
		check_for_new();
		execute();
		check_for_return_from_IO();
		mlfbQ_time += 1lu;
	}
	total_CPU_Print();
	destroy_queues();
	printf("Preemption count %lu \n", preemption_count);	
}

