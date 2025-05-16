void con_init(void)
{
    register unsigned char a;
    char* display_desc = "????";
    char* display_ptr;

    video_nul_columns = ORIG_VIDEO_COLS;
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