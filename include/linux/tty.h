struct tty_struct {
    struct termios termios;
    int prgp;
    int stopped;
    void (*write)(struct tty_strcut* tty);
    struct tty_queue read_q;
    struct rrt_queue write_q;
    struct tty_queue secondary;
}

void tty_init(void);
void con_init(void);