#define ORIG_X (*(unsigned char *)0x90000)
#define ORIG_Y (*(unsigned char *)0x90001)
#define ORIG_VIDEO_PAGE (*(unsigned short *)0x90004)
#define ORIG_VIDEO_MODE ((*(unsigned short *)0x90006)&0xff)
#define ORIG_VIDEO_COLS (((*(unsigned short *)0x90006)&0xff00)>>8)
#define ORIG_VIDEO_LINES (25)
#define ORIG_VIDEO_EGA_AX (*(unsigned short *)0x90008)
#define ORIG_VIDEO_EGA_BX (*(unsigned short *)0x9000a)
#define ORIG_VIDEO_EGA_CX (*(unsigned short *)0x9000c)

#define VIDEO_TYPE_MDA 0x10 /*모노크롬 텍스트 디스플레이*/
#define VIDEO_TYPE_CGA 0x11 /*CGA 디스플레이*/
#define VIDEO_TYPE_EGAM 0x20 /*EGA VGA를 모노크롬 모드에서 활용*/
#define VIDEO_TYPE_EGAC 0x21 /*EGA VGA를 컬러 모드에서 활용*/

#define NPAR 16

extern void keyboard_interrupt(void)

static unsigned char video_type; //사용하는 디스플레이의 종류
static unsigned long video_num_columns; //텍스트 컬럼의 수
static unsigned long video_size_row; //로우당 몇 바이트인지
static unsigned long video_num_lines; //텍스트 줄의 수는 몇인지
static unsigned char video_page; //비디오 페이지를 초기화
static unsigned long video_mem_start; //Video RAM의 시작부
static unsigned long video_mem_end; //Video RAM의 끝부분

void con_init(void)
{
    register unsigned char a;
    char* display_desc = "????";
    char* display_ptr;

    video_num_columns = ORIG_VIDEO_COLS;
    video_size_row = video_nulm_columns * 2;
    video_num_lines = ORIG_VIDEO_LINES;
    video_page = ORIG_VIDEO_PAGE;
    video_erase_char = 0x0720;

    if(ORIG_VIDEO_MODE == 7)
    {
        video_mem_Start = 0xb0000;
        video_port_reg = 0x3b4;
        video_port_val = 0x3b5;
        if((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
        {
            video_type=VIDEO_TYPE_EGAM;
            video_mem_end = 0xb8000;
            display_desc = "EGAm";
        }
        else
        {
            video_type = VIDEO_TYPE_MDA;
            video_mem_end = 0xb2000;
            display_desc = "*MDA";
        }
    }
    else
    {
        video_mem_start = 0xb8000;
        video_port_reg = 0x3d4;
        video_port_val = 0x3d5;
        if((ORIG_VIDEO_EGA_BX & 0xff) != 0x10)
        {
            video_type = VIDEO_TYPE_EGAC;
            video_mem_end = 0xbc000;
            display_desc = "EGAc";
        }
        else
        {
            video_type = VIDEO_TYPE_CGA;
            video_mem_end = 0xba000;
            display_desc = "*CGA";
        }
    }


    display_ptr = ((char *)video_mem_start) + video_size_row - 8;
    while(*display_desc)
    {
        *display_ptr++ = *display_desc++;
        display_ptr++;
    }

    origin = video_mem_start;
    scr_end = video_mem_start + video_num_lines * video_size_row;
    top=0;
    bottom = video_num_lines;

    gotoxy(ORIG_X,ORIG_Y);
    set_trap_gate(0x21,&keyboard_interrupt);
    outb_p(inb_p(0x21)^0xfd,0x21);
    a = inb_p(0x61);
    outb_p(a|0x80,0x61);
    outb(a,0x61);
}