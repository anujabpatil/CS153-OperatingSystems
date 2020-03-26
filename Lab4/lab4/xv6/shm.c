#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
  acquire(&(shm_table.lock));
  struct proc* currProc = myproc();

  //find the id in the shm_table
  int i;
  for(i = 0; i < 64; i++) {
    if(shm_table.shm_pages[i].id == id) {
      // Another process did shm_open before current process. So, get the physical page from shm_table
      // and map it to current process's page table using mappages().
      char* mem = shm_table.shm_pages[i].frame;
      uint va = PGROUNDUP(currProc->sz);
      mappages(currProc->pgdir, (void*)va, PGSIZE, V2P(mem), PTE_W|PTE_U);
      shm_table.shm_pages[i].refcnt++;
      *pointer = (char*) va;
      currProc->sz = PGROUNDUP(currProc->sz) + PGSIZE; //update size of process' virtual address space

      release(&(shm_table.lock));
      return 0;
    }
  }
  
  // Did not find page with id, so current process is the first one to shm_open for this id.
  // We are assuming that 1 <= id <= 64 because there are 64 pages in physical memory.
  shm_table.shm_pages[id-1].id = id;
  shm_table.shm_pages[id-1].frame = kalloc(); //allocating physical page for new kernal memory
  memset(shm_table.shm_pages[id-1].frame, 0, PGSIZE);

  //Rest is same like the condition "if that id is found in the shm_table".
  char* mem = shm_table.shm_pages[id-1].frame;
  uint va = PGROUNDUP(currProc->sz);
  mappages(currProc->pgdir, (void*)va, PGSIZE, V2P(mem), PTE_W|PTE_U);
  shm_table.shm_pages[id-1].refcnt = 1;
  *pointer = (char*) va;
  currProc->sz = PGROUNDUP(currProc->sz) + PGSIZE; //update size of process' virtual address space

  release(&(shm_table.lock));
  return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
  acquire(&(shm_table.lock));
  
  // We know that physical memory page corresponding to this id can be found
  // at position (id-1) in the shm_pages array. Hence, we now check its 
  // refcnt. Decrement it and then check if it is greater than 0. If yes, then
  // no worries - leave as it is. If refcnt is equal to 0, then we need to clear
  // the shm_table. According to the lab4 problem statement, we do not need 
  // to free up the page since it is still mapped in the page table. 
  // It is okay to leave it that way.
  int i = id - 1;
  shm_table.shm_pages[i].refcnt--;
  if(shm_table.shm_pages[i].refcnt == 0) { //Reset this page
    shm_table.shm_pages[i].id = 0;
    shm_table.shm_pages[i].frame = 0;
  }

  release(&(shm_table.lock));
  return 0; //added to remove compiler warning -- you should decide what to return
}
