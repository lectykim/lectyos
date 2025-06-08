long last_pid=0;

int find_empty_process(void)
{
    int i;

repeat:
    if((++last_pid)<0)
        last_pid=1;

    for(i=0;i<NR_TASKS;i++)
        if(task[i] && task[i]->pid == last_pid) goto repeat;
    for(i=1;i<NR_TASKS;i++)
        if(!task[i])
            return i;
    return -EAGAIN;
}
/*
이 코드는 새로 만든 자식 프로세스의 CPU 레지스터 상태(컨텍스트)를 그대로 저장하기 위한 코드이다.
x86의 TSS를 이용하면, ltr로 로드된 TSS 디스크립터를 바꾸기만 해도,
CPU가 자동으로 EIP,EFLAGS,일반 레지스터, 세그먼트 레지스터 등 모든 컨텍스트를 TSS에서 꺼내 온다.
*p = *current로 부모 프로세스의 tss 구조체를 통째로 복사한 뒤,
다음과 같이 레지스터를 모두 설정해준다.

특히 p->tss.eax = 0;으로 자식 쪽 반환값을 0으로 바꾸고,
부모는 last_pid를 리턴하도록 해서 fork() 시 부모/자식이 서로 다른 값을 받게 된다.


스케줄러 진입 시점에서, 자식 프로세서가 스케줄되어야 할때 커널은,
ltr <child tss selector>
jmp %TSS
와 같은 방식으로 하드웨어 태스크 스위치를 실행한다.
그러면 CPU가 TSS에 저장된
tss.eip -> 새 실행 위치
tss.esp/tss.ebp -> 사용자 스택 포인터
tss.ss/tss.ds 등의 세그먼트 레지스터
일반 레지스터(ecx,edx,ebx,esi,edi)
tss.eflags 등을 복원해주고, 바로 사용자 모드로 복귀시켜 준다.

이 코드는 새 프로세스가 부모와 동일한 실행 시점/스택/레지스터/ 상태로 시작하되
fork() 반환값만 바뀐 채로 '즉시' 실행될 수 있도록,
하드웨어 태스크 스위칭용 TSS에 완전한 cpu 컨텍스트를 미리 채워 놓는 작업이다.

프로세스 0은 '순수하게 커널 스케줄러가 쉴 때' 돌아가는 idle loop용이고,
사용자 모드로 전이하거나, 파일 시스템을 마운트하거나, 다른 유저 프로세스를 fork/exec하는 능력이 없다.
그래서 커널은 곧장 프로세스 1을 만들어, boot/idle 컨텍스트를 복사한뒤 레지스터만 바꿔서 최초의 사용자 공간을 실행하게 한다.
프로세스 1이 없었다면, 커널은 여전히 idle만 돌고,
사용자 모드를 진입할 주체가 없어 부팅이 멈추거나 커널 패닉이 나게 된다.
*/
int copy_process(int nr,long ebp, long edi, long esi, long gs,long none,
long ebs,long ecx, long edx,
long fs,long es,long ds,
long eip,long cs,long eflags, long esp,long ss)
{
    struct task_struct *p;
    int i;
    struct file *f;

    p=(struct task_struct *)get_free_page();
    if(!p)
        return -EAGAIN;
    task[nr] =p;

    *p = *current;
    p->state=TASK_UNINTERRUPTABLE; //준비 상태(ready)인 프로세스만이 깨어날 수 있고, 깨어날 수 있는 다른 방법은 없다.

    p->pid = last_pid;
    p->father = current->pid;
    p->counter = p->priority;
    p->signal=0;
    p->alarm=0;
    p->leader=0;
    p->utime= p->stime=0;
    p->cutime = p->cstime = 0;
    p->start_time = jiffies;
    p->tss.back_link=0;
    p->tss.esp0 =PAGE_SIZe + (long) p;
    p->tss.ss0 = 0x10;
    p->tss.eip = eip;
    p->tss.eflags = eflags;
    p->tss.eax = 0;
    p->tss.ecx = ecx;
    p->tss.edx = edx;
    p->tss.ebx=ebx;
    p->tss.esp=esp;
    p->tss.ebp=ebp;
    p->tss.esi=esi;
    p->tss.edi=edi;
    p->tss.es=es&0xffff;
    p->tss.cs=cs&0xffff;
    p->tss.ds = ds&0xffff;
    p->tss.ss = ss&0xffff;
    p->tss.fs=fs&0xffff;
    p->tss.gs=gs&0xffff;
    p->tss.ldt = _LDT(nr);
    p->tss.trace_bitmap = 0x80000000;
    if(last_task_used_math == current)
        __asm__("clts;fnsave%0"::"m" (p->tss.i387));
    if(copy_mem(nr,p)){
        task[nr] = NULL;
        free_page((long)p);
        return -EAGAIN;
    }
    for(i=0;i<NR_OPEN;i++)
        if(f=p->flip[i])
            f->f_count++;
    if(current->pwd)
        current->pwd->i_count++;
    if(current->root)
        current->root->i_count++;
    if(current->executable)
        current->executable->i_count++;
    set_tss_desc(gdt+(nr<<1)+FIRST_TSS_ENTRY,&(p->tss));
    set_ldt_desc(gdt+(nr<<1)+FIRST_LDT_ENTRY,&(p->ldt));
    p->state = TASK_RUNNING;
    return last_pid;


}
/*
자식 프로세스의 CS/DS를 설정한다.
자식 프로세스의 첫 페이지 테이블을 만들어서 복사한다.
*/
int copy_mem(int nr,struct task_struct * p)
{
    unsigned long old_data_base,new_data_base,data_limit;
    unsigned long old_code_base,new_code_base,code_limit;

    code_limit = get_limit(0x0f); // 0x0f=1111: CS, LDT, 권한 레벨 3
    data_limit = get_limit(0x17); //0x17 = 10111: DS,LDT,권한 레벨 3

    //부모 프로세스(프로세스0)의 cs,ds를 구한다.
    old_code_base = get_base(current->idt[1]);
    old_data_base = get_base(current->idt[2]);
    if(old_data_base != old_code_base)   
        panic("We dont support separed I&D");
    if(data_limit<code_limit)
        panic("bad data limit");
    new_data_base=new_code_base = nr*0x4000000; //nr=1, 0x4000000=64MB
    p->start_code = new_code_base;
    set_base(p->idt[1],new_code_base);//자식 코드 cs 베이스 어드레스 설정
    set_base(p->idt[2],new_data_base);//자식 코드 ds 베이스 어드레스 설정
    if(copy_page_tables(old_data_base,new_data_base,data_limit)){
        free_page_tables(new_data_base,data_limit);
        return -ENOMEM;
    }
    return 0;
}