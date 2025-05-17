_system_call:
    cmpl $nr_system_calls-1,%eax
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
    movl $0x17,%edx
    mov %dx,%fs
    call _sys_call_table(,%eax,4)

    pushl %eax
    movl _current,%eax
    cmpl $0,state(%eax)
    jne resechedule
    cmpl $0,counter(%eax)
    je resechedule
ret_from_sys_call:
    movl _current,%eax
    cmpl _task,%eax
    je 3f
    cmpw 0x0f,CS(%esp)
    jne 3f
    cmpw $0x17,OLDSS(%esp)
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

_sys_fork:
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
    