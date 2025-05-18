#ifndef _SCHED_H
#define _SCHED_H

#define NR_TASKS 64
#define HZ 100

extern void trap_init(void);
struct tss_struct{
    long back_link;
    long esp0;
    long esp1;
    long ss1;
    long esp2;
    long ss2;
    long cr3;
    long eip;
    long eflags;
    long eax,ecx,edx,ebx;
    long esp;
    long ebp;
    long esi;
    long edi;
    long es;
    long cs;
    long ss;
    long ds;
    long fs;
    long gs;
    long ldt;
    long trace_bitmap;
    struct i387_struct i387;
};
struct task_struct{
    long state;
    long counter;
    long priority;
    long signal;
    struct sigcation sigcation[32];
    long blocked;
    int exit_code;
    unsigned long start_code,end_code,end_data,brk,start_stack;
    long pid,father,prgp,session,leader;
    unsigned short uid,euid,suid;
    unsigned short gid,egid,sgid;
    long alarm;
    long utime,stime,cutime,cstime,start_time;
    unsigned short used_math;
    int tty;
    unsigned short umask;
    struct m_inode * pwd;
    struct m_inode * root;
    struct m_inode * executable;
    unsigned long close_on_exec;
    struct file* flip[NR_OPEN];
    struct desc_struct idt[3];
    struct tss_struct tss;
}

#define INIT_TASK \
{0,15,15 \
0,{{},},0,\
0,0,0,0,0,0,\
0,-1,0,0,0\
0,0,0,0,0,0,\
0,0,0,0,0,0,\
0,\
-1,0022,NULL,NULL,NULL,0,\
{NULL,},\
{\
    {0,0,},\
    {0x9f,0xc0fa00},\
},\
    {0,PAGE_SIZE+(long)&init_task,0x10,0,0,0,0,(long)&pg_dir,\
    0,0,0,0,0,0,0,0,\
    0,0,0x17,0x17,0x17,0x17,0x17,0x17\
    _LDT(0),0x80000000,\
        {}\
    },\
}