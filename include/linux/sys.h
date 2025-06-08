extern int sys_fork(); //system_call.s의 _sys_fork와 연동된다.
                        //어셈블리로 작성한 함수들을 c언어 함수명과 달리 이름 앞에 _가 붙는다.
extern int sys_setup();
extern int sys_exit();

extern int sys_read();
extern int sys_exit();
extern int sys_open();

fn_ptr sys_call_table[] = {
    sys_setup,sys_exit,sys_fork
}

