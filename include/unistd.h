#define __NR_SETUP 0
#define __NR_EXIT 1
#define __NR_FORK 2

#define __syscall0(type,name) \
type name(void) \
{\
long __res;\
__asm__ volatile ("int $0x80" \ //int 0x80 = 모든 시스템 콜의 진입점이다.
:"=a"(__res)\ //fork() 함수도 이 중 하나이다.
:"0" (__NR_##name));\ //__res에 eax의 값이 저장된다.
                      //입력부의 "0"은 eax를 의미하고, NR_FORK는 2다.
                      //즉, eax에 2가 대입된다.
                      //이 부분은 어셈블리 코드가 실행된 이후에 실행된다.
if(__res>=0)\
return (type)__res;\
errno= -__res;\
return -1;\
}
//주의  - int 0x80이 실행되면 하드웨어가 자동으로 스택에 ss,esp,eflags,cs,eip가 들어간다.
int fork(void);
int pause(void);
int sync(void);