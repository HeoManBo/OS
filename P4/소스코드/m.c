#include"m.h"

//새로운 페이지 프레임 객체를 만든다.
page* make_page_frame(int page_num){
    page* init = (page *)malloc(sizeof(page));
    init->num = page_num;
    init->ref = 0;
    init-> R = 1; // clock 알고리즘에서 참조가 되었는지 안되었는지 확인, second change에서는 Read bit로 사용함 
    init-> W = 0;
	init->counter = 0;
    init->next = NULL;
    return init;
}

void init_test(page** head, page** tail){ //페이지 교체 알고리즘 수행시 초기화해야할 목록들 수행
    *head = make_page_frame(-1);
    *tail = make_page_frame(-1);
    page_frame_size = 0;
    page_fault = 0;
    paging_success = 0;
}


//동적할당한 페이지 프레임을 메모리에서 모두 할당 해제한다.
void free_memory(page* head,page* tail){
    page* cursor = head;
    page* next = cursor->next;
    while(cursor->next != NULL){
        free(cursor);
        cursor = next;
        next = next->next;
    }
    free(cursor);
	free(tail);
}

//해당 페이지 번호가 페이지 프레임에 존재하는지 확인한다
//만약 존재한다면 1, 그렇지 않다면 -1을 리턴한다.
int search_frame(int page_num, int ref, page* head, page* tail)
{
    page* cursor = head;
    if(cursor->next == NULL){
        return -1;
    }
    while(cursor->next != tail->next){
        cursor = cursor->next;
        if(cursor->num == page_num){
			cursor->counter++; //참조 counter 값 증가 LFU에서 사용
			cursor->ref = ref; //몇번째 참조 스트링에서 참조 되었는지 저장 LRU에서 사용
			if(cursor->R == 0){ //CLOCK 알고리즘에서 탐색하려는 페이지 번호가 페이지 리스트에 존재할 때 해당 페이지 프레임의 R비트가 0이면 1로 수정
				cursor->R = 1;
			}
            return 1;
        }
    }
	cursor = cursor->next;
	if(cursor->num == page_num){
		cursor->counter++;
		cursor->ref = ref;
		if(cursor->R == 0){
			cursor->R =1;
		}
		return 1;
	}

    //끊나지 않았다면 해당 페이지 번호가 페이지 프레임에 존재하지 않으므로
    //-1를 리턴한다
    return -1;
}

//인자로 넘어온 페이지 번호를 바탕으로 페이지 프레임을 만들어 페이지 리스트 tail에 추가한다. 
void addPage(int page_num,page* head, page* tail){
    page* new = make_page_frame(page_num);
    new->counter++;
    //페이지 프레임이 비어있는 경우 (맨 처음 초기상태에 해당)
    if(head->next ==  NULL){
        head->next = new;
        tail->next = new;
    }
    //페이지 프레임이 비어있지 않은 경우
    //비어있는 맨 끝 페이지 프레임에 추가한다.
    else{
        page* cursor = tail->next;
        tail->next->next = new;
        tail->next = cursor->next;
    }

    page_frame_size++;
}

//파일에 실행 결과를 저장하기 위한 버퍼 초기화 
void buf_init(){
	for(int i=0; i<3000; i++){
		for(int j=0; j<300; j++){
			save[i][j] = 0;
		}
	}
}

