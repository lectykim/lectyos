!
! message 출력 테스트
! 과연 vga를 연결하는 셋업 코드가 없어도, 문자열은 출력 가능할 것인가

.org 0x7c00

BOOTSEG = 0x07c0 ! 부트 섹터는 여기에 로드 돼요
INITSEG = 0x9000 ! 부트 섹터를 여기로 옮겨요
SETUPSEG = 0x9020 ! SETUP은 여기서 시작돼요

entry start
start:
    mov ax,#BOOTSEG
    mov ds,ax
    mov ax,#INITSEG
    mov es,ax
    mov cx,#256
    sub si,si
    sub di,di
    rep
    movw

    mov ah,#0x03
    xor bh,bh
    int 0x10

    mov cx,#29
    mov bx,#0x0007 ! page = 0, attribute 7 (보통)
    mov bp,#msg1
    mov ax,#0x1301 !스트링 입력 후 커서 옮기기
    int 0x10

msg1:
    .byte 13,10 ! CR LF -> 새 줄 시작
    .ascii "LectyOS Mesage Test ..." ! ASCII문자 그대로 출력
    .byte 13,10,13,10

.org 510

boot_flag:
    .word 0xAA55
