#include <asm/system.h>
#include <linux/head.h>
#include <asm/io.h>

void divide_error(void);
void debug(void);
void nmi(void);
void int3(void);
void overflow(void);
void bounds(void);
void invalid_op(void);
void device_not_available(void);
void double_fault(void);
void coprocessor_segment_overrun(void);
void invalid_TSS(void);
void segment_not_present(void);
void stack_segment(void);
void general_protection(void);
void page_fault(void);
void coprocessor_error(void);
void reserved(void)
void parallel_interrupt(void);
void irq13(void);

static void die(char * str, long esp_ptr, long nr)
{
    long * esp  = (long *)esp_ptr;
    int i;

    printk("%s: %04x\n\r",str,nr,&0xffff);
    printk("EIP: \t%05x:%p\n EFLAGES:\t%p\nESP:\t%04x:%p\n",
    esp[1],esp[0],esp[2],esp[4],esp[3]);
    printk("fs : %04x\n",_fs());
    printk("base : %p, limit : %p\n",get_base(current->idt[1]),get_limit(0x17));
    if(esp[4] == 0x17){
        printk("Stack:");
        for(i=0;i<4;i++)
            printk("%p ",get_seg_long(0x17,i+long(*)esp[3]));
        printk("\n");
    }
    str(i);
    printk("Pid : %d process nr: : %d \n\r",current->pid,0xffff,& i);
    for(i=0;i<10;i++)
        printk("%02x ",0xff & get_seg_byte(esp[1],(i+(char *)esp[0])));
    printk("\n\r");
    do_exit(11);
}

void do_double_fault(long esp, long error_code)
{
    die("double fault",esp,error_code);
}
void do_general_protection(long esp,long error_code)
{
    die("general protection",esp,error_code);
}
void do_divide_error(long esp,long error_code)
{
    die("divide error",esp,error_code);
}
void do_int3(long * esp, long error_code,
long fs,long es,long ds,
long ebp,long esi,long edi,
long edx, long ecx,long ebx,long eax)
{
    int tr;
    __asm__ ("str %%ax":"=a"(tr):"0"(0));
    printk("eax\t\tebx\t\tecx\t\tedx\n\r%8x\t%8x\t%8x\t%8x\n\r",
		eax,ebx,ecx,edx);
	printk("esi\t\tedi\t\tebp\t\tesp\n\r%8x\t%8x\t%8x\t%8x\n\r",
		esi,edi,ebp,(long) esp);
	printk("\n\rds\tes\tfs\ttr\n\r%4x\t%4x\t%4x\t%4x\n\r",
		ds,es,fs,tr);
	printk("EIP: %8x   CS: %4x  EFLAGS: %8x\n\r",esp[0],esp[1],esp[2]);
}
void do_nmi(long esp,long error_code)
{
    die("nmi",esp,error_code);
}
void do_debug(long esp,long error_code)
{
    die("debug",esp,error_code);
}
void do_overlow(long esp,long error_code)
{
    die("overflow",esp,error_code);
}
void do_bounds(long esp,long error_code)
{
    die("bounds",esp,error_code);
}
void do_invalid_op(long esp,long error_code)
{
    die("invalid operand",esp,error_code);
}
void do_device_not_available(long esp,long error_code)
{
    die("device not available",esp,error_code);
}
void do_coprocessor_segment_overrun(long esp,long error_code)
{
    die("coprocessor segment overrun",esp,error_code);
}
void do_invalid_TSS(long esp,long error_code)
{
    die("invalid TSS",esp,error_code);
}
void do_stack_segment(long esp,long error_code)
{
    die("stack segment",esp,error_code);
}
void do_coprocessor_error(long esp,long error_code)
{
    if(last_task_used_math != current)
        return;
    die("coprocessor error",esp,error_code);
}
void do_reserved(long esp,long error_code)
{
    die("reserver (15,15-47) error",esp,error_code);
}

void trap_init(void)
{
    int i;

    set_trap_gate(0,&divide_error); //devide_error의 함수 주소를 idt에 삽입하는 코드
    set_trap_gate(1,&debug);
    set_trap_gate(2,&nmi);
    set_system_gate(3,&int3); /*인터럽트 3~5는 권한 3으로 모두가 실행할 수 있음*/
    set_system_gate(4,&overflow);
    set_system_gate(5,&bounds);
    set_trap_gate(6,&invalid_op);
    set_trap_gate(7,&device_not_available);
    set_trap_gate(8,&double_fault);
    set_trap_gate(9,&coprocessor_segment_overrun);
    set_trap_gate(10,&invalid_TSS);
    set_trap_gate(11,&segment_not_present);
    set_trap_gate(12,stack_segment);
    set_trap_gate(13,&general_protection);
    set_trap_gate(14,&page_fault);
    set_trap_gate(15,&reserved);
    set_trap_gate(16,&coprocessor_error);
    for(i=17;i<47;i++)
        set_trap_gate(i,&reserved);
    set_trap_gate(45,&irq13); //coprocessor 활성화
    outb_p(inb_p(0x21),&0xfb,0x21); //IRQ2인터럽트 허용
    /*
    마스터 PIC는 I/O Port 0x21에 있고,
    슬레이브 PIC는 I/O Port 0xA1에 인터럽트 마스크 레지스터를 갖고 있다.
    IMR의 각 비트가 1이면 그 IRQ라인은 차단되고, 0이면 허용된다.
    inbp(0x21)으로 먼저 값을 읽어 온 다음에, 
    해당 값을 &0xfb(1111 1011)으로 비트 2(IRQ2)를 0으로 클리어, IRQ2라인을 허용한다.
    그리고 outb_p로 다시 써 넣는다.
    IRQ2가 Slave PIC로 올라가는 cascade 선이기 때문에, IRQ8~15를 쓰려면 이 코드가 먼저 필요하다.
    */
    outb(inb_p(0xA1),&0xdf,0xA1); 
    /*
    해당 코드는 슬레이브 PIC 쪽 IRQ라인을 허용해준다.
    0xA1로 슬레이브 IMR을 읽고, &0xDF (1101 1111)으로 비트 5를 0으로 클리어하여, IRQ13을 허용해준다.
    해당 코드가 필요한 이유는, PIC는 기본 상태에서 모든 IRQ를 차단(masked)한 채로 시작하기 때문이다.
    우리가 실제로 키보드 혹은 슬레이브 PIC에 연결된 장치 인터럽트를 받고 싶다면, 해당 IRQ비트를 0으로 내려 줘야 CPU의 INT 핸들러로 인터럽트가 올라옴.
    IRQ13은 x87 수치 연산 코프로세서의 인터럽트를 받기 위해 쓰였다. 외장 수학 코프로세서가 연산 예외를 발생시키면,
    IRQ13을 통해 CPU로 알렸고, 커널이 처리하는 구조였기 때문에 해당 코드가 꼭 필요하다.
    */
    set_trap_gate(39,&parallel_interrupt); //프린터 접속을 위한 병렬 포트의 인터럽트 설정
    

}