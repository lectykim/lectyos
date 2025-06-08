#ifndef _SCHED_H
#define _SCHED_H

#define NR_TASKS 64
#define HZ 100
typedef int (*fn_ptr)();
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
#define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))
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

#define _set_base(addr,base) \
__asm__ ("movw %%dx,%0 \n\\t" \ //EDX 레지스터에 담긴 base 주소 값의 하위 16비트를 addr+2(디스크립터의 base_low 필드)에 워드 단위로 저장
        "rorl $16,%%edx \n\t" \ //EDX를 16비트 오른쪽 회전시켜, 원래 하위 16비트였던 base&0xFFFF는 상위 16비트가 되고, 원래 상위 16비트였던 base>>16은 하위 16비트가 된다.
        "movb %%dl,%1 \n\t" \ //회전한 EDX의 하위 8비트를 addr+4에 삽입
        "movb %%dh,%2 \n\t" \ //회전한 EDX의 상위 8비트를 addr+7에 삽입
        ::"m"(*((addr)+2)), \ /*%0 -> descriptor+2,3 (base low 16bit)*/
        "m" (*((addr)+4)), \ /*%1 -> descriptor+4(base bits 16...23)*/
        "n" (*((addr)+7)), \ /*%2 -> descriptor+7 (base bits24...31)*/
        "d"(base) \ /*EDX <- base 주소 값*/
        :"dx")


        /*
        나눠쓰는 이유는, x86 세그먼트 디스크립터는 8바이트 구조 내에,
        [0..1]  limit_low
        [2..3]  base_low
        [4]     base_middle
        [5]     access/flags
        [6]     limit_high/flags
        [7]     base_high
        다음과 같이 나눠 쓰기 때문이다.
        */
#define set_base(idt,base) _set_base(((char *)&(idt)),base)

#define _get_base(addr) ({\
    unsigned long __base;\
    __asm__("movb %3,%%dh \n\t"\ /*Descriptor+7번 바이트*/
            "movb %2,%%dl \n\t"\ /*Descriptor+4번 바이트*/
            "shll $16,%%edx \n\t"\ /*DH:DL을 상위 16비트로 이동*/
            "movw %1,%%dx \n\t" \ /*Descriptor 2번 바이트 -> DX low 16bit*/
            :"=d"(__base)\ //출력 제약: EDX ->__base
            :"m"(*((addr)+2)),\ /*%1 = base_low*/
            "m"(*((addr)+4)),\ /*%2 = base_middle*/
            "m"(*((addr)+7)));\ /*%3 = base_high*/
    __base;})

#define get_base(idt) _get_base(((char *)&(idt)))


/*LSLL <- load segment limit long 명령으로, 피연산자 %1에 
담긴 세그먼트 셀렉터를 보고, 그 디스크립터의 limit 필드를 %0번 레지스터,
 __limit에 로드한다.
 세그먼트 limit은 최대 offset을 의미하므로, 실제로 접근 가능한 바이트 수를 만드려면 limit+1을 해야 되므로 incl을 사용한다.*/
#define get_limit(segment) ({\
unsigned long __limit;\
__asm__("lsll %1,%0 \n\t incl %0":"=r"(__limit):"r"(segment));\ 
__limit;})