//sequence+1번째에 페이지 스트림에 대한 결과를 갖는 페이지 리스트의 상태와 페이지 폴트/히트 등을 저장한다.
void save_result_except_clock(int sequence,int page_num, page* head, page* tail, int flag,int j){
	char buffer[400];
	char* what = flag == 1? "YES" : "NO";
	int idx=0;
	char f = 'N';
	if(flag == 1){
		f = 'Y';
	}
	page* cursor = head;
	while(cursor->next != tail->next){
		cursor = cursor->next;
		if(cursor->num >= 10 ){
			buffer[idx++] = ((cursor->num)/10) + '0';
			buffer[idx++] = ((cursor->num)%10) + '0';
		}
		else{
			buffer[idx++] = cursor->num + '0';
		}
		buffer[idx++] = 32;
		buffer[idx++] = '-';
		buffer[idx++] = '>';
		buffer[idx++] = 32;
	}
	cursor = cursor->next;
	if(cursor->num >= 10 ){
            buffer[idx++] = ((cursor->num)/10) + '0';
            buffer[idx++] = ((cursor->num)%10) + '0';
    }
	else{
        buffer[idx++] = cursor->num + '0';
    }
	buffer[idx++] = 32;
	buffer[idx] = '\0';
	//printf("  %d        %d        %c \t %s\n",sequence+1, page_num, f, buffer);
	sprintf(save[j],"  %d           %d             %c \t    %s \n%c",sequence+1, page_num, f, buffer,'\0');
}
//sequence+1번째에 페이지 스트림에 대한 결과를 갖는 페이지 리스트의 상태와 페이지 폴트/히트 등을 저장한다.
//clock과 enhanced second clock에서 사용 
void save_result_clock(int sequence,int page_num, page* head, page* tail, int flag,int j, page* candidate,int flag2,int modify_bit){
	char buffer[400];
	char* what = flag == 1? "YES" : "NO";
	int idx=0;
	char f = 'N';
	if(flag == 1){
		f = 'Y';
	}
	page* cursor = head;
	while(cursor->next != tail->next){
		cursor = cursor->next;
		buffer[idx++] = '[';
		if(cursor->num >= 10 ){
			buffer[idx++] = ((cursor->num)/10) + '0';
			buffer[idx++] = ((cursor->num)%10) + '0';
		}
		else{
			buffer[idx++] = cursor->num + '0';
		}
		if(flag2 == 1){
			buffer[idx++] = '|';
			buffer[idx++] = cursor->R + '0';
			buffer[idx++] = '|';
			buffer[idx++] = cursor->W + '0';
		}
		if(cursor->R == 1 && flag2 == 0){
			buffer[idx++] = '|';
			buffer[idx++] = '*';
		}
		if(cursor == candidate){
			buffer[idx++] = '|';
			buffer[idx++] = '^';
		}
		buffer[idx++] = ']';
		buffer[idx++] = 32;
		buffer[idx++] = '-';
		buffer[idx++] = '>';
		buffer[idx++] = 32;
	}
	cursor = cursor->next;
	buffer[idx++] = '[';
	if(cursor->num >= 10 ){
            buffer[idx++] = ((cursor->num)/10) + '0';
            buffer[idx++] = ((cursor->num)%10) + '0';
    }
	else{
        buffer[idx++] = cursor->num + '0';
    }
	if(flag2 == 1){
		buffer[idx++] = '|';
        buffer[idx++] = cursor->R + '0'; 
		buffer[idx++] = '|';
        buffer[idx++] = cursor->W + '0';
	}
    if(cursor->R == 1 && flag2 == 0){
        buffer[idx++] = '|';
        buffer[idx++] = '*';
     }
     if(cursor == candidate){
         buffer[idx++] = '|';
		 buffer[idx++] = '^';
     }
    buffer[idx++] = ']';
	buffer[idx++] = 32;
	buffer[idx] = '\0';
	//printf("  %d        %d        %c \t %s\n",sequence+1, page_num, f, buffer);
	if(flag2 == 1){
		sprintf(save[j],"  %d           [%d|%d]             %c \t    %s \n%c",sequence+1, page_num,modify_bit, f, buffer,'\0');
	}
	else if(flag2 == 0){
		sprintf(save[j],"  %d           %d             %c \t    %s \n%c",sequence+1, page_num, f, buffer,'\0');
	}
}

//시뮬레이션 결과를 출력하고 파일에 저장한다.
void save_and_print(int idx){
	 for(int i=0; i<idx; i++){
         printf("%s",save[i]);
		 fprintf(fd,"%s",save[i]);
     }
}
