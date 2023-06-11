#include"m.h"

page* c_head = NULL;
page* c_tail = NULL;
page* candidate_clock = NULL; //교체 후보를 가리킴
int size = 0;

void add_page_clock(int page_num, int frame_size){
	page* link = make_page_frame(page_num);
    //페이지 프레임이 비어있는 경우 (맨 처음 초기상태에 해당)
    if(c_head->next ==  NULL){
        c_head->next = link;
        c_tail->next = link;
    }
    //페이지 프레임이 비어있지 않은 경우
    //비어있는 맨 끝 페이지 프레임에 추가한다.
    else{
        page* cursor = c_tail->next;
        c_tail->next->next = link;
        c_tail->next = cursor->next;
    }
    page_frame_size++;
	//환영 버퍼식으로 교체 후보를 이동시킴
	if(candidate_clock == NULL){
		candidate_clock = link->next;
		//만약 페이지 프레임이 꽉찼다면 
		//교체 후보는 맨 처음 헤드를 가리키게 만듬 
		//해당 함수는 페이지 프레임이 꽉 차지 않았을 때만 작동
		if(page_frame_size == frame_size){
			candidate_clock = c_head->next;
		}
	}
}

//R비트가 0인 페이지를 찾는다.
void insertPage_CLOCK(int page_num){
	page* cursor = candidate_clock; //교체 후보부터 탐색한다.
	page* del = NULL;
	int del_num = -1;
	while(cursor->R == 1){ //R이 0인 페이지를 찾는다
		if(cursor == c_tail->next){ //만약 조사 페이지가 프레임에 끝일 때
			if(cursor->R == 0){ // 교체 대상이라면 반복문을 탈출
				del = cursor;
				break;
			}
			else if(cursor->R == 1){//프레임에 끝 페이지의 R비트가 1이라면
				cursor->R = 0; //0으로 바꾸고
				cursor = c_head->next; //프레임에 처음 페이지를 가리키도록 바꾼다 (환영식 구성)
			}
		}
		else{ //끝이 아니라면
			cursor->R = 0; //R비트를 0으로 바꾸고
			cursor = cursor->next; //다음 페이지 프레임 조사;
		}
	}
	//if(cursor == candidate_clcok) { //만약 한 바퀴를 다 돌았다면
	//	del = candidate; //맨 처음에 교체 후보가 제거 대상이 됨.
	//	del_num = candidate -> num;
	//	candidate = candidate->next;
	//}
	//else{ //한 바퀴를 다 안돌았다면
	//	if(cursor == c_tail->next){ //만약 교체 대상이 tail이 가리키는 것이라면 
	
	//교체 대상이전 프레임을 찾는다.
	page* asdf = c_head;
	
	while(asdf->next != cursor){
		asdf = asdf->next;
	}

	//교체 대상이 프레임에 끝이라면 
	if(cursor == c_tail->next){
		//다음 교체 후보를 c_head로 바꿈
		candidate_clock = c_head->next;
		c_tail->next = asdf;
		page_frame_size--;
		add_page_clock(page_num, size);
	}
	else{
		candidate_clock = cursor->next;
		asdf->next = make_page_frame(page_num);
		asdf->next->next = cursor->next;
	}
}
	

void CLOCK(int frame_size, int* page_stream, int stream_size,int* unused1, int* unused2){
	init_test(&c_head,&c_tail);
	buf_init();
	int idx=4;
	sprintf(save[0],"------------------- CLCOK 알고리즘 테스트---------------------------------------\n%c",'\0');
	sprintf(save[1]," use_bit은 *으로 표시되고 교체 후보는 ^으로 표시됩니다. 즉 페이지 프레임은 [페이지 번호|*|^] 형태를 가집니다.\n%c",'\0');
	sprintf(save[2],"Sequence||page_num||page_fault||page_frame_list[page_num|*(use_bit)|^(candidate)\n%c",'\0');
	sprintf(save[3],"--------------------------------------------------------------------------------\n%c",'\0');
	for(int i=0; i<stream_size; i++){
		if(search_frame(page_stream[i],0,c_head,c_tail) == 1){
			paging_success++;
			save_result_clock(i,page_stream[i],c_head,c_tail,0,idx++,candidate_clock,0,0);
			continue;
		}
		else{
			if(page_frame_size < frame_size){
				add_page_clock(page_stream[i], frame_size);
			}
			else{
				insertPage_CLOCK(page_stream[i]);
			}
			page_fault++;
			save_result_clock(i,page_stream[i],c_head,c_tail,1,idx++,candidate_clock,0,0);
		}
	}
	sprintf(save[idx++],"----------------------------------------------------\n%c",'\0');
	sprintf(save[idx++],"==== CLOCK  페이징 알고리즘 결과는 다음과 같습니다.\n%c",'\0');
    sprintf(save[idx++],"page fault 횟수는 : %d\n%c", page_fault,'\0');
    sprintf(save[idx++],"Page hit 횟수는 : %d\n%c", paging_success,'\0');
	sprintf(save[idx++],"page fault rate 는 : %.0f%%\n%c", ((double)(page_fault)/(double)stream_size)*100,'\0');
    sprintf(save[idx++],"hit rate는 : %.0f%%입니다.\n%c", ((double)paging_success/(double)stream_size)*100,'\0');
	sprintf(save[idx++],"\n\n\n%c",'\0');
	save_and_print(idx);
    free_memory(c_head,c_tail);
	candidate_clock = NULL;
}
/*
int main(){
    int page_stream[]  ={2,3,2,1,5,2,4,5,3,2,5,2};
    int frame_size = 3;
    int stream_size = sizeof(page_stream)/sizeof(int);
    CLOCK(frame_size, page_stream, stream_size);
    return 0;
}
*/
