/* Project 1 */
/* Daniel Lee 
   Yi Fan
   Joe Shvedsky */

//Process scheduling simulator for first-come first-serve,
//shortest remaining time, and round robin algorithms
//pretty bad code structure but it works

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void print_queue(char* queue,int count){ 
    printf("[Q");
    int i;
    for(i=0;i<count;i++){
	printf(" %c", queue[i]);
    }

    if(count==0){
        printf(" <empty>");
    }
    printf("]\n");

    fflush(stdout);
}

void remove_first(char* queue,int* count){
    int i;
    for(i=1;i<*count;i++){
        queue[i-1]=queue[i];
    }
    queue[i-1]='\0';
    (*count)--;
}

void insert(char* queue, int where, int* count,char ID){
    if(where==0){
	queue[(*count)]=ID;
    }
    else{
	int i;
	for(i=1;i<=where;i++){
	    queue[(*count)-i+1]=queue[(*count)-i];
	}
	queue[(*count)-where]=ID;
    }
    (*count)++;	
}


int main(int argc, char* argv[]){
    
    typedef struct{
        char ID;
        int arrivalTime;
        int CPUBurst;
        int numberOfBursts;
        int IOBurst;
        
        int CPUleft;
        int IOleft;
	    int Switchleft;
	    int tempArrive;
	    int tempBursts;
	    int roundNumber;

        int arrived;
        int switching;
	    int waiting;
	    int waitTime;
	    int running;
        int blocked;
     	int preempt;
        int terminated;
        
        int turnaround;
        
        int paused;
    }process;
    
    if (argc != 3){
        fprintf(stderr, "ERROR: Invalid arguments\nUSAGE: ./a.out <input-file> <stats-output-file>\n");
        return EXIT_FAILURE;
    }
    
    char* filename = argv[1];
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL){ /* If file cannot be opened */
        fprintf(stderr, "ERROR: Invalid input file format\n");
        return EXIT_FAILURE;
    }
    FILE* stats;
    char* output_file = argv[2];
    stats = fopen(output_file, "wt");
    if (stats == NULL){ /* If file cannot be opened */
        fprintf(stderr, "ERROR: Invalid input file format\n");
        return EXIT_FAILURE;
    }        
    int count = 0; /* Counts number of processes */
    int t_cs = 6;
    
    int initSize = 1;
    process* processes = malloc(initSize*sizeof(process));
    
    char line[512];
    
    while (fgets(line, sizeof(line), fp)){
        if (isalnum(line[0])>0){
            if (count == initSize){
                initSize = 2 * initSize;
                processes = realloc(processes, initSize*sizeof(process));
            }
            int i = 0;
            char* array[5];
            char* split = strtok(line, "|");
            while (split != NULL){
                array[i] = split;
                split = strtok(NULL, "|");
                i++;
            }
            processes[count].ID = *array[0];
            processes[count].arrivalTime = atoi(array[1]);
            processes[count].tempArrive = processes[count].arrivalTime;
            processes[count].arrived = 0;
            processes[count].switching = 0;
	        processes[count].CPUBurst = atoi(array[2]);
            processes[count].numberOfBursts = atoi(array[3]);
            processes[count].tempBursts = processes[count].numberOfBursts;
            processes[count].IOBurst = atoi(array[4]);
	        processes[count].roundNumber=0;
    	    processes[count].blocked=0;
    	    processes[count].waiting=0;
    	    processes[count].waitTime=0;
    	    processes[count].CPUleft=0;
    	    processes[count].turnaround=0;
    	    processes[count].IOleft=0;
    	    processes[count].terminated=0;
    	    processes[count].paused=0;
	        count++;
        }
    }
    
    /* for (i=0; i < 4; i++){
        printf("%c %d %d %d %d\n", processes[i].ID, processes[i].arrivalTime, processes[i].CPUBurst,processes[i].numberOfBursts, processes[i].IOBurst);
     } */
    
    fclose(fp);
    
    float average_burst = 0;
    int p;
    float total_bursts = 0;
    for(p = 0; p < count; ++p)
    {
        average_burst += (processes[p].CPUBurst * processes[p].numberOfBursts);
        total_bursts += processes[p].numberOfBursts;
    }
    average_burst /= total_bursts;
    fprintf(stats, "Algorithm FCFS\n");
    fprintf(stats, "-- average CPU burst time: %.2f ms\n", average_burst);
    
