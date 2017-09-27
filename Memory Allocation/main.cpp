/*Joseph Shvedsky - shvedj
  Yi Fan          - fany4
  Daniel Lee      - leed19*/
  

#include <string> 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include "MemAndProcess.h"

#define Inf 100000
using namespace std;

void nonContiguous(std::vector<process> processes){
  memory table;
  int t = 0;
  int isDone = 0;
  cout << "time " << t << "ms: Simulator started (Non-contiguous)" << endl;
  sort(processes.begin(), processes.end());
  while (isDone != (int)processes.size()){
  	vector<int> toAdd;
  	for (int i = 0; i < (int) processes.size(); i++){
  		if (t == processes[i].arrival[0]){
  			toAdd.push_back(i);
  		}
  		if (processes[i].run){
  			processes[i].runtime[0]--;
  		}
  		if (processes[i].runtime[0] == 0 && processes[i].flag == false){
  			cout << "time " << t << "ms: ";
  			table.nonc_remove(processes[i]);
  		}
  		if (processes[i].runtime.size() == 0 && processes[i].flag == false){
  			processes[i].flag = true;
  			isDone++;
  		}
  	}
  	vector<int>::iterator it;
  	for(it = toAdd.begin() ; it < toAdd.end(); it++){
  		cout << "time " << t << "ms: " << "Process " << processes[*it].name << " arrived (" << "requires " << processes[*it].mem << " frames)" << endl;
  		cout << "time " << t << "ms: ";
  		table.nonc_add(processes[*it]);
  	}
  	if (isDone == (int)processes.size()){
  		cout << "time " << t << "ms: " << "Simulator ended (Non-contiguous)" << endl;
  	}
  	t++;
  }
}

bool isfinished(vector<process> &p){ /* if all processes are done */
	int NofP=p.size();
	bool finished = true;
	for(int i=0;i<NofP;i++){
	    if(p[i].flag==false){
			finished = false;
			break;
		}
	
	}
	return finished;
}

void next_event(int t,int &next_t,int &next_p,bool &isArrFst,vector<process>& p){
	/* check first leave */
	int NofP=p.size();
	for(int i=0;i<NofP;i++){
		if(!p[i].run){continue;}
		int leave=Inf;
		for(int j=0;j<p[i].num;j++){
			leave=p[i].arrival[j]+p[i].runtime[j];
			if(leave>=t){
				break;
			}else if(j==p[i].num-1){
				leave=Inf;
			}
		}
		if(next_t>leave){
			next_t=leave;
			next_p=i;
		}
	}
	/* check first arrival */
	for(int i=0;i<NofP;i++){
		if(p[i].run){continue;}
		int arriv=Inf;
		for(int j=0;j<p[i].num;j++){
			arriv=p[i].arrival[j];
			if(arriv>=t){
				
				break;
			}else if(j==p[i].num-1){
				arriv = Inf;
			}
		}
		if(next_t>arriv){
			next_t=arriv;
			next_p=i;
			isArrFst=true;
		}
	}
}

void next_fit(vector<process> p)
{
	cout << "time 0ms: Simulator started (Contiguous -- Next-Fit)" << endl;
	memory table;	
	int t = 0;
	int t_dfrag = 0;
	int last_placed = 0;
	while(1){
		if(isfinished(p)){
			cout << "time " << t+t_dfrag
			<< "ms: Simulator ended (Contiguous -- Next-Fit)" << endl;
			break;		
			
		}
		int next_t = Inf;
		int next_p = 0;
		bool isArrFst = false;
		next_event(t, next_t, next_p, isArrFst, p);
		if(isArrFst){
			t = next_t;
			cout << "time " << t+t_dfrag << "ms: Process " << p[next_p].name
			<< " arrived (requires " << p[next_p].mem << " frames)" << endl;
			bool success = table.next_add(p[next_p], last_placed);
			if(!success){
				/* skip this process */
				cout << "time " << t+t_dfrag << "ms: Cannot place process " 
				<< p[next_p].name << " -- skipped!" << endl;
				for(int i=0;i<p[next_p].num;i++){
					if(p[next_p].arrival[i]==t){
						p[next_p].arrival[i]=-1;
						p[next_p].runtime[i]=-1;
						if(i==p[next_p].num-1){
							p[next_p].flag=true;
						}
						break;
					}
				}				
			}
			else{
				if(p[next_p].run){
					/*add successful */
					cout << "time " << t+t_dfrag << "ms: Placed process " 
					<< p[next_p].name << ":" << endl;
					table.print();
				}else{
					/*defragmentation needed*/
					cout << "time " << t+t_dfrag << "ms: Cannot place process " 
					<< p[next_p].name << " -- starting defragmentation" << endl;
					set<char> queue;
					int moved = table.defrag(queue);
					t_dfrag+=moved;
					cout << "time " << t+t_dfrag << "ms: Defragmentation complete " 
						<< "(moved " << moved << " frames: ";
					set<char>::iterator itr=queue.begin();
					cout << *itr;
					itr++;
					while(itr!=queue.end()){
						cout << ", " << *itr;
						itr++;
					}
					cout << ")" << endl;
					table.print();
					last_placed = 256 - table.get_free_bytes();
					table.next_add(p[next_p], last_placed);
					cout << "time " << t+t_dfrag << "ms: Placed process " 
					<< p[next_p].name << ":" << endl;
					table.print();				
			}
		}

	}
		else{
			/* remove next process */
			t=next_t;
			last_placed = table.worst_remove(p[next_p], last_placed);
			cout << "time " << t+t_dfrag << "ms: Process " 
			<< p[next_p].name << " removed:" << endl;
			p[next_p].run=false;
			table.print();
			/* if this is the last round of this process, flag up and done here */
			int n=p[next_p].num-1;
			if((p[next_p].arrival[n]+p[next_p].runtime[n]) == next_t){
			p[next_p].flag=true;
		}
	}
}
}

