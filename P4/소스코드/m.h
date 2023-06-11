//각종 헤더 파일 및 구조체 정의를 위한 폴더
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<ctype.h>

//페이지 프레임 구조체
typedef struct node{
	int num; //페이지 번호
	int ref; //언제 참조됐는지 확인(LRU)
	int R; // r- bit;(clock/enhanced second clock)
	int W; // w- bit; (enhanced second clock)
	int counter; //몇번 참조됐는지 확인(LFU)
    struct node *next; // 다음 페이지를 가리키는 포인터(링크드 리스트의 형태를 가짐)
}page;

//빈 페이지 프레임 공간을 만든다.
page* make_page_frame(int page_num);

//페이지 프레임 링크드 리스트를 메모리에서 모두 해제한다.
void free_memory(page* head,page* tail);

//해당 페이지 번호가 페이지 프레임에 존재하는지 확인한다
//만약 존재한다면 1, 그렇지 않다면 -1을 리턴한다.
int search_frame(int page_num, int ref, page* head, page* tail);

//각 알고리즘 수행시 초기화 작업
void init_test(page** head, page** tail);

//링크드 리스트 tail에 새로운 페이지 프레임을 추가한다.
void addPage(int page_num,page* head, page* tail);

//마지막 2개의 파라미터는 ESC의 read_bit와 write_bit를 위함임. 
void FIFO(int, int*, int,int*,int*);
void LIFO(int, int*, int,int*,int*);
void OPTIMAL(int, int*, int,int*,int*);
void LRU(int, int*, int,int*,int*);
void LFU(int, int* , int,int*,int*);
void CLOCK(int, int*, int,int*,int*);
void ESC(int, int*, int, int*, int*);

int page_frame_size; // 링크드 리스트 연결된 프레임 개수
int page_fault; //페이지 폴트 수
int paging_success; //페이지 히트 수

//시뮬레이션 결과를 저장할 파일 포인터
//main함수에서 파일을 열게됨.
FILE* fd;
//시뮬레이션 결과를 파일에 저장할 버퍼
char save[3000][300];

void buf_init();
//시뮬레이션 결과 저장 및 출력 
void save_result_except_clock(int sequence,int page_num, page* head, page* tail, int flag,int j);
void save_result_clock(int sequence,int page_num, page* head, page* tail, int flag,int j, page* candidate,int flag2,int modify_bit);
void save_and_print(int idx);
