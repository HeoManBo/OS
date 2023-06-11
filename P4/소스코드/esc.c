#include"m.h"

page* candidate_esc = NULL;
page* esc_head = NULL;
page* esc_tail = NULL;

int search_frame_esc(int page_num,int write)
{
    page* cursor = esc_head;
    if(cursor->next == NULL){
        return -1;
    }
    while(cursor->next != esc_tail->next){
        cursor = cursor->next;
        if(cursor->num == page_num){
			if(cursor->R == 0){
				cursor->R = 1;
			}
			cursor->W = write;
            return 1;
        }
    }
	//esc_tail값 비교, LRU 수정 14.23.33
	cursor = cursor->next;
	if(cursor->num == page_num){
		if(cursor->R == 0){
			cursor->R = 1;
		}
		cursor->W = write;
		return 1;
	}
	////////////////////////////////
    //while문에서 끊나지 않았다면 해당 페이지 번호가 페이지 프레임에 존재하지 않으므로
    //-1를 리턴한다
    return -1;
}


//ESC 알고리즘시 esc_tail에 추가 
void addPage_ESC(int page_num, int write,int frame_size){
    page* link = make_page_frame(page_num);
	link -> W = write;
    //페이지 프레임이 비어있는 경우 (맨 처음 초기상태에 해당)
    if(esc_head->next ==  NULL){
        esc_head->next = link;
        esc_tail->next = link;
    }
    //페이지 프레임이 비어있지 않은 경우
    //비어있는 맨 끝 페이지 프레임에 추가한다.
    else{
        page* cursor = esc_tail->next;
        esc_tail->next->next = link;
        esc_tail->next = cursor->next;
    }
    page_frame_size++;
	//환영 버퍼식으로 교체 후보를 이동시킴
    if(candidate_esc == NULL){
        candidate_esc = link->next;
        //만약 페이지 프레임이 꽉찼다면 
        //교체 후보는 맨 처음 헤드를 가리키게 만듬 
        //해당 함수는 페이지 프레임이 꽉 차지 않았을 때만 작동
        if(page_frame_size == frame_size){
            candidate_esc = esc_head->next;
        }
    }
}

//페이지 폴트시 교체될 페이지 프레임을 찾는다.
page* find(){
	page* ret = NULL;
	//교체 대상을 찾을 때까지 반복한다.
	while(1){
		int flag = 0;
		int turn_around = 0; //한 바퀴 돌았는지 확인
	    //동작과정 1 : read_bit == 0 && write_bit == 0인 페이지 프레임을 candidate가 가리키고 있는 프레임부터 
		//찾아낸다.
		page* cursor = candidate_esc;
		while(turn_around == 0 || cursor != candidate_esc){
			//찾았으면
			if(cursor->R == 0 && cursor->W == 0){
				//해당 대상이 교체가 됨.
				ret = cursor;
				flag = 1;
				break;
			}
			//교체 후보가 처음에 esc_tail이 가리키던 것이라면
			//esc_head로 이동
			if(cursor == esc_tail->next){
				cursor = esc_head->next;
				turn_around = 1; //순환 하였는지 체크
			}
			//아니면 다음 프레임으로 이동
			else{
				cursor = cursor->next;
			}
		}
		//만약 찾았다면
		if(flag){
			//candidate 를 교체 대상 다음 프레임을 가리키게 만듬.
			//만약 교체 대상이 esc_tail이 가리키는 것이였다면
			if(ret == esc_tail->next){
				//다음 교체 후보는 esc_head가 가리키는 것이 됨.
				candidate_esc = esc_head->next;
			}
			else{
				candidate_esc = cursor->next;
			}
			return ret;
		}
		//Read_bit가 0이고 write_bit 0인 페이지 프레임을 못찾았다면
		//동작과정2 Read_bit가 0이고 write_bit 1인 페이지 프레임을 찾는다. 
		else{
			//동작과정1에서 찾지 못했다면
			//cursor는 다시 candidate를 가리키고 있을 것이다. 
			turn_around = 0;
			flag = 0;
			while(turn_around == 0 || cursor != candidate_esc){
				//찾았다면
				if(cursor->R == 0 && cursor->W == 1){
					ret = cursor;
					flag = 1;
					break;
				}
				//이 과정에서 R=1인 페이지 프레임을 R=0의 값으로 바꾼다.
				else{
					cursor->R = 0;
				}
			//교체 후보가 처음에 esc_tail이 가리키던 것이라면
              //esc_head로 이동
              if(cursor == esc_tail->next){
                  cursor = esc_head->next;
                  turn_around = 1; //순환 하였는지 체크
              }   
              //아니면 다음 프레임으로 이동
             else{
                  cursor = cursor->next;
              }   
			}
			//만약에 찾았다면
			if(flag){
              //candidate 를 교체 대상 다음 프레임을 가리키게 만듬.
              //만약 교체 대상이 esc_tail이 가리키는 것이였다면
              if(ret == esc_tail->next){
                  //다음 교체 후보는 esc_head가 가리키는 것이 됨.
                  candidate_esc = esc_head->next;
              }
              else{
                  candidate_esc = cursor->next;
              }
              return ret;
          }
		}
		//동작 과정 2에서도 못찾앗다면
		//다시 동작 과정1을 다시 탐색함. 궁극적으로 교체 대상 프레임을 찾아냄.
	}
}

