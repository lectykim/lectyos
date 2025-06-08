#define LOW_MEM 0x100000
#define PAGING_MEMORY (15*1024*1024)
#define PAGING_PAGES (PAGING_MEMORY>>12)
#define MAP_NR(addr) (((addr)-LOW_MEM)>>12)
#define USED 100

#define invalidate()\
__asm__("movl %%eax,%%cr3"::"a"(0)) //CR3를 0으로 초기화

static long HIGH_MEMORY = 0;

static unsigned char mem_map[PAGING_PAGES] = {0,};

void mem_init(long start_mem,long end_mem)
{
    int i;

    HIGH_MEMORY = end_mem;
    for(int i=0;i<PAGING_PAGES;i++)
        mem_map[i] = USED;
    i = MAP_NR(start_mem);
    end_mem -= start_mem;
    end_mem >>= 12;
    while(end_mem-->0)
        mem_map[i++]=0;
}


unsigned long get_free_page(void) //빈 페이지를 구한다.
{
    register unsigned long __res asm("ax");

__asm__ ("std;\n\t" /*(1)DF=1설정, ->문자열 명령 인덱스(EDI)가 감소 방향으로 움직이게 함*/
        "repne;scasb\n\t"    /*(2) AL(=0)과 ES:EDI를 비교하여, ZF=1이 될 때까지(=0바이트를 찾을 때 까지 ECX번 반복한다.)*/
        "jne 1f \n\t" /*(3) 만약 ECX=0이어서 0바이트를 못 찾았다면 [모든 페이지가 사용 중이라면] 레이블 1로 분기 (실패)*/
        "movb %1,1(%%edi)\n\t" /*막 찾은 위치 (EDI+1)에 1 바이트 저장. ->mem_map[index]=1 할당 표시*/
        "sall $12,%%ecx \n\t"/*ECX <<= 12 -> 페이지 인덱스를 물리바이트 오프셋(4096배)로 전환*/
        "movl %%ecx,%%edx\n\t" /*LOW_MEM(커널 아래 예약된 바이트) 더하기 -> 실제 물리 주소*/
        "leal 4092(%%edx),%%edi \n\t"/*edi = edx+4092 -> 페이지의 마지막 4바이트 블록 위치*/
        "rep; stosl \n\t" /*EAX(=0)을 [EDI]에 1024번 써서 페이지 전체를 0으로 초기화 (DF=1)이니 역방향이다.*/
        "movl %%edx,%%eax\n" /*반환 값 __res를 물리주소 (EDX)에 넣는다.*/
        "1:" /*실패 시 eax=0을 그대로 출력한다.*/
        :"=a"(__res) /*출력, ->EAX 레지스터에 담겨 있는 __res*/
        :   "0"(0), /*입력0: 0(AL=0) -> repne scasb의 비교 값*/
            "i"(LOW_MEM), /*입력 2: LOW_MEM 상수*/
            "c"(PAGING_PAGES), /*입력 ECX 초기값: 페이지 수 (mem_map 길이)*/
            "D"(mem_map+PAGING_PAGES-1) /*입력 EDI 초기값: mem_map 끝부터 역 탐색*/
        :"di","cx","dx"); /*클로버 :  이 레지스터들이 내부에서 변경 됨*/
        return __res;
}

int copy_page_tables(unsigned long from, unsigned long to, long size)
{
    unsigned long * from_page_table;
    unsigned long * to_page_table;
    unsigned long this_page;
    unsigned long * from_dir,*to_dir;
    unsigned long nr;

    /*
    0x3fffff = 4MB, 페이지 테이블이 관리할 수 있는 영역이다.
    이진수로 따지면 22개의 1이 있으므로, 그 영역은 반드시 0이어야 한다.
    4MB의 연속 어드레스 공간에 대응하는 페이지 테이블을 만드는 것이다. 
    */
    if((from&0x3fffff) || (to&0x3fffff))
        panic("copy_page tables called with wrong alignment");

    /*
    하나의 페이지 디렉토리 엔트리는 4MB의 공간을 관리한다.
    그리고 각 아이템의 크기는 4바이트이다.
    따라서 아이템의 어드레스는 nr * 4이다.
    이 값에 MB단위를 붙히면 아이템에서 관리는 선형 어드레스의 시작 주소가 된다. 
    >>22의 표현은 어드레스를 MB단위로ㅓ 환산한 값이고, 0xffc는 페이지 디렉토리 엔트리를 구하는 것이다.
    */
   from_dir = (unsigned long*)((from>>20)&0xffc); /*pg_dir = 0*/
   to_dir= (unsigned long*)((to>>20)&0xffc);
   size = ((unsigned) (size+0x3fffff))>>22;
   for(; size-->0;from_dir++,to_dir++){
    if(1&to_dir)
        panic("copy_page_tables : already exist");
    if(!(1&from_dir))
        continue;

    /*from_dir은 페이지 엔트리의 주소이다. 0xfffff000이라는 표현은 하위 12비트를 0으로 만들고, 
    페이지 태이블 어드레스에 해당하는 상위 20비트는 그대로 두라는 뜻이다.*/
    from_page_table = (unsigned long *) (0xfffff000&*from_dir);
    if(!(to_page_table = (unsigned long *)get_free_page()))
        return -1; //memory 부족
    *to_dir = ((unsigned long)to_page_table)|7; //7은 111
    nr = (from==0)?0xA0:1024; //0xA0은 160, 복사할 페이지이다.
    for(;nr-->0;from_page_table++,to_page_table++){
        this_page = *from_page_table;
        if(!(1&this_page))
            continue;
        this_page &= ~2; //페이지 테이블의 속성 설정, ~2는 101b가 된다.

        *to_page_table = this_page;
        if(this_page > LOW_MEM){
            //1MB이내의 커널 영역은 유저페이지를 관리하지 않는다.

            *from_page_table = this_page;
            this_page -= LOW_MEM;
            this_page >>=12;
            mem_map[this_page]++; //참조 카운터 증가 시키기
        }
    }
   }
   invalidate();
   return 0;
}