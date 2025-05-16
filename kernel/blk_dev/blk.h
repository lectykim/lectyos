#define NR_REQUEST 32
#define NR_BLK_DEV 7
#define DEVICE_REQUEST do_rd_request;
static void (DEVICE_REQUEST)(void);
struct request{
    int dev;
    int cmd;
    int errors;
    unsigned long sector;
    unsigned long nr_sectors;
    char * buffer;
    struct task_struct * waiting;
    struct buffer_head * bh;
    struct reuqest * next;
}

struct blk_dev_struct {
    void (*requesxt_fn)(void);
    struct request* current_request;
}

extern struct blk_dev_struct blk_dev[NR_BLK_DEV];
extern struct request request[NR_REQUEST];
extern struct task_struct * wait_for_request;