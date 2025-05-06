!
! setup.s는 BIOS에서 신뢰할 수 있게 시스템 데이터를 가져옵니다.
! 그리고 적절한 시스템 메모리에 넣습니다/
!  setup.s와 시스템은 부트블록에 로드됩니다.
!
! 이 코드는 memory/dsk/other 파라미터를 묻고 있습니다.
! 그리고 안전한 장소인 0x90000-0x901FF에 적재합니다.
! 부트 블록이 사용되는 장소는 보호 모드로 보호됩니다.

! 이 변수는 bootsect.s랑 같아야 좋아요.

INITSEG = 0x9000
SYSSEG = 0x1000
SETUPSEG = 0x9020

.globl begtext, begdata, begvss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

entry start:
start:

! 현재 커서 저장하기
    mov ax,#INITSEG
    mov ds,ax
    mov ah,#0x03 !read cursor pos
    xor bh,bh
    int 0x10 ! 잘 알려진 곳에 저장
    mov [0], dx ! it from 0x90000

! 메모리 크기 알아오기 (확장 메모리, kB)
    mov ah,#0x88
    int 0x15
    mov [2], ax

! Video-card 데이터 받아오기
    mov ah,0x0f
    int 0x10
    mov [4],bx ! bh = 디스플레이 페이지
    mov [6],ax ! al = video mode, ah = window width

! EGA/VGA 설정 파라미터 체크하기
    mov ah,#0x12
    mov bl,#0x10
    int 0x10
    mov [8],ax
    mov [10],bx
    mov [12],cx

! hd0 data 가져오기
    mov ax,#0x0000
    mov ds,ax
    lds si,[4*0x41]
    mov ax, #INITSEGB  
    mov es,ax
    mov di,#0x0080
    mov cx,#0x10
    rep
    movsb

! hd1 data 가져오기
    mov ax,#0x0000
    moc ds,ax
    lds si,[4*0x46]
    mov ax,#INITSEG
    mov es,ax
    mov di,#0x0090
    mov cx,#0x10
    rep
    movsb

! hd1이 있는지 체크하기
    mov ax,#0x1500
    mov dl,0x81
    int 0x13
    jc no_disk1
    cmp ah,#3
    je is_disk1
no_disk1:
    mov ax,#INITSEG
    mov es,ax
    mov di,#0x0090
    mov cx,#0x10
    mov ax,#0x00
    rep
    stosb
is_disk1:
! 이제 우리는 보호 모드로 전환하길 원해.
    cli ! no interrupt allowed.
! 먼저 시스템을 올바른 곳으로 이동.
    mov ax,#0x0000
    cld
do_move:
    mov es,ax !목적지 세그먼트
    add ax,#0x1000
    cmp ax,#0x9000
    jz end_move
    mov ds,ax !소스 세그먼트
    sub di,di
    sub si,si
    mov cx,#0x8000
    rep
    movsw
    jmp do_move

! 그리고 우린 세그먼트 디스크립터를 로딩한다.
end_move:
    mov ax,#SETUPSEG
    mov ds,ax
    lidt idt_48 ! load idt with 0.0
    lgdt gdt_48 ! load gdt with whatever appropriate

!that was painless now we enable a20
    call empty_8042
    mov al,#0xD1
