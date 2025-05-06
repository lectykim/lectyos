! LECTYOS SYSSIZE는 아직 계산하지 않았어요 >_<
! 일단 리눅스 0.11처럼 0x3000 click = 0x30000 = 196kB로 해둘게요
SYS_SIZE = 0x3000
! 일단 BIOS의 관례 상, 0x7c00으로 로딩될 거에요
! 그러나 0x90000으로 복사하고 점프할게요
!
! 그리고 setup을 0x90200으로 설정하고, bios interrupts를 이용하여 시스템을 
! 작동시킬 거에요



SETUPLEN = 4 ! 셋업 섹터는 4개에요!
BOOTSEG = 0x07c0 ! 부트 섹터는 여기에 로드 돼요
INITSEG = 0x9000 ! 부트 섹터를 여기로 옮겨요
SETUPSEG = 0x9020 ! SETUP은 여기서 시작돼요
SYSSEG = 0x1000 ! 시스템은 0x10000 (65336)에 로드돼요!
ENDSEG = SYSSEG + SYS_SIZE ! 0X10000 + SYSSIZE만큼만 로드하고 로딩 멈춰요

! ROOT_DEV: 0x000 - 미정의하면 플루피 디스크 타입과 같아진다구요?
! 0x301 - 첫 번째 드라이브의 첫 번째 파티션을 의미해요
ROOT_DEV = 0x301 

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
    jmpi go, INITSEG
! BOOTSEG에서부터 INITSEG로 데이터 복사하기
! 256 워드(512 바이트) 만큼 복사해요! (512바이트는 1섹터 크기)
go:
    mov ax,cs
    mov ds,ax
    mov es,ax
! 스택을 0x9FF00부터 0x90000까지 쓸 수 있게 설정해둬요
    mov ss,ax
    mov sp, #0xFF00 

! bootblock 뒤에 셋업 섹터를 배치 해요
! es가 이미 세팅되어있음을 이해해야 돼요.

load_setup:
    mov dx,#0x0000 ! drive 0, head 0
    mov cx,#0x0002 ! sector 2, track 0
    mov bx,#0x0200 ! address = 512 in INITSEG
    mov ax,#0x0200+SETUPLEN ! service 2, 섹터의 수
    int 0x13
    jnc ok_load_setup ! ok - continue
    mov dx,#0x0000 
    mov ax,#0x0000 ! 플로피디스크 리셋하기
    int 0x13
    j load_setup

ok_load_setup:

! 디스크 드라이브 파라미터를 받아와요.
    mov dl,#0x00
    mov ax,#0x0800 ! AH=8은 디스크 파라미터를 의미해요
    int 0x13
    mov ch,#0x00
    seg cs
    mov sectors, cx
    mov ax,#INITSEG
    mov es,ax

! 여기서 메세지를 출력해요
    mov ah,#0x03 ! 커서의 포지션 읽어오기
    xor bh,bh
    int 0x10

    mov cx,#29 ! 문자열 길이는 24
    mov bx,#0x0007 ! page 0, attribute 7(normal)
    mov bp,#msg1
    mov ax,#0x1301 ! 스트링을 입력하고 커서를 옮겨요
    int 0x10

sectors:
    .word 0
msg1:
    .byte 13,10
    .ascii "LectyOS Mesage Test ..."
    .byte 13, 10, 13, 10
.org 510
boot_flag:
    .word 0xAA55