void best_fit(vector<process> p){ /* processes need to be sorted by name */
	cout << "time 0ms: Simulator started (Contiguous -- Best-Fit)" << endl;
	memory table;
	
	int t=0; /* keep track process time */
	int t_dfrag=0; /*keep track of defragmentation time */
	/* when output real time, output t+t_dfrag */
	while(1){
		/* end loop if all processes are done */
		if(isfinished(p)){
			cout << "time " << t+t_dfrag
			<< "ms: Simulator ended (Contiguous -- Best-Fit)" << endl;
			break;
		}
		int next_t = Inf; /* need a huge initial value */
		int next_p = 0;   
		bool isArrFst = false; // true for arrival, false for leave
		next_event(t,next_t,next_p,isArrFst,p); //find next event
		if(isArrFst){ // next event is arrival
			/* add the next process */
			t=next_t;
			cout << "time " << t+t_dfrag << "ms: Process " << p[next_p].name
			<< " arrived (requires " << p[next_p].mem << " frames)" << endl;
			bool success=table.best_add(p[next_p]);
			if(!success){
				/* skip this process */
				cout << "time " << t+t_dfrag << "ms: Cannot place process " 
				<< p[next_p].name << " -- skipped!" << endl;
				for(int i=0;i<p[next_p].num;i++){
					if(p[next_p].arrival[i]==t){
						p[next_p].arrival[i]=-1;
						p[next_p].runtime[i]=-1;
						if(i==p[next_p].num-1){
							p[next_p].flag=true;
						}
						break;
					}
				}
			}else{
				if(p[next_p].run){
					/*add successful */
					cout << "time " << t+t_dfrag << "ms: Placed process " 
					<< p[next_p].name << ":" << endl;
					table.print();
				}else{
					/*defragmentation needed*/
					cout << "time " << t+t_dfrag << "ms: Cannot place process " 
					<< p[next_p].name << " -- starting defragmentation" << endl;
					set<char> queue;
					int moved = table.defrag(queue);
					t_dfrag+=moved;
					cout << "time " << t+t_dfrag << "ms: Defragmentation complete " 
						<< "(moved " << moved << " frames: ";
					set<char>::iterator itr=queue.begin();
					cout << *itr;
					itr++;
					while(itr!=queue.end()){
						cout << ", " << *itr;
						itr++;
					}
					cout << ")" << endl;
					table.print();
					table.best_add(p[next_p]);
					cout << "time " << t+t_dfrag << "ms: Placed process " 
					<< p[next_p].name << ":" << endl;
					table.print();
				}
			}
		}else{ // next event is a process leaving
			/* remove next process */
			t=next_t;
			table.worst_remove(p[next_p]);
			cout << "time " << t+t_dfrag << "ms: Process " 
			<< p[next_p].name << " removed:" << endl;
			p[next_p].run=false;
			table.print();
			/* if this is the last round of this process, flag up and done here */
			int n=p[next_p].num-1;
			if((p[next_p].arrival[n]+p[next_p].runtime[n]) == next_t){
				p[next_p].flag=true;
			}
		}	
	}
}

