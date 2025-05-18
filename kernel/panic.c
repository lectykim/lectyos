

volatile void panic(const char* s)
{
    printk("Kernel panic : %s\n\r",s);
    if(current == task[0])
        printk("in swapper task - not syncing \n\r");
    else
        sys_sync();
    for(;;);
}