#include "blk.h"

#define MAJOR_NR 1
char *rd_start;
int rd_length=0;

void do_rd_request(void)
{
    int len;
    char* addr;
}

/*
간단한 코드.
램디스크 메모리의 영역을 1바이트씩, 0으로 만드는 과정이다.
이 함수가 실행될 시점에는 램디스크는 아직 초기화되지 않았음.
*/
long rd_init(long mem_start,int length)
[
    int i;
    char * cp;
    blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
    rd_start = (char*) mem_start;
    rd_length = length;
    cp = rd_start;
    for(i=0;i<length;i++)
        *cp++='\0';
    retrun (length);

]