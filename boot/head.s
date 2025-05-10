/*
* 32bit 시작
*
* 
*/
.text
.globl _idt,_gdt,_pg_dir,_tmp_floppy_area
_pg_dir:
startup_32:
    movl $0x10,%eax
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax, %gs
    lss stack_start,%esp
    call setup_idt
    call setup_gdt
    movl $0x10,%eax #reload all the segment registers
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax, %gs
    lss stack_start,%esp
    xorl %eax,%eax
1:  incl %eax
    movl %eax,0x000000
    cmpl %eax,0x100000
    je 1b
/*
* 486은 16bit를 써야 되기 때문에 supervisor mode를 위해서는, 
* verify_area()라는 함수를 불러야 한다.
* 486 유저는 int 16 math error를 내지 않기 위해 NE (#5) bit를 세팅해야한다.
*/

    movl %cr0,%eax
    andl $0x80000011, %eax
    orl $2,%eax
    call check_x87
    jmp after_page_tables

/* 287/387을 위한 ET 비트 체크 */
check_x87:
    fninit
    fstsw %ax
    cmpb $0,%al
    je 1f
    movl %cr0,%eax
    xorl $6,%eax
    movl %eax,%cr0
    ret
.p2align 2
1: .byte 0xDB, 0xE4
    ret

setup_idt:
    lea ignore_int, %edx
    movl $0x00080000, %eax
    movw %dx,%ax
    movw $0x8E00,%dx

    lea _idt,%edi
    mov $256,%ecx
rp_sidt:
    movl %eax, (%edi)
    movl %edx,4(%edi)
    addl $8,%edi
    dec %ecx
    jne rp_sidt
    lidt idt_descr
    ret

setup_gdt:
    lgdt gdt_descr
    ret

.org 0x1000
pg0:

.org 0x2000
pg1:

.org 0x3000
pg2:

.org 0x4000
pg3:

.org 0x5000

_tmp_floppy_area:
    .fill 1024,1,0

after_page_tables:
    pushl $0
    pushl $0
    pushl $0
    pushl $L6
    pushl $main
    jmp setup_paging
L6:
    jmp L6

int_msg:
    .asciz "Unknown interrupt \n\r"
.p2align 2
ignore_int:
    pushl %eax
    pushl %ecx
    pushl %edx
    push %ds
    push %es
    push %fs
    movl $0x10,%eax
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    pushl $int_msg
    popl %eax
    pop %fs
    pop %es
    pop %ds
    popl %edx
    popl %ecx
    popl %eax
    iret

.p2align 2
setup_paging:
    movl $1024*5,%ecx
    xorl %eax,%eax
    xorl %edi,%edi
    cld;rep;stosl
    movl $pg0+7,_pg_dir
    movl $pg1+7,_pg_dir+4
    movl $pg2+7,_pg_dir+8
    movl $pg3+7,_pg_dir+12
    movl $pg3+4092,%edi
    movl $0xfff007,%eax
    std
1: stosl
    subl $0x1000,%eax
    jge 1b
    xorl %eax,%eax
    movl %eax,%cr3
    movl %cr0,%eax
    orl $0x80000000,%eax
    movl %eax,%cr0
    ret

.p2align 2
.word 0
idt_descr:
    .word 256*8-1
    .long _idt
.p2align 2
.word 0
gdt_descr:
    .word 256*8-1
    .long _gdt

.p2align 3
_idt: .fill 256,8,0

_gdt: .quad 0x0000000000000000
      .quad 0x00c09a0000000fff
      .quad 0x00c0920000000fff
      .quad 0x0000000000000000
      .fill 252,8,0
