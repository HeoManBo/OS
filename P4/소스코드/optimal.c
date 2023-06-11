#include"m.h"

page* op_head = NULL;
page* op_tail = NULL;

page* predict(int frame_size, int* page_stream, int start_idx, int stream_size){
    //만약 페이지 프레임 공간이 하나라면 바로 리턴
    if(op_head->next == op_tail->next){
        return op_head;
    }
    int value=0, fathest = start_idx, j=0;
    page* ret = NULL;
    page* cursor = op_head;
    //페이지 프레임에 존재하는 각 페이지 번호마다
    //페이지 스트림에서 언제 참조되는지 확인한다.
    for(; cursor->next != NULL; cursor = cursor->next){
        value = cursor->next->num;
        for(j = start_idx; j<stream_size; j++){
            //가장 나중에 참조되는 페이지 번호를 가리키는 페이지를 찾는다.
            if(value == page_stream[j]){
                if(fathest < j){
                    fathest = j;
                    ret = cursor;
                }
                break;
            }
        }
        //만일 해당 페이지 번호가
        //이후 페이지 스트림을 모두 탐색해도 참조되지 않는다면
        //해당 페이지 번호를 리턴한다.
        //큐의 방식을 채용해 여러 개의 페이지 번호가 참조되지 않는다면
        //먼저 페이지 프레임에 추가된 페이지를 제거한다.
        if(j==stream_size){
            ret = cursor;
            break;
        }
    }

    //만약 모든 페이지가 참조되지 않는다면
    //큐의 방식대로 제일 오래전에 프레임에 추가된 페이지를 교체한다.
    if(ret == NULL){
        ret = op_head;
    }
    return ret;
}

void insert_OPTIMAL(page* cursor, int page_num){
	//삭제하는 페이지 프레임을 가리킴 
    page* del = cursor->next;
    //만약 교체 대상이 op_tail이 가리키는 것이라면
    if(del == op_tail->next){
        //op_tail이 cursor 프레임을 가리키게 만들고
        cursor->next = NULL;
        op_tail->next = cursor;
        page_frame_size--;
        addPage(page_num,op_head,op_tail);
    }
    //그 외의 경우는 그냥 교체하면됨.
    //교체 대상이 op_head가 가리키는 것이여도 상관없음
    else{
        //교체 대상이 가리키던 것을
        //교체 대상을 가리키던 페이지 프레임이
        //가리키게 만듬.
        cursor->next = del->next;
        page_frame_size--;
        //링크드 리스트 끝에 페이지 프레임 추가
		addPage(page_num,op_head,op_tail);
    }
    free(del);
}

void OPTIMAL(int frame_size, int* page_stream, int stream_size,int* unused1, int* unused2){
	init_test(&op_head,&op_tail);
	for(int i=0; i<stream_size; i++){
		printf("%d %d %d\n",page_stream[i],unused1[i],unused2[2]);
	}
	buf_init();
	int idx=3;
	sprintf(save[0],"-------------OPTIMAL 알고리즘 테스트--------------\n%c",'\0');
	sprintf(save[1],"Sequence||page_num||page_fault||page_frame_list\n%c",'\0');
	for(int i=0; i<stream_size; i++){
		if(search_frame(page_stream[i], 0,op_head,op_tail) == 1){
			paging_success++;
			save_result_except_clock(i,page_stream[i],op_head,op_tail,0, idx++);
			continue;
		}
		else{
			if(page_frame_size < frame_size){
				addPage(page_stream[i],op_head,op_tail);
			}
			else{
				page* exchange = predict(frame_size,page_stream, i, stream_size);
				insert_OPTIMAL(exchange, page_stream[i]);
			}
			save_result_except_clock(i,page_stream[i],op_head,op_tail,1,idx++);
			page_fault++;
		}
		sprintf(save[idx++],"------------------------------\n%c",'\0');
	}
	sprintf(save[idx++],"----------------------------------------------------\n%c",'\0');
	sprintf(save[idx++],"==== OPTIMAL  페이징 알고리즘 결과는 다음과 같습니다.\n%c",'\0');
    sprintf(save[idx++],"page fault 횟수는 : %d\n%c", page_fault,'\0');
    sprintf(save[idx++],"Page hit 횟수는 : %d\n%c", paging_success,'\0');
	sprintf(save[idx++],"page fault rate 는 : %.0f%%\n%c", ((double)(page_fault)/(double)stream_size)*100,'\0');
    sprintf(save[idx++],"hit rate는 : %.0f%%입니다.\n%c", ((double)paging_success/(double)stream_size)*100,'\0');
    printf(save[idx++],"\n\n\n%c",'\0');
	save_and_print(idx);
	free_memory(op_head,op_tail);
}
/*
int main(){
    int page_stream[]  ={2,3,2,1,5,2,4,5,3,2,5,2};
	int page_stream2[] = {7, 0, 1, 2,0, 3, 0, 4, 2, 3, 0, 3, 2, 1,2, 0, 1, 7, 0, 1};
    int frame_size = 3;
	int frame_size2 = 3;
    int stream_size = sizeof(page_stream)/sizeof(int);
	int stream_size2 = sizeof(page_stream2)/sizeof(int);
    OPTIMAL(frame_size, page_stream, stream_size);
    return 0;
}
*/
