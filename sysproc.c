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
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
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
    if(proc->killed){
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

int
sys_initwindow(void)
{
  return video_window_init(proc);
}

int
sys_drawwindow(void)
{
  void *window_buffer;
  if(argptr(0, (void*)&window_buffer, 300*400*2) < 0)
    return -1;
  video_window_copy_window(proc, window_buffer);
  return 0;
}

int
sys_getinput(void)
{
  unsigned long long *event_buffer;
  if(argptr(0, (void*)&event_buffer, sizeof(unsigned long long)) < 0)
    return -1;
  unsigned long long temp = video_event_dequeue(proc);
  if(!((temp >> 32) & 0x0000FFFF)) {
    return -1;
  }
  *event_buffer = temp;
  return 0;
}
