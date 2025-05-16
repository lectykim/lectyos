#define MAJOR_NR=1


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