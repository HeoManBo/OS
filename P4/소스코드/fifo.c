#include"m.h"

page* fifo_head = NULL;
page* fifo_tail = NULL;


//페이지 프레임이 꽉 찬 경우 
//fifo_head를 head가 가리키는 페이지 프레임을 삭제시키고
//fifo_head가 가리키던 페이지 프레임이 가리키던 다음 페이지 프레임을 가리키게 만든다.
//이때 메모리 관리를 위해 삭제되는 프레임은 free 해준다.
void insertPage_FIFO(int page_num,page* head, page* tail){
	page* remove = head->next;
	//헤드가 가리키던 프레임의 다음 프레임을 가리키게 만든다.
	head->next = head->next->next;
	//헤드가 가리키던 프레임 메모리 해제 
	free(remove);
	remove == NULL;
	page_frame_size--;
	//이렇게 되면 페이지 프레임이 꽉차지 않게 되므로 페이지를 추가할 수 있게된다.
	addPage(page_num,head,tail);
}

void FIFO(int frame_size, int* page_stream, int stream_size,int* unused1,int* unused2){
	init_test(&fifo_head,&fifo_tail);
	buf_init();
	int idx=3;
	sprintf(save[0],"-------------FIFO 알고리즘 테스트--------------\n%c",'\0');
	sprintf(save[1],"Sequence||page_num||page_fault||page_frame_list\n%c",'\0');
    sprintf(save[2],"-----------------------------------------------\n%c",'\0');
	//500개의 스트림에 대한 FIFO 구현 
	for(int i=0; i<stream_size; i++){
		//페이지를 페이지 프레임에서 찾는 것을 성공하면 다음 페이지 스트림을 탐색
		if(search_frame(page_stream[i],0,fifo_head,fifo_tail) == 1){
			paging_success++;
			save_result_except_clock(i,page_stream[i],fifo_head,fifo_tail,0,idx++);
			continue;
		}
		//실패 했다면 프레임 크기와 현재 프레임을 차지하고 있는 
		//페이지 개수를 비교하여 페이지 크기와 동일한 페이지 개수를 가지고 있다면
		//FIFO 방식으로 fifo_head가 가리키는 페이지 번호를 페이지 프레임에 방출시키고
		//새로 들어오는 페이지 번호를 맨 끝 페이지 프레임에 위치시킨다.
		else {
			if(page_frame_size < frame_size){
			  addPage(page_stream[i],fifo_head,fifo_tail);
			}
			else {
				insertPage_FIFO(page_stream[i],fifo_head,fifo_tail);
			}
			save_result_except_clock(i,page_stream[i],fifo_head,fifo_tail,1,idx++);
			page_fault++;
		}
		sprintf(save[idx++],"------------------------------\n%c",'\0');
	}
	sprintf(save[idx++],"----------------------------------------------------\n%c",'\0');
	sprintf(save[idx++],"==== FIFO  페이징 알고리즘 결과는 다음과 같습니다.\n%c",'\0');
    sprintf(save[idx++],"page fault 횟수는 : %d\n%c", page_fault,'\0');
    sprintf(save[idx++],"Page hit 횟수는 : %d\n%c", paging_success,'\0');
	sprintf(save[idx++],"page fault rate 는 : %.0f%%\n%c", ((double)(page_fault)/(double)stream_size)*100,'\0');
    sprintf(save[idx++],"hit rate는 : %.0f%%입니다.\n%c", ((double)paging_success/(double)stream_size)*100,'\0');
	sprintf(save[idx++],"\n\n\n%c",'\0');
	save_and_print(idx);
	//FIFO 알고리즘 테스트시 동적할당한 메모리 해제 
	free_memory(fifo_head,fifo_tail);
}


/*
//알고리즘 테스트 
int main(void){
	int page_stream[]  ={2,3,2,1,5,2,4,5,3,2,5,2};
	int frame_size = 3;
	int stream_size = sizeof(page_stream)/sizeof(int);
	FIFO(frame_size, page_stream, stream_size);
	return 0;
}
*/
