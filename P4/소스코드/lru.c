#include"m.h"

page* lru_head = NULL;
page* lru_tail = NULL;


//페이지 프레임이 꽉 차지 않는 경우..
//인자로 넘어온 페이지 번호를 lru_tail이 가리키는 페이지 프레임에 추가시킨다.
void addPage_LRU(int page_num,int i){
    page* link = make_page_frame(page_num);
    link->counter++;
	link->ref = i;
    //페이지 프레임이 비어있는 경우 (맨 처음 초기상태에 해당)
    if(lru_head->next ==  NULL){
        lru_head->next = link;
        lru_tail->next = link;
    }
    //페이지 프레임이 비어있지 않은 경우
    //비어있는 맨 끝 페이지 프레임에 추가한다.
    else{
        page* cursor = lru_tail->next;
        lru_tail->next->next = link;
        lru_tail->next = cursor->next;
    }
    page_frame_size++;
}

//가장 오래전에 참조된 페이지를 찾는다 -->  페이지 구조체에서 가장 작은 ref 값을 찾음
//즉 추가, 탐색시 ref 값을 참조 스트림에서 몇 번째로 참조됐는지를 저장하므로
//ref가 가장 작은 값이 교체될 페이지이다. 
void insertPage_LRU(int page_num, int ref){
	int min_ref = lru_head->next->ref;
	page* cursor = lru_head;
	page* del = lru_head->next;
	//lru_tail이 가리키는 페이지 프레임 이전까지 일단 후보를 탐색하면서
	//ref가 가장 작은 값을 찾는다.
	while(cursor->next != lru_tail->next){
		cursor = cursor->next;
		if(cursor->ref < min_ref){
			del = cursor;
			min_ref = cursor->ref;
		}
	}
	//lru_tail이 가리키는 페이지 프레임도 확인한다.
    if(cursor->next == lru_tail->next){
        if(cursor->next->ref < min_ref){
            del = cursor->next;
        }
    }
	//커서를 지우는 페이지 프레임을 가리키는 페이지까지 이동함
	cursor = lru_head;
	//교체되기 전까지 커서를 옮김
	while(cursor->next != del){
		cursor = cursor->next;
	}
	//만약 교체 대상이 lru_tail 이 가리키는 것이라면
	if(del == lru_tail->next){
		cursor->next = NULL;
		lru_tail->next = cursor;
        page_frame_size--;
        addPage_LRU(page_num,ref);
	}
	//lru_head와 lru_tail사이에 존재한다면
	else{
		cursor->next = NULL;
		cursor->next = make_page_frame(page_num);
		cursor = cursor->next;
		cursor->ref = ref;
		cursor->next = del->next; //삭제하려는 페이지 프레임이 가리키던 다음 페이지프레임을 가리키게함
	}
	free(del);
}

void LRU(int frame_size, int* page_stream, int stream_size, int* unused1, int* unused2){
	init_test(&lru_head,&lru_tail);
	buf_init();
	int idx=3;
	sprintf(save[0],"-------------LRU 알고리즘 테스트--------------\n%c",'\0');
	sprintf(save[1],"Sequence||page_num||page_fault||page_frame_list\n%c",'\0');
    sprintf(save[2],"-----------------------------------------------\n%c",'\0');
	for(int i=0; i<stream_size; i++){
		if(search_frame(page_stream[i], i,lru_head,lru_tail) == 1){
			paging_success++;
			save_result_except_clock(i,page_stream[i],lru_head,lru_tail,0,idx++);
			continue;
		}
		//페이지 폴트시
		else{
			if(page_frame_size < frame_size){
				addPage_LRU(page_stream[i], i);
			}
			else{
				insertPage_LRU(page_stream[i],i);
			}
			save_result_except_clock(i,page_stream[i],lru_head,lru_tail,1,idx++);
			page_fault++;
		}
		sprintf(save[idx++],"------------------------------\n%c",'\0');
	}
	sprintf(save[idx++],"----------------------------------------------------\n%c",'\0');
	sprintf(save[idx++],"==== LRU 페이징 알고리즘 결과는 다음과 같습니다.\n%c",'\0');
    sprintf(save[idx++],"page fault 횟수는 : %d\n%c", page_fault,'\0');
    sprintf(save[idx++],"Page hit 횟수는 : %d\n%c", paging_success,'\0');
	sprintf(save[idx++],"page fault rate 는 : %.0f%%\n%c", ((double)(page_fault)/(double)stream_size)*100,'\0');
    sprintf(save[idx++],"hit rate는 : %.0f%%입니다.\n%c", ((double)paging_success/(double)stream_size)*100,'\0');
	save_and_print(idx);
    //알고리즘 테스트시 동적할당한 메모리 해제
    free_memory(lru_head,lru_tail);
}

/*
int main(){
	int page_stream[]  ={2,3,2,1,5,2,4,5,3,2,5,2};
    int frame_size = 3;
    int stream_size = sizeof(page_stream)/sizeof(int);
	LRU(frame_size, page_stream, stream_size);
	return 0;
}
*/
