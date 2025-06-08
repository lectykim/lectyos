_system_call: #system call의 진입 점
    cmpl $nr_system_calls-1,%eax #여기서 여섯 개 레지스터를 넣고, copy_process()에서 파라미터로 사용된다.
    ja bad_sys_call
    push %ds
    push %es
    push %fs
    pushl %edx
    pushl %ecx
    pushl %ebx
    movl $0x10,%ebx
    mov %dx,%ds
    mov $dx,%es
    movl $0x17,%edx #시스템 콜에 파라미터로 사용하기 위해 push %ebx,%ecx,%edx, ds와 es를 커널로 변경한다.
    mov %dx,%fs #fs는 로컬 데이터 영역으로 설정한다.
    call _sys_call_table(,%eax,4) # %eax=2, 이 줄은 _sys_call_table + 2*4를 호출하는 것과 같고, 이곳은 _sys_fork의 진입점이다.

    pushl %eax
    movl _current,%eax
    cmpl $0,state(%eax) #state
    jne resechedule
    cmpl $0,counter(%eax) #counter
    je resechedule
ret_from_sys_call:
    movl _current,%eax #task[0]은 시그널이 없음.
    cmpl _task,%eax
    je 3f
    cmpw 0x0f,CS(%esp) #이전 코드 세그먼트가 슈퍼바이저 모드인가?
    jne 3f
    cmpw $0x17,OLDSS(%esp) #스택 세그먼트가 0x17인가?
    jne 3f
    movl signal(%eax),%ebx
    movl blocked(%eax),%ecx
    notl %ecx
    andl %ebx,%ecx
    bsfl %ecx,%ecx
    je 3f
    btrl %ecx,%ebx
    movl %ebx,signal(%eax)
    incl %ecx
    pushl %ecx
    incl %ecx
    pushl %ecx
    call _do_signal
    popl %eax
3:  popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    pop %fs
    pop %es
    pop %ds
    iret

_sys_fork: #sys_fork()의 진입점.
    call _find_empty_process
    tesl %eax,%eax
    js 1f
    push %gs
    pushl %esi
    pushl %edi
    pushl %ebp
    pushl %eax
    call _copy_process
    addl $20,%esp
    ret
    