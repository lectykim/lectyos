#ifndef _SCHED_H
#define _SCHED_H

#define NR_TASKS 64
#define HZ 100

/*
 * x86 세그먼트 선택자 (selector)은 GDT/LDT 내 디스크립터 인덱스 x 8 값을 가지도록 설계되어 있다.
 * 즉, cpu n에 해당하는 tss 인덱스는, FIRST_TSS_ENTRY + 2*n이고,
 * 이 idx를 선택자로 만들기 위해서는 3비트를 밀어야 한다.
 * 2*n은 1비트를 더 미는 것으로 표현이 가능하므로, 해당 식이 나오게 된다.
 * */
#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))
#define ltr(n) __asm__("ltr %%ax::"a" (_TSS(n)))
#define lldt(n) __asm__("lldt %%ax"::"a" (_LDT(n)))


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
