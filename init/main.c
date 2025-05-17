#define __LIBRARY__

#include <linux/sched.h>
#include <asm/io.h>
#include <time.h>

#define DRIVE_INFO (*(struct drive_info *)0x90080)
#define ORIG_ROOT_DEV (*(unsigned short *)0x901FC)
#define EXE_MEM_K (*(unsigned short *)0x90002)
struct drive_info{char dummy[32];} drive_info;

long user_stack [4096>>2];

extern void mem_init(long start,long end);

struct{
    long *a;
    short b;
} stack_start = {&user_stack[4096>>2],0x10};

extern int vsprintf();
extern void init(void);
extern void blk_dev_init(void);
extern void chr_dev_init(void);
extern void hd_init(void);
extern void floppy_init(void);
extern void mem_init(long start,long end);
extern long rd_init(long mem_start,int length);
extern long kernel_mktime(struct tm* tm);
extern long startup_time;
static void time_init(void)
/*
0x80은 0x1000 0000이다. 
이를 설정해두는 이유는,  NMI를 비활성화 하기 위함이다.
sti(), cli()와 같은 명령으로 비활성화되지 않으며, nmi비트를 비활성화
하는것과 같이 특별한 방법으로 비활성화가 가능하다.
원래는 긴급한 오류 처리용이기 때문.
*/
#define CMOS_READ(addr) ({\
    outb_p(0x80|addr,0x70);\
    inb_p(0x71); \
})
/*
BCD (Binary-Code-Decimal)이란 십진수(Decimal) 한 자릿수를 4비트 이진수로 표현한 것.
보통 12를 표현하려면, 0x0c의 형태로 표현이 가능하나, BCD 표기법으로는 0x12, 0001 0010으로 표현된다.
val&0x0f는 하위 비트를 가져오는 것,
val>>4는 상위 비트를 가져오는 것이므로,
두 개를 더하면 BCD 형식의 수를 하나의 십진수로 변환할 수 있다.
예를 들어 bcd가 0x45라면,
하위 비트인 5를 가져와서,
0x05를 만들고,
0x40>>4를 적용하여 0x04를 만들고,10을 곱하면 0x28, 두 개를 더하면 0x2d로 
45를 만들 수 있다.
*/
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

{
    struct tm time;

    do{
        time.tm_sec = CMOS_READ(0); //초 = 0x00
        time.tm_min=CMOS_READ(2); //분 = 0x02
        time.tm_hour=CMOS_READ(4); //시간 = 0x04
        time.tm_mday=CMOS_READ(7); //day = 0x07
        time.tm_mon=CMOS_READ(8); //month = 0x08
        time.tm_year=CMOS_READ(9); // year = 0x09
    }while(time.tm_sec != CMOS_READ(0)); //원자 시계가 가리키는 값이 다를 때 종료

    BCD_TO_BIN(time.tm_sec);
    BCD_TO_BIN(time.tm_min);
    BCD_TO_BIN(time.tm_hour);
    BCD_TO_BIN(time.tm_mday);
    BCD_TO_BIN(time.tm_mon);
    BCD_TO_BIN(time.tm_year);
    time.tm_mon--;
    //1970년부터 얼마나 지났는지를 계산하는 함수
    startup_time = kernel_mktime(&time);


}
void main(void)
{
    //ROOT_DEV = makefile에 정의되어 있음.
    //이후 상수들은 setup.s에서 코드 데이터에 저장해둔 값을 읽어오는 것.
    ROOT_DEV=ORIG_ROOT_DEV;
    drive_info=DRIVE_INFO;
    memory_end = (1<<20) + (EXE_MEM_K << 10);
    memory_end &= 0xfffff000;
    if(memory_end > 16*1024*1024)
        memory_end = 16*1024*1024;
    if(memory_end > 12*1024*1024)
        buffer_memory_end = 4*1024*1024
    else if(memory_end > 6*1024*1024)
        buffer_memory_end = 2*1024*1024;
    else
        buffer_memory_end = 1*1024*1024;
    main_memory_start = buffer_memory_end;
#ifdef RAMDISK
        main_memory_start += rd_init(main_memory_start,RAMDISK*1024);
#endif
    mem_init(main_memory_start,memory_end);
    trap_init();
    blk_dev_init();
    tty_init();
    time_init();
    sched_init();
    buffer_init(buffer_memory_end);
    hd_init();
    floppy_init();
    sti();
    move_to_user_mode();
    if(!fork()){
        init();
    }



    for(;;) pause();

}

void init(void)
{
    int pid,i;

    setup((void*)&drive_info);
    (void) open("/dev/tty0",O_RDWR,0);
    (void) dup(0);
    (void) dup(0);
    printf("%d buffers = %d bytes buffer space \n\r" , NR_BUFFERS,NR_BUFFERS+BLOCK_SIZE);
    printf("Free mem: %d bytes \n\r",memory_end-main_memory_start);
    int (!(pid=fork())){
        close(0);
    }
    
}