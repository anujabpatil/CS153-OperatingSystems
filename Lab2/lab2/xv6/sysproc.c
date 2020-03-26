#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

// Changed for assignment1
int
sys_wait(void)
{
  int* status;
  argptr(0, (void*)&status, sizeof(status));
  return wait(status);
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Added for assignment1
// This method is a handler for our created exit(). This reads exit status
// from the user provided command-line argument and then calls our created
// exit() which takes that argument as its parameter.
int
sys_exitUDef(void)
{
  int exitStatus;
  if(argint(0, &exitStatus) < 0) return -1;
  return exitUDef(exitStatus);
}

// Added for assignment 1
// This method is added for assignment1. It waits for a process whose pid is
// passed as an argument to this method. The process for which current process
// waits need not be a child of current process.
// For bonus part, we are checking if options argument passed is valid or not.
// If not valid, then return -1.
// If no options is passed in argument, then just pass options = 0 (default)
// in waitpid.
int
sys_waitpid(void)
{
  int pid, options = 0;
  int* status;
  if(argint(0, &pid) < 0) return -1;
  if(argptr(1, (void*)&status, sizeof(status)) < 0) return -1;
  if(argint(2, &options) < 0) return -1;
  return waitpid(pid, status, options);
}

// Added for assignment 2
// This method changes the priority of a process.
int
sys_setPriority(void)
{
  int priority;
  if(argint(0, &priority) < 0)
    return -1;
  return setPriority(priority);
}

// Added for assignment 2
// This method returns the current priority of a process.
int sys_getPriority(void)
{
  return getPriority();
}