/*  working on FCFS */
    int time=0; /* real time */
    int running=0;
    /* int switching=0;
    int cont = 0;  */ 
    
    
    printf("time %dms: Simulator started for FCFS [Q <empty>]\n",time);
    fflush(stdout);
    char q[count]; /*processes queue */
    int count_q=0; /*count how many processes in queue */
    int turn_around = 0;
    int total_wait = 0;
    while(1){
     
        int j;

        for(j = 0; j < count; j++){
            if(processes[j].terminated == 0){ /* Only for processes that are not terminated */
                if (processes[j].waiting == 1 && processes[j].switching == 0 && processes[j].running == 0){
                    processes[j].waitTime++;
                    processes[j].turnaround++;
                }
                if(processes[j].tempArrive > 0){ /* If process has not arrived yet, decrement arrival */
                    processes[j].tempArrive--;
                }
                else if (processes[j].tempArrive == 0 && processes[j].arrived == 0){ /* Else if arrived */
                    q[count_q]=processes[j].ID;
                    count_q++;
                    printf("time %dms: Process %c arrived and added to ready queue ",time,processes[j].ID);
                    processes[j].waiting = 1;
                    processes[j].arrived = 1;
                    print_queue(q,count_q);
                }
		/*=========*/
		        if (processes[j].CPUleft > 0 && processes[j].running == 1){ /* If process is currently running, decrement CPUleft */
                    processes[j].CPUleft--;
                    turn_around++;
                }
                if (processes[j].CPUleft == 0 && processes[j].running == 1){ /* If process has no CPU time left and still running, switch to IO */
		            processes[j].running=0;
	   	            processes[j].switching=1;
		            processes[j].Switchleft=t_cs/2+1;
		    /* finshed cpuburst */
                    processes[j].tempBursts--;
                    if (processes[j].tempBursts > 0 ){ /* If there are sill a number of bursts left */
                        if (processes[j].tempBursts == 1){
                            printf("time %dms: Process %c completed a CPU burst; 1 burst to go ",time,processes[j].ID);

                        }
                        else {
                            printf("time %dms: Process %c completed a CPU burst; %d bursts to go ",time,processes[j].ID,processes[j].tempBursts);

                        }
                        print_queue(q,count_q);
                        turn_around += processes[j].turnaround;
                        processes[j].turnaround = 0;
                        printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",time,processes[j].ID, time+processes[j].IOBurst+t_cs/2);
                        print_queue(q,count_q);
                    }
                    else { /* Else terminate the process */
                        printf("time %dms: Process %c terminated ",time,processes[j].ID);
                        turn_around += processes[j].turnaround;
                        processes[j].turnaround = 0;
                        print_queue(q,count_q);
                    } 
		        }

	            if(processes[j].switching==1){
		            if(processes[j].Switchleft>0){
		                processes[j].turnaround++;
			            processes[j].Switchleft--;
			            if(processes[j].Switchleft>0){continue;}
		            }
		            if(running==0 ){/* ready to run and is in front of queue*/
		                processes[j].waiting=0;
		                total_wait += processes[j].waitTime;
		                processes[j].waitTime=0;
			            processes[j].switching=0;
		                processes[j].running=1;
		                processes[j].CPUleft = processes[j].CPUBurst;
		                running=1;
	   	                remove_first(q,&count_q);
		                printf("time %dms: Process %c started using the CPU ",time, processes[j].ID);
		                print_queue(q,count_q);
		            }
		            else if(running==1){ /* finished the switch after a cpuburst */
			            if(count_q>0 && q[0]<processes[j].ID){/* take care of some delay, when next process has already been scaned in this round */
			                int l;
			                for(l=0;l<count;l++){
				                if(processes[l].ID==q[0]){processes[l].Switchleft=-1;}
				                total_wait--;
			                }
			            }
			            running=0;
			            processes[j].switching=0;
			            processes[j].blocked = 1;
                        processes[j].IOleft = processes[j].IOBurst;
			            if(processes[j].tempBursts==0){
			                processes[j].terminated = 1;
			                processes[j].blocked=0;
			            }
		            }
                }		
		        if(processes[j].running == 0 && running == 0 && processes[j].blocked == 0 && q[0] == processes[j].ID){ /* If processes is in front of queue and CPU is idle */
		            processes[j].switching=1;
		            processes[j].Switchleft+=t_cs/2;
		        }           
                if (processes[j].IOleft > 0 && processes[j].blocked == 1){ /* If process is in the middle of IO burst */
                    processes[j].IOleft--;
                }
                else if (processes[j].IOleft == 0 && processes[j].blocked == 1){ /* If IO burst is finished, place process on ready queue */
                    processes[j].blocked = 0;
                    processes[j].CPUleft = processes[j].CPUBurst;
                    processes[j].waiting = 1;
                    q[count_q] = processes[j].ID;
		            if(count_q==0 && running==0){ /* deal with delay: newly added process will go to CPU imeadiately */ 
			            processes[j].Switchleft=-1;
		            }
                    count_q++;
                    printf("time %dms: Process %c completed I/O; added to ready queue ",time, processes[j].ID);
                    print_queue(q,count_q);    
                }
            }
        }
        int k;
        int terminated = 1;
        for (k = 0; k < count; k++){
            if (processes[k].terminated == 0){
                terminated = 0;
            }
        }
        if (terminated == 1){
            printf("time %dms: Simulator ended for FCFS\n", time);
            fflush(stdout);
            break;
        }
        time++;
    }
    
    fprintf(stats, "-- average wait time: %.2f ms\n", total_wait/total_bursts);
    fprintf(stats, "-- average turnaround time: %.2f ms\n", turn_around/total_bursts);
    fprintf(stats, "-- total number of context switches: %.0f\n", total_bursts);
    fprintf(stats, "-- total number of preemptions: 0\n");
    


    int i;
    for (i = 0; i < count; i++){
        processes[i].arrived = 0;
        processes[i].switching = 0;
        processes[i].roundNumber=0;
	    processes[i].blocked=0;
	    processes[i].waiting=0;
	    processes[i].waitTime=0;
        processes[i].CPUleft=0;
    	processes[i].turnaround=0;
        processes[i].IOleft=0;
	    processes[i].Switchleft=0;
        processes[i].terminated=0;
        processes[i].tempArrive = processes[i].arrivalTime;
        processes[i].tempBursts = processes[i].numberOfBursts;
        processes[i].paused=0;
    }
    
    printf("\n");
    fflush(stdout);