void worst_fit(vector<process> p){ /* processes need to be sorted by name */
	cout << "time 0ms: Simulator started (Contiguous -- Worst-Fit)" << endl;
	memory table;
	
	int t=0; /* keep track process time */
	int t_dfrag=0; /*keep track of defragmentation time */
	/* when output real time, output t+t_dfrag */
	while(1){
		/* end loop if all processes are done */
		if(isfinished(p)){
			cout << "time " << t+t_dfrag
			<< "ms: Simulator ended (Contiguous -- Worst-Fit)" << endl;
			break;
		}
		int next_t = Inf; /* need a huge initial value */
		int next_p = 0;   
		bool isArrFst = false; // true for arrival, false for leave
		next_event(t,next_t,next_p,isArrFst,p); //find next event
		if(isArrFst){ // next event is arrival
			/* add the next process */
			t=next_t;
			cout << "time " << t+t_dfrag << "ms: Process " << p[next_p].name
			<< " arrived (requires " << p[next_p].mem << " frames)" << endl;
			bool success=table.worst_add(p[next_p]);
			if(!success){
				/* skip this process */
				cout << "time " << t+t_dfrag << "ms: Cannot place process " 
				<< p[next_p].name << " -- skipped!" << endl;
				for(int i=0;i<p[next_p].num;i++){
					if(p[next_p].arrival[i]==t){
						p[next_p].arrival[i]=-1;
						p[next_p].runtime[i]=-1;
						if(i==p[next_p].num-1){
							p[next_p].flag=true;
						}
						break;
					}
				}
			}else{
				if(p[next_p].run){
					/*add successful */
					cout << "time " << t+t_dfrag << "ms: Placed process " 
					<< p[next_p].name << ":" << endl;
					table.print();
				}else{
					/*defragmentation needed*/
					cout << "time " << t+t_dfrag << "ms: Cannot place process " 
					<< p[next_p].name << " -- starting defragmentation" << endl;
					set<char> queue;
					int moved = table.defrag(queue);
					t_dfrag+=moved;
					cout << "time " << t+t_dfrag << "ms: Defragmentation complete " 
						<< "(moved " << moved << " frames: ";
					set<char>::iterator itr=queue.begin();
					cout << *itr;
					itr++;
					while(itr!=queue.end()){
						cout << ", " << *itr;
						itr++;
					}
					cout << ")" << endl;
					table.print();
					table.worst_add(p[next_p]);
					cout << "time " << t+t_dfrag << "ms: Placed process " 
					<< p[next_p].name << ":" << endl;
					table.print();
				}
			}
		}else{ // next event is a process leaving
			/* remove next process */
			t=next_t;
			table.worst_remove(p[next_p]);
			cout << "time " << t+t_dfrag << "ms: Process " 
			<< p[next_p].name << " removed:" << endl;
			p[next_p].run=false;
			table.print();
			/* if this is the last round of this process, flag up and done here */
			int n=p[next_p].num-1;
			if((p[next_p].arrival[n]+p[next_p].runtime[n]) == next_t){
				p[next_p].flag=true;
			}
		}	
	}
}

void parse(ifstream& file, vector<process>& processes){
	int NumOfp;
	file >> NumOfp;
	for(int i=0;i<NumOfp;i++){
		process p;
		char c;
		int tmp;
		file >> c;
		p.name=c;
		p.flag=false;
		p.run=false;
		file >> tmp;
		p.mem=tmp;
		while(!file.eof()){
			file >> tmp;
			p.arrival.push_back(tmp);
			file >> c; // get rid of the '/'
			file >> tmp;
			p.runtime.push_back(tmp);
			file.get(c);
			if(c == '\n'){break;} // next process
		}
		p.num=p.arrival.size();
		processes.push_back(p);
	}
}

int main(int argc, char* argv[]){
	if(argc != 2){
		cerr << "ERROR: Input file is needed" << endl;
		return 1;
	}
	ifstream infile(argv[1]);
	if(!infile.good()){
		cerr << "ERROR: Can't open " << argv[1] << endl;
	}
	/* parse the input file */
	vector<process> processes;
	parse(infile,processes);
	sort(processes.begin(), processes.end());
	infile.close();
	/* start the algorithms here */
	
	next_fit(processes);
	cout << endl;
	
	best_fit(processes);
	cout << endl;
	worst_fit(processes);
	cout << endl;
	nonContiguous(processes);
	return 0;
}
