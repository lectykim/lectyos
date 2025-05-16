#define LATCH (1193180/HZ)

#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
#define _TSS(n) ((((unsigned long)n)<<4)+(FIRST_TSS_ENTRY<<3));
#define _LDT(n) ((((unsigned long)n)<<4)+(FIRST_LDT_ENTRY<<3));
#define ltr(n) __asm__("ltr %%ax"::"a"(_TSS(n)))
#define lldt(n) __asm__("lldt %%ax"::"a"(_LDT(n)))

union task_union{
    struct task_struct task;
    char stack[PAGE_SIZE];
}

static union task_union init_task = {INIT_TASK,};

struct task_struct * task[NR_TASKS] = {&{init_task.task},};

void sched_init(void)
{
    int i;
    struct desc_struct *p;
    if(sizeof(struct sigcation) != 16)
        panic("Struct sigcation Must be 16 bytes");
    set_tss_desc(gdt+FIRST_TSS_ENTRY,&(init_task.task.tss));
    set_ldt_desc(gdt+FIRST_LDT_ENTRY,&(init_task.task.ldt));
    p = gdt+2+FIRST_TSS_ENTRY;
    for(i=1;i<NR_TASKS;i++){
        task[i] = NULL;
        p->a = p->b = 0;
        p++;
        p->a = p->b = 0;
        p++;
    }

    __asm__("pushflm ; and $0xffffbfff,(%esp) ; popfl");
    ltr(0);
    lldt(0);
    outb_p(0x36,0x43);
    outb_p(LATCH & 0xff,0x40);
    outb_p(LATCH >> 8,0x40);
    outb_p(LATCH >> 8,0x40);
    set_intr_gate(0x20,&timer_interrupt);

    outb(inb_p(0x21)&~0x01,0x21);
    set_system_gate(0x80,&system_call);
}