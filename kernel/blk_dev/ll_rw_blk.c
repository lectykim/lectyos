#include "blk.h"

struct request request[NR_REQUEST];
struct task_struct * wait_for_request = NULL;
struct blk_dev_struct blk_dev[NR_BLK_DEV] = {
    {NULL,NULL},
    {NULL,NULL},
    {NULL,NULL},
    {NULL,NULL},
    {NULL,NULL},
    {NULL,NULL},
    {NULL,NULL}
}
/*
 * 블럭 디바이스(hdd,floppy) 등을 연결하는 리퀘스트 객체를 초기화한다.
 * */
void blk_dev_init(void)
{
    int i;
    for(i=0;i<NR_REQUEST;i++){
        request[i].dev = -1;
        request[i].next = NULL;
    }
}
