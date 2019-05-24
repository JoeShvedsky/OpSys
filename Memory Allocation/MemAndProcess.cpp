/*Joseph Shvedsky - shvedj*/
  
#include "MemAndProcess.h"
#include <iostream>
#include <set>
#include <cstring>

using namespace std;

/* functions for class process */
void process::print()const{
	int s = this->arrival.size();
	int i;
	cout << this-> name << endl;
	cout << this-> mem << endl;
	cout << this-> num << endl;
	for(i=0;i<s;i++){
		cout << this->arrival[i] << "/" << runtime[i] << " ";
	}
	cout << endl;
}

bool process::operator<(const process&p) const{
    return name < p.name;
  }


/* functions for class memroy */
memory::memory(){
	for(int i=0;i<256;i++){
		mem[i]='.';
	}
	free_bytes=256;
}

void memory::print()const{
	cout << "================================" << endl;
	int i,j;
	for(i=0;i<8;i++){
		for(j=0;j<32;j++){
			cout << mem[i*32+j];
		}
		cout << endl;
	}
	cout << "================================" << endl;
}

bool memory::next_add(process &p, int& last_placed)
{
	
	if(free_bytes < p.mem){
		return false;
	}
	int i = last_placed;
	int count = 0;
	for(int j = last_placed; j < 256; ++j){
		if(mem[j] == '.'){
			count++;
			if(count == p.mem){
				int k;
				for(k = i; k < i + count; ++k){
					mem[k] = p.name;
				}
				last_placed = k; // next starting point
				free_bytes-=p.mem;
				p.run = true;
				return true;
			}
		}
		else{
			count = 0;
			i = j+1;
		}
	}
	//loop around 
	if(last_placed > 0){
		count = 0;
		i = 0;
		for(int j = 0; j < 256; ++j){
			if(mem[j] == '.'){
				count++;
				if(count == p.mem){
					int k;
					for(k = i; k < i + count; ++k){
						mem[k] = p.name;
					}
					last_placed = k;
					free_bytes-=p.mem;
					p.run = true;
					return true;
				}
			}
			else{
				count = 0;
				i = j+1;
			}		
		}
	}
	return true;
}

bool memory::worst_add(process &p){
	if(free_bytes<p.mem){
		return false;
	}
	/* find largest partition */
	int i = 0; /* starting point of largest partition */
	int bytes = 0; /* largest partition so far */
	int count = 0;
	for(int j=0;j<256;j++){
		if(mem[j]=='.'){
			count++;
			if(j==255 && bytes < count){
				bytes = count;
				i=j-count+1;
				count=0;
			}
		}
		else{
			if(bytes < count){
				bytes = count;
				i = j-count;
			}
			count = 0;
		}
	}
	if(p.mem<=bytes){
		free_bytes-=p.mem;
		for(int j=i;j<i+p.mem;j++){
			mem[j]=p.name;
		}
		p.run=true;
		return true;
	}else{
		/* deframentation */
		return true;
	}
	return false;
}

int memory::defrag(set<char> & queue){
	int move_bytes = 0;
	int count = 0;
	for(int i=0;i<256;i++){
		if(mem[i]=='.'){ /* count free bytes */
			count++;
		}else if(count!=0){ /* swap free bytes with in-used bytes */
			move_bytes+=count;
			int c=0;
			for(int j=i-count;j<i;j++){
				if(j+count<255 && mem[j+count]!='.'){
					queue.insert(mem[j+count]);
					mem[j]=mem[j+count];
					mem[j+count]='.';
					c++;  /* deal with cases that bytes need to be moved
						  is less than count, c++ and free_bytes--  */
				}else{ 
					move_bytes--;
				}
			}
			count-=c;
			count+=1;
	//		cout << move_bytes << " " << count << endl;
	//		print();
		}
	}
	return move_bytes;
}

int memory::worst_remove(process &p, int last_placed){
	int count = 0;
	for(int i = 0;i<256;i++){
		if(mem[i]==p.name){
			//if(count == 0  && last_placed == i+p.mem)
			//	last_placed = i;	//if removing last process
			mem[i]='.';
			count++;
		}
		if(count==p.mem){
			break;
		}
	}
	free_bytes+=p.mem;
	return last_placed;
}

bool memory::best_add(process &p){
	if(free_bytes<p.mem){
		return false;
	}
	/* find smallest partition */
	int i = 0; /* starting point of largest partition */
	int bytes = 257; /* largest partition so far */
	int count = 0;
	for(int j=0;j<256;j++){
		if(mem[j]=='.'){
			count++;
			if(j==255 && bytes > count && count >= p.mem){
				bytes = count;
				i=j-count+1;
				count=0;
			}
		}
		else{
			if(bytes > count && count >= p.mem){
				bytes = count;
				i = j-count;
			}
			count = 0;
		}
	}
	if(p.mem<=bytes && bytes !=257){
		free_bytes-=p.mem;
		for(int j=i;j<i+p.mem;j++){
			mem[j]=p.name;
		}
		p.run=true;
		return true;
	}else{
		/* deframentation */
		return true;
	}
	return false; // won't reach here.
}

void memory::nonc_add(process &p){
	int k = 0;
	if (free_bytes < p.mem){
		cout << "Cannot place process " << p.name << " -- skipped!" << endl;
	  	p.runtime.erase(p.runtime.begin());
  		p.arrival.erase(p.arrival.begin());	
	}
	else{
		cout << "Placed process " << p.name << ":" << endl;
		for (int j = 0; j < 256; j++){
		  	if (mem[j] == '.'){
		  		mem[j] = p.name;
		  		k++;
		  		free_bytes--;
		  	}
		  	if (k == p.mem){
		  		break;
		  	}
		}
		p.run = true;
		print();
	}
}

void memory::nonc_remove(process &p){
	cout << "Process " << p.name << " removed:" << endl;
	for (int j = 0; j < 256; j++){
	  	if (mem[j] == p.name){
	  		mem[j] = '.';
	  		free_bytes++;
	  	}
	}
  	p.run = false;
  	p.runtime.erase(p.runtime.begin());
  	p.arrival.erase(p.arrival.begin());
  	print();
}