/*  working on SRT */
    i=0;
    time=0; /* real time */
    running=0;
    int preemption=0;
    int numpreempts=0;
    printf("time %dms: Simulator started for SRT [Q <empty>]\n",time);
    fflush(stdout);
    turn_around = 0;
    total_wait = 0;
    
    count_q=0; /*count how many processes in queue */
    while(1){
     
        int j;

        for(j = 0; j < count; j++){
            if(processes[j].terminated == 0){ /* Only for processes that are not terminated */
                /* if not arrived, do nothing */
                if (processes[j].blocked == 0 && processes[j].running == 0 && processes[j].switching == 0 && count_q > 0){
                    processes[j].turnaround++;
                    processes[j].waitTime++;
                }
                if (processes[j].arrivalTime == time && processes[j].arrived == 0){ /* if arrived */
                    processes[j].arrived = 1;
		            processes[j].CPUleft=processes[j].CPUBurst;                    
	  	            int where=0;
		            int k,runningID;			
		            for(k=0;k<count;k++){
			/* compare new arrived process with all others (remaining time) */
	                    if(k==j){continue;}
			            if(processes[k].blocked == 1 || processes[k].switching==1 || processes[k].arrived==0){continue;}
			            if(k<j && processes[j].CPUleft<processes[k].CPUleft){
			                where++;
			            }
		                else if(k>j && processes[j].CPUleft<=processes[k].CPUleft){/* if tie, process with lower ID will win*/	
			                where++;
			                if(processes[k].running==1 && processes[j].CPUleft==processes[k].CPUleft){
				                where--;
			                }
			            }
			            if(processes[k].running==1){runningID=k;}	
		            }
		            if(where<=count_q){/* insert to queue */
			            insert(q,where,&count_q,processes[j].ID);
			            printf("time %dms: Process %c arrived and added to ready queue ",time,processes[j].ID);
			            print_queue(q,count_q);
		            }
		            else{/*preempt runningID*/
    			        printf("time %dms: Process %c arrived and will preempt %c ",time,processes[j].ID,processes[runningID].ID);
    			        numpreempts++;
    			        print_queue(q,count_q);		 	
    			        processes[runningID].running=0;
    			        processes[runningID].paused=1;
    			        processes[runningID].switching=1;
    			        if(runningID>j){ /* deal with some early run for runnningID and delay for j */
    					 /* also need to compensate 1 ms for the cpuburst of runningID	*/
    			            processes[runningID].CPUleft--;								
    			            processes[runningID].Switchleft=1;
    			            processes[j].Switchleft=-1;
    		 	        }
    			        processes[runningID].Switchleft+=t_cs/2;
    			        processes[j].Switchleft+=t_cs/2;			
    			        preemption=1;
    			        processes[j].preempt=1;
   		            }
                }
		/*=========*/
        		if (processes[j].CPUleft > 0 && processes[j].running == 1){ /* If process is currently running, decrement CPUleft */
        		    processes[j].turnaround++;
                    processes[j].CPUleft--;
                }
                if (processes[j].CPUleft == 0 && processes[j].running == 1){ /* If process has no CPU time left and still running, switch to IO */
        		    processes[j].running=0;
        	   	    processes[j].switching=1;
        		    processes[j].Switchleft+=t_cs/2+1;
        		    /* finshed cpuburst */
                    processes[j].roundNumber++;
                    if (processes[j].roundNumber < processes[j].numberOfBursts ){ /* If there are sill a number of bursts left */
        			    if(processes[j].numberOfBursts-processes[j].roundNumber>1){
                            printf("time %dms: Process %c completed a CPU burst; %d bursts to go ",time,processes[j].ID,processes[j].numberOfBursts-processes[j].roundNumber);
        			    }
        			    else{
        				    printf("time %dms: Process %c completed a CPU burst; %d burst to go ",time,processes[j].ID,processes[j].numberOfBursts-processes[j].roundNumber);
        			    }
                        print_queue(q,count_q);
                        turn_around += processes[j].turnaround;
                        processes[j].turnaround = 0;
                        printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",time,processes[j].ID, time+processes[j].IOBurst+t_cs/2);
                        print_queue(q,count_q);
                        }
                    else { /* Else terminate the process */
                        turn_around += processes[j].turnaround;
                        processes[j].turnaround = 0;
                        printf("time %dms: Process %c terminated ",time,processes[j].ID);
                        print_queue(q,count_q);
                    } 
        		}
        	    if(processes[j].switching==1){
        		    if(processes[j].Switchleft>0){
        		        turn_around++;
        		        processes[j].Switchleft--;
        		        if(processes[j].Switchleft>0){continue;}
        		    }
        		    if(preemption==1){
        			    if(running==1){/* process that be preempted done with switching */
        			        running=0;
        			        insert(q,count_q,&count_q,processes[j].ID);
        			        processes[j].switching=0;
        			    }
        			    else if (running==0){ /*preemptiong done,ready to run */
        			        preemption=0;
        			        printf("time %dms: Process %c started using the CPU ",time, processes[j].ID);
        		            print_queue(q,count_q);
        			        processes[j].running=1;
        			        running=1;
        			        processes[j].switching=0;
        			        processes[j].preempt=0;
        			    }
        		    }
        		    else{	
        			    if(running==0){/* ready to run */
        			        processes[j].switching=0;
        			        processes[j].running=1;
        		            running=1;
          	    	        remove_first(q,&count_q);
        			        if(processes[j].CPUBurst==processes[j].CPUleft){
        				        printf("time %dms: Process %c started using the CPU ",time, processes[j].ID);
        			        }
        			        else{
        			            processes[j].paused=0;
        				        printf("time %dms: Process %c started using the CPU with %dms remaining ",time, processes[j].ID,processes[j].CPUleft);
        			        }
        			        total_wait += processes[j].waitTime;
		                    processes[j].waitTime=0;
        		            print_queue(q,count_q);
                        }
        		        else if(running==1){ /* finished the switch after a cpuburst */
        			        if(count_q>0 && q[0]<processes[j].ID){/* take care of some delay, when next process has already been scaned in this round */
        			            int l;
        		  	            for(l=0;l<count;l++){
        				            if(processes[l].ID==q[0]){processes[l].Switchleft=-1;}
        			            }
        		 	        }
        			        running=0;
        			        processes[j].switching=0;
        			        processes[j].blocked = 1;
                            processes[j].IOleft = processes[j].IOBurst;
        			        if(processes[j].roundNumber==processes[j].numberOfBursts){
        			            processes[j].terminated = 1;
        			            processes[j].blocked=0;
        			        }
        		        }
                    }		
         		}     
        		if(processes[j].preempt==1 && running==0){
        		    processes[j].switching=1;
        		}
        		if(processes[j].running == 0 && preemption == 0 && running == 0 && processes[j].blocked == 0 && q[0] == processes[j].ID){ /* If processes is in front of queue and CPU is idle */
        		    processes[j].switching=1;
        		    processes[j].Switchleft+=t_cs/2;
        		}           
                if (processes[j].IOleft > 0 && processes[j].blocked == 1){ /* If process is in the middle of IO burst */
                    processes[j].IOleft--;
                }
                else if (processes[j].IOleft == 0 && processes[j].blocked == 1){ /* If IO burst is finished, place process on ready queue */
                    processes[j].blocked = 0;
                    processes[j].CPUleft = processes[j].CPUBurst;
                    
        		    int where=0;
        		    int k,runningID;			
        		    for(k=0;k<count;k++){
        			/* compare new arrived process with all others (remaining time) */
        	            if(k==j){continue;}
        			    if(processes[k].blocked == 1 || processes[k].switching==1 || processes[k].arrived==0){continue;}
        			    if(k<j && processes[j].CPUleft<processes[k].CPUleft){
        			        where++;
        				}
        				else if(k>j && processes[j].CPUleft<=processes[k].CPUleft){/* if tie, process with lower ID will win*/	
        			        where++;
        			        if(processes[k].running==1 && processes[j].CPUleft==processes[k].CPUleft){
        				        where--;
        			        }
        			    }
        			    if(processes[k].running==1){runningID=k;}	
                    }
        		    if(where<=count_q){/* insert to queue */
        			    insert(q,where,&count_q,processes[j].ID);
        		        if(count_q==1 && running==0){ /* deal with delay: newly added process will go to CPU imeadiately */ 
        			        processes[j].Switchleft=-1;
        		        }
        			    printf("time %dms: Process %c completed I/O; added to ready queue ",time, processes[j].ID);
        			    print_queue(q,count_q);
                    }
        		    else{/*preemt runningID*/
        			    printf("time %dms: Process %c completed I/O and will preempt %c ",time,processes[j].ID,processes[runningID].ID);
        			    numpreempts++;
        			    print_queue(q,count_q);		 	
        			    processes[runningID].running=0;
        			    processes[runningID].switching=1;
        			    processes[runningID].paused=1;
        			    if(runningID>j){ /* deal with some early run for runnningID and delay for j */
        			        /* also need to compensate 1 ms for the cpuleft of runningID	*/
        			        processes[runningID].CPUleft--;
        			        processes[runningID].Switchleft=1;
        			        processes[j].Switchleft=-1;
        		 	    }
        			    processes[runningID].Switchleft+=t_cs/2;
        			    processes[j].Switchleft+=t_cs/2;			
        			    preemption=1;
        			    processes[j].preempt=1;
           		    }   
                }
            }
        }
        int k;
        int terminated = 1;
        for (k = 0; k < count; k++){
            if (processes[k].terminated == 0){
                terminated = 0;
            }
        }
        if (terminated == 1){
            printf("time %dms: Simulator ended for SRT\n", time);
            fflush(stdout);
            break;
        }
        time++;
        i++;
    }
    
    fprintf(stats, "Algorithm SRT\n");
    fprintf(stats, "-- average CPU burst time: %.2f ms\n", average_burst);
    fprintf(stats, "-- average wait time: %.2f ms\n", total_wait/total_bursts);
    fprintf(stats, "-- average turnaround time: %.2f ms\n", (turn_around)/total_bursts);
    fprintf(stats, "-- total number of context switches: %d\n", (int)total_bursts + numpreempts);
    fprintf(stats, "-- total number of preemptions: %d\n", numpreempts);
    
    for (i = 0; i < count; i++){
        processes[i].arrived = 0;
        processes[i].switching = 0;
        processes[i].roundNumber=0;
	    processes[i].blocked=0;
	    processes[i].waiting=0;
	    processes[i].waitTime=0;
        processes[i].CPUleft=0;
    	processes[i].turnaround=0;
        processes[i].IOleft=0;
	    processes[i].Switchleft=0;
        processes[i].terminated=0;
        processes[i].tempArrive = processes[i].arrivalTime;
        processes[i].tempBursts = processes[i].numberOfBursts;
        processes[i].paused = 0;
    }
    
    printf("\n");
    
    time=0; /* real time */
    running=0;
    /* switching=0;
    cont = 0;  */   
    int t_slice = 94;
    
    count_q=0; /*count how many processes in queue */
    turn_around = 0;
    total_wait = 0;
    turn_around = 0;
    total_wait = 0;
    numpreempts = 0;
    int slice = t_slice;
    printf("time %dms: Simulator started for RR [Q <empty>]\n",time);
    fflush(stdout);
    while(1){
        int j;

        for(j = 0; j < count; j++){
            if(processes[j].terminated == 0){ /* Only for processes that are not terminated */
                /*if (slice == 0){
                    if (processes[j].CPUleft > 0 && processes[j].running == 1){
                        if (count_q > 0){
                            processes[j].preempt = 1;
                            processes[j].switching = 1;
		                    processes[j].Switchleft=t_cs/2+1;
                            processes[j].running = 0;
                            processes[j].waiting = 1;
                            printf("time %dms: Time slice expired; process %c preempted with %d ms to go ",time, processes[j].ID, processes[j].CPUleft);
                            print_queue(q,count_q);
                            q[count_q] = processes[j].ID;
                            count_q++;
                        }
                        else {
                            printf("time %dms: Time slice expired; no preemption because ready queue is empty ",time);
                            print_queue(q,count_q);
                        }
                    }
                }*/
                if ((processes[j].waiting == 1 || processes[j].paused == 1) && processes[j].switching == 0){
                    processes[j].turnaround++;
                    processes[j].waitTime++;
                }
                if(processes[j].tempArrive > 0){ /* If process has not arrived yet, decrement arrival */
                    processes[j].tempArrive--;
                }
                else if (processes[j].tempArrive == 0 && processes[j].arrived == 0){ /* Else if arrived */
                    q[count_q]=processes[j].ID;
                    count_q++;
                    printf("time %dms: Process %c arrived and added to ready queue ",time,processes[j].ID);
                    processes[j].waiting = 1;
                    processes[j].arrived = 1;
                    print_queue(q,count_q);
                }
		/*=========*/
		        if (processes[j].CPUleft > 0 && processes[j].running == 1){ /* If process is currently running, decrement CPUleft */
                    processes[j].CPUleft--;
                    slice--;
                    turn_around++;
                }


                if (slice == 0 && processes[j].running == 1 && processes[j].CPUleft!=0){
                    if(count_q == 0){
                        printf("time %dms: Time slice expired; no preemption because ready queue is empty ",time);
                        print_queue(q,count_q);
                        slice = t_slice;
                    }
                    else{
                        processes[j].paused=1;
                        processes[j].running=0;
                        processes[j].switching=1;
                        processes[j].Switchleft=t_cs/2+1;
                        printf("time %dms: Time slice expired; process %c preempted with %dms to go ",time, processes[j].ID, processes[j].CPUleft);
                        numpreempts++;
                        print_queue(q,count_q);
                        q[count_q] = processes[j].ID;
    		            
                        count_q++;
                    }
                }
                if (processes[j].CPUleft == 0 && processes[j].running == 1){ /* If process has no CPU time left and still running, switch to IO */
		            processes[j].running=0;
	   	            processes[j].switching=1;
		            processes[j].Switchleft=t_cs/2+1;
		    /* finshed cpuburst */
                    processes[j].tempBursts--;
                    if (processes[j].tempBursts > 0 ){ /* If there are sill a number of bursts left */
                        if (processes[j].tempBursts == 1){
                            printf("time %dms: Process %c completed a CPU burst; 1 burst to go ",time,processes[j].ID);
                        }
                        else {
                            printf("time %dms: Process %c completed a CPU burst; %d bursts to go ",time,processes[j].ID,processes[j].tempBursts);
                        }
                        print_queue(q,count_q);
                        turn_around += processes[j].turnaround;
                        processes[j].turnaround = 0;
                        printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",time,processes[j].ID, time+processes[j].IOBurst+t_cs/2);
                        print_queue(q,count_q);
                    }
                    else { /* Else terminate the process */
                        printf("time %dms: Process %c terminated ",time,processes[j].ID);
                        turn_around += processes[j].turnaround;
                        processes[j].turnaround = 0;
                        print_queue(q,count_q);
                    } 
		        }

	            if(processes[j].switching==1){
		            if(processes[j].Switchleft>0){
		                turn_around++;
			            processes[j].Switchleft--;
			            if(processes[j].Switchleft>0){continue;}
		            }
		            if(running==0 && q[0] == processes[j].ID){/* ready to run */
		            	processes[j].waiting=0;
			            processes[j].switching=0;
		                processes[j].running=1;
		                if (processes[j].paused == 0){
		                    processes[j].CPUleft = processes[j].CPUBurst;
		                    running=1;
		                    slice = t_slice;
	   	                    remove_first(q,&count_q);
		                    printf("time %dms: Process %c started using the CPU ",time, processes[j].ID);
		                    processes[j].waitTime--;
		                    print_queue(q,count_q);
		                }
		                else{
		                    running=1;
		                    processes[j].paused=0;
		                    slice = t_slice;
		                    remove_first(q,&count_q);
		                    processes[j].waitTime--;
		                    printf("time %dms: Process %c started using the CPU with %dms remaining ",time, processes[j].ID, processes[j].CPUleft);
		                    print_queue(q,count_q); 
		                }
		                total_wait += processes[j].waitTime;
		                processes[j].waitTime=0;
		            }
		            else if(running==1){ /* finished the switch after a cpuburst */
			            if(count_q>0 && q[0]<processes[j].ID){/* take care of some delay, when next process has already been scaned in this round */
			                int l;
			                for(l=0;l<j;l++){
				                if(processes[l].ID==q[0]){processes[l].Switchleft=-1;}
			                }
			            }
			            running=0;
			            processes[j].switching=0;
			            if (processes[j].paused == 0){
			                processes[j].blocked = 1;
                            processes[j].IOleft = processes[j].IOBurst;
			            }
			            if(processes[j].tempBursts==0){
			                processes[j].terminated = 1;
			                processes[j].blocked=0;
			            }
		            }
                }		
		        if(processes[j].running == 0 && running == 0 && processes[j].blocked == 0 && q[0] == processes[j].ID){ /* If processes is in front of queue and CPU is idle */
		            processes[j].switching=1;
		            processes[j].Switchleft+=t_cs/2;
		        }           
                if (processes[j].IOleft > 0 && processes[j].blocked == 1){ /* If process is in the middle of IO burst */
                    processes[j].IOleft--;
                }
                else if (processes[j].IOleft == 0 && processes[j].blocked == 1){ /* If IO burst is finished, place process on ready queue */
                    processes[j].blocked = 0;
                    processes[j].CPUleft = processes[j].CPUBurst;
                    processes[j].waiting = 1;
                    q[count_q] = processes[j].ID;
		            if(count_q==0 && running==0){ /* deal with delay: newly added process will go to CPU immediately */ 
			            processes[j].Switchleft=-1;
		            }
                    count_q++;
                    printf("time %dms: Process %c completed I/O; added to ready queue ",time, processes[j].ID);
                    print_queue(q,count_q);    
                }
            }
        }
        int k;
        int terminated = 1;
        for (k = 0; k < count; k++){
            if (processes[k].terminated == 0){
                terminated = 0;
            }
        }
        if (terminated == 1){
            printf("time %dms: Simulator ended for RR\n", time);
            fflush(stdout);
            break;
        }
        time++;
    }
    

    fprintf(stats, "Algorithm RR\n");
    fprintf(stats, "-- average CPU burst time: %.2f ms\n", average_burst);
    fprintf(stats, "-- average wait time: %.2f ms\n", total_wait/total_bursts);
    fprintf(stats, "-- average turnaround time: %.2f ms\n", (turn_around)/total_bursts);
    fprintf(stats, "-- total number of context switches: %d\n", (int)total_bursts + numpreempts);
    fprintf(stats, "-- total number of preemptions: %d\n", numpreempts);
    
    
    fclose(stats);
    
    free(processes);
    return 0;
}
