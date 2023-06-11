#include"m.h"
page* lfu_head = NULL;
page* lfu_tail = NULL;
void insertPage_LFU(int page_num){
	page* del = NULL; //삭제할 페이지 프레임을 가리킴
	int min_counter = 987654321;
	page* cursor = lfu_head;
	//가장 작은 counter값을 가진 프레임을 찾는다.
	while(cursor->next != lfu_tail->next){
		cursor = cursor->next;
		if(cursor->counter < min_counter){
			del = cursor;
			min_counter = cursor->counter;
		}
	}
	//lfu_tail이 가리키는 페이지 프레임도 확인한다.
	if(cursor->next == lfu_tail->next){
		if(cursor->next->counter < min_counter){
			del = cursor->next;
		}
	}
	//커서를 지우는 프레임 이전까지 이동시킨다. 
	cursor=lfu_head;
	while(cursor->next != del){
		cursor = cursor->next;
	}
	//만약 삭제할 페이지 프레임이 lfu_tail이 가리키는 것이라면
	//새로 페이지 프레임을 만들고 lfu_tail이 만든 프레임을 가리키게 한다.
	if(del == lfu_tail->next){
		cursor->next = NULL;
		lfu_tail->next = cursor;
		page_frame_size--;
		addPage(page_num,lfu_head,lfu_tail);
	}
	//lfu_tail이 가리키는 것이 아니라면
	//lfu_tail에 새로운 페이지 프레임을 가리키도록 만들고
	//삭제하려는 페이지 프레임을 가리키던 페이지 프레임은 삭제되는 페이지 프레임이 가리키던
	//페이지 프레임을 가리키도록 만든다.
	else{
		cursor->next = del->next;
		page_frame_size--;
		addPage(page_num,lfu_head,lfu_tail);
	}
	//삭제하는 페이지 프레임 삭제
	free(del);
}

void LFU(int frame_size, int* page_stream, int stream_size, int* unused1, int* unused2){
	init_test(&lfu_head,&lfu_tail);
	buf_init();
	int idx=3;
	sprintf(save[0],"-------------LFU 알고리즘 테스트--------------\n%c",'\0');
	sprintf(save[1],"Sequence||page_num||page_fault||page_frame_list\n%c",'\0');
    sprintf(save[2],"-----------------------------------------------\n%c",'\0');
	for(int i=0; i<stream_size; i++){
		if(search_frame(page_stream[i], 0, lfu_head, lfu_tail) == 1){
			paging_success++;
			save_result_except_clock(i+1,page_stream[i],lfu_head,lfu_tail,0, idx++);
			continue;
		}
		else{
			if(page_frame_size < frame_size){
				addPage(page_stream[i],lfu_head,lfu_tail);
			}
			else{
				insertPage_LFU(page_stream[i]);
			}
			save_result_except_clock(i+1,page_stream[i],lfu_head,lfu_tail,1, idx++);
			page_fault++;
		}
		sprintf(save[idx++],"------------------------------\n%c",'\0');
    }
	sprintf(save[idx++],"----------------------------------------------------\n%c",'\0');
	sprintf(save[idx++],"==== LFU 페이징 알고리즘 결과는 다음과 같습니다.\n%c",'\0');
    sprintf(save[idx++],"page fault 횟수는 : %d\n%c", page_fault,'\0');
    sprintf(save[idx++],"Page hit 횟수는 : %d\n%c", paging_success,'\0');
	sprintf(save[idx++],"page fault rate 는 : %.0f%%\n%c", ((double)(page_fault)/(double)stream_size)*100,'\0');
    sprintf(save[idx++],"hit rate는 : %.0f%%입니다.\n%c", ((double)paging_success/(double)stream_size)*100,'\0');
	sprintf(save[idx++],"\n\n\n%c",'\0');
	save_and_print(idx);
	free_memory(lfu_head,lfu_tail);
}
/*
int main(void){
	int pages[] = {1, 2, 3, 4, 2, 1, 5 };
	LFU(3, pages, 7);
	return 0;
}
*/
