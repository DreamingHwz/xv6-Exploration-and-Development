#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"
#include <stddef.h>
#include "spinlock.h"
#include "vm.c"

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

int
settickets(int number){
  if (number != 1 && number != 0){
    return -1;
  }
  else {
    myproc()->tickets = number;
    return 0;   
  }
}

int
getpinfo(struct pstat * p){
  if (p == NULL) return -1;
  //struct pstat * ps = getpstat();
  acquire(&ptable.lock);
  int i = 0;
  struct proc * pr;
  for(pr = ptable.proc; pr < &ptable.proc[NPROC]; pr++){
    if(pr->state == UNUSED)
      continue;
    p->inuse[i] = 1;
    p->tickets[i] = pr->tickets;
    p->pid[i] = pr->pid;
    p->ticks[i] = pr->ticks;
    i++;  
  }
  release(&ptable.lock);
  return 0;
}

int 
mprotect(void *addr, int len){
  acquire(&ptable.lock);
  pde_t * pgdir = myproc()->pgdir;
  if((uint) addr % PGSIZE != 0 || len <= 0)
    release(&ptable.lock);
    return -1;
  pte_t *pte;
  
  void * a = addr;
  for(int i = 0; i < len; i++){
    if((pte = walkpgdir(pgdir, a + i*PGSIZE, 0)) == 0)
      release(&ptable.lock);
      return -1;
    *pte = (*pte) & 0xffd;
  }
  lcr3(V2P(pgdir));
  release(&ptable.lock);
  return 0;  
}

int
munprotect(void *addr, int len){
  return 0;
}

int
sys_mprotect(void){
  struct void * addr;
  int len;
  if(argptr(0, (void*)&addr, sizeof(*addr)) < 0 || argint(1, &len) < 0)
    return -1;
  return mprotect(addr, int);
}

int
sys_munprotect(void){
  return 0;
}

int
sys_settickets(void){
  int n;
  if(argint(0, &n) < 0)
    return -1;
  return settickets(n);
}

int
sys_getpinfo(void){
  struct pstat * ps;
  if(argptr(0, (void*)&ps, sizeof(*ps)) < 0)
    return -1;
  return getpinfo(ps);
}

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

int
sys_wait(void)
{
  return wait();
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
