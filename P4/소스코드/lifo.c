#include"m.h"

page* lifo_head = NULL;
page* lifo_tail = NULL;

//LIFO 알고리즘에서 페이지 교체 과정
//가장 최근에 추가된 페이지를 제거시키고 새로운 페이지를 추가시킨다.
void insertPage_LIFO(int page_num){
    page* remove = lifo_head;
    while(remove->next != lifo_tail->next){
        remove = remove->next;
    }
    //현재 remove가 가리키는 페이지는 페이지 프레임에서 가장 최근에 추가된 페이지 프레임
    //앞 페이지 프레임이다.
    page* cursor = remove->next;
    //프레임을 삭제함.
    free(cursor);
    //lifo_tail을 삭제한 이전 페이지 프레임을 가리키게 한다.
    lifo_tail->next = remove;
    lifo_tail->next->next = NULL;
    page_frame_size--;
    addPage(page_num,lifo_head,lifo_tail);
}

void LIFO(int frame_size, int* page_stream, int stream_size,int* unused1, int* unused2){
    init_test(&lifo_head,&lifo_tail);
	buf_init();
	int idx=3;
	sprintf(save[0],"-------------LIFO 알고리즘 테스트--------------\n%c",'\0');
	sprintf(save[1],"Sequence||page_num||page_fault||page_frame_list\n%c",'\0');
    sprintf(save[2],"-----------------------------------------------\n%c",'\0');
    //입력 스트림에 대한 LIFO구현
    for(int i=0; i<stream_size; i++){
        if(search_frame(page_stream[i],0,lifo_head,lifo_tail) == 1){
            paging_success++;
			save_result_except_clock(i,page_stream[i],lifo_head,lifo_tail,0, idx++);
            continue;
        }
        else{
            if(page_frame_size < frame_size){
				addPage(page_stream[i],lifo_head,lifo_tail);
            }
            else{
                insertPage_LIFO(page_stream[i]);
            }
            page_fault++;
			save_result_except_clock(i,page_stream[i],lifo_head,lifo_tail,1, idx++);
        }
		sprintf(save[idx++],"------------------------------\n%c",'\0');
    }
	sprintf(save[idx++],"----------------------------------------------------\n%c",'\0');
	sprintf(save[idx++],"==== LIFO  페이징 알고리즘 결과는 다음과 같습니다.\n%c",'\0');
    sprintf(save[idx++],"page fault 횟수는 : %d\n%c", page_fault,'\0');
    sprintf(save[idx++],"Page hit 횟수는 : %d\n%c", paging_success,'\0');
	sprintf(save[idx++],"page fault rate 는 : %.0f%%\n%c", ((double)(page_fault)/(double)stream_size)*100,'\0');
    sprintf(save[idx++],"hit rate는 : %.0f%%입니다.\n%c", ((double)paging_success/(double)stream_size)*100,'\0');
	sprintf(save[idx++],"\n\n\n%c",'\0');
	save_and_print(idx);
    //알고리즘 테스트시 동적할당한 메모리 해제
    free_memory(lifo_head,lifo_tail);
}

/*
//알고리즘 테스트
int main(void){
    //LIFO테스트
    int p[] = {3,1,2,1,6,5,1,3};
    int p_s = 8;
    LIFO(3, p, p_s);
    return 0;
}
*/