void insert_page_esc(page* ret, int page_num, int write_bit)
{
	page* cursor = esc_head;
	//교체 대상 페이지 프레임을 가리키는 
	//페이지 프레임으로 이동
	while(cursor->next != ret){
		cursor = cursor->next;
	}
	//이번에 참조될 페이지 프레임 생성 
	page* link = make_page_frame(page_num);
	link->W = write_bit;
	
	//이제 교체할 페이지 프레임 위치에 만든 페이지를 넣는다.
	if(cursor->next == esc_tail->next){
		cursor->next = link;
		esc_tail->next = cursor->next;
	}
	else{
		cursor->next = link;
		link->next = ret->next;
	}
	free(ret);
}

/*
   enhnaced second chance algorithm
   동작과정 1. R-bit == 0 이고 동시에 W-bit == 0 인 페이지 프레임을 찾는다.
   동작과정 2. 1번에서 찾지 못했다면 R-bit == 0이고 W-bit이 1인 페이지 프레임을 찾는다
   이 과정에서 R-bit = 1인 페이지 프레임은 R-bit 값을 0으로 바꾼다
   만약 동작과정 2에서도 찾지 못한다면 다시 동작과정 1번으로 돌아가 찾을 때까지 1,2번을 반복한다.
   만약 동작과정 1에서 R이 0이고 W또 0인 페이지 프레임이 여러 개가 있다면
   교체 후보 포인터가 가리키는 것에서부터 시작하여 먼저 발견되는 것을 교체한다.
   동작과정 2에서도 마찬가지로 적용된다.
*/
void ESC(int frame_size, int* page_stream, int stream_size, int* read_bit, int* write_bit){
	init_test(&esc_head,&esc_tail);
	buf_init();
	int idx=4;
	sprintf(save[0],"------------------- Enhanced second chance 알고리즘 테스트---------------------------------------\n%c",'\0');
	sprintf(save[1],"해당 알고리즘의 페이지 리스트의 각 페이지 프레임은  [페이지 번호|use_bit|modify_bit|^(교체 후보)] 형태를 가집니다.\n%c",'\0');
	sprintf(save[2],"Sequence||[page_num|modify_bit]||page_fault||page_frame_list[page_num|use_bit|modify_bit|^(candidate)\n%c",'\0');
	sprintf(save[3],"--------------------------------------------------------------------------------\n%c",'\0');
	for(int i=0; i<stream_size; i++){
		if(search_frame_esc(page_stream[i], write_bit[i])== 1){
			paging_success++;
			save_result_clock(i,page_stream[i],esc_head,esc_tail,0,idx++,candidate_esc,1,write_bit[i]);
		}
		else{
			if(page_frame_size < frame_size){
				addPage_ESC(page_stream[i], write_bit[i], frame_size);
			}
			else{
				page* ret = find(); //교체 대상 페이지 프레임을 찾는다.
				insert_page_esc(ret, page_stream[i], write_bit[i]);
			}
			save_result_clock(i,page_stream[i],esc_head,esc_tail,1,idx++,candidate_esc,1,write_bit[i]);
			page_fault++;
		}
	}
	sprintf(save[idx++],"---------------------------------------------------------------------------------------\n%c",'\0');
	sprintf(save[idx++],"============= Enhanced second chance  페이징 알고리즘 결과는 다음과 같습니다.=============\n%c",'\0');
    sprintf(save[idx++],"page fault 횟수는 : %d\n%c", page_fault,'\0');
    sprintf(save[idx++],"Page hit  횟수는 : %d\n%c", paging_success,'\0');
	sprintf(save[idx++],"page fault rate 는 : %.0f%%\n%c", ((double)(page_fault)/(double)stream_size)*100,'\0');
    sprintf(save[idx++],"hit rate는 : %.0f%%입니다.\n%c", ((double)paging_success/(double)stream_size)*100,'\0');
	sprintf(save[idx++],"\n\n\n%c",'\0');
	save_and_print(idx);
	free_memory(esc_head,esc_tail);
	candidate_esc = NULL;
}

/*
int main(void){
    int page_stream[]  ={2,3,2,1,5,2,4,5,3,2,5,2};
	int write_bit[] = {0,1,1,1,0,0,1,0,1,1,0,1};
    int frame_size = 3;
    int stream_size = sizeof(page_stream)/sizeof(int);
    ESC(frame_size, page_stream, stream_size, write_bit);
    return 0;
}
*/
