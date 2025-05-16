//하드웨어 포트에 데이터 넣기
#define outb(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))

//하드웨어 포트에서 데이터 읽어오기
#define inb(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx, %%al":"=a" (_v):"d"(port));\
_v;\
})
/*
두 번의 jmp 인스트럭션을 사용하는 이유는?
어차피 버스 트랜잭션이 완료될 때 까지 기다리므로(동기적)
jmp가 실행될 시점에는 이미 데이터는 받아와 진 상태이다.
x86에서 짧은 분기의 점프는 15~20 cycle 정도 걸린다.
레거시 isa 디바이스(구형 사운드,프린터,시리얼 칩)은 이 정도 짧은 공백이 있어야
내부 레지스터가 업데이트될 시간을 확보할 수 있다.
*/
#define outb_p(value,port) \
__asm__ ("outb %%al, %%dx \n" \
    "\tjmp 1f\n" \
    "1:\tjmp 1f\n" \
    "1:"::"a"(value),"d"(port))

#define inb_p(port) ({\
unsigned char _v\
__asm__ volatile ("inb %%dx,%%al\n" \
    "\tjmp 1f \n" \
    "1:\t jmp 1f\n" \
    "1:":"=a" (_v):"d"(port)); \
_v; \
})