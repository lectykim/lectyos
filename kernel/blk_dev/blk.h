#define NR_REQUEST 32
#define NR_BLK_DEV 7
#define DEVICE_REQUEST do_rd_request;
static void (DEVICE_REQUEST)(void);
/*
리눅스 커널의 블록 디바이스(저장 공간을 블록으로 나누는 디바이스, 하드디스크,플로피디스크) 구조이다.
*/
struct request{
    int dev; //리퀘스트가 없을 때는 -1로 설정한다.
    int cmd; //명령, 읽기 혹은 쓰기를 의미한다.
    int errors; //에러 코드를 의미한다.
    unsigned long sector; //몇 번째 섹터인지를 의미한다.
    unsigned long nr_sectors; //볓 번 섹터인지를 의미한다.
    char * buffer; //byte 형식의 데이터를 저장해두는 버퍼이다.
    struct task_struct * waiting;
    struct buffer_head * bh;
    struct reuqest * next; //내부 항목들은 linked list 형태로 이어져 있다.
}

struct blk_dev_struct {
    void (*requesxt_fn)(void);
    struct request* current_request;
}

extern struct blk_dev_struct blk_dev[NR_BLK_DEV];
extern struct request request[NR_REQUEST];
extern struct task_struct * wait_for_request;