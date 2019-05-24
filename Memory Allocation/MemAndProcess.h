/*Joseph Shvedsky - shvedj*/
  
#ifndef MemAndProcess_h_
#define MemAndProcess_h_
#include <vector>
#include <set>
#include <cstring>

class process; // forward declaration

class memory{
public:
  memory();
  /* functions 
  defragementation
  add process
  get rid of a process */
  
  /* for different algorithms */
  /* worst-fit */
  bool worst_add(process &p);
  int worst_remove(process &p, int = 0);
  
  /* best-fit */  //similar to worst-fit
  bool best_add(process &p);
  
  /*next-fit*/
  bool next_add(process &p, int&);
  
  /* noncontiguous */
  void nonc_add(process &p);
  void nonc_remove(process &p);
  
  /* helper functions */
  int defrag(std::set<char> &queue); /*queue stores the process that is moved */
  void print()const;
  
  const int& get_free_bytes() {return free_bytes;}
  
private:
  char mem[256];
  int free_bytes;
  
};

class process{
public:
  // helper function
  void print()const;
  bool operator<(const process&p)const;
  
  // member variable
  char name;
  int num;
  int mem;
  bool flag; /* is it done? */
  bool run; /* is it running? */
  
  std::vector<int> arrival;
  std::vector<int> runtime;
};

#endif
