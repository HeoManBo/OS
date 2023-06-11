#include"m.h"

int make_data_string();
int read_file(FILE* in);

int* data_string;
int* read_string;
int* write_string;

void (*replace_function[])(int, int* , int, int*, int *) =
{[0] = NULL, [1] = OPTIMAL, [2] = FIFO, [3] = LIFO, [4] = LFU, [5] = LRU, [6] = CLOCK, [7] = ESC};

int main(void){
	char selection[100];
	char input[2];
	int page_frame_num;
	int data_selection;
	int algorithm[3], idx=0;
	memset(algorithm,0,sizeof(algorithm));
	//int data_string[] = {2,3,2,1,5,2,4,5,3,2,5,2};
	int data_size;
	char filename[1024];
	int i,j;
	//알고리즘 입력...
	idx=0;
	printf("=====                      Page Replacement 알고리즘을 테스트하는 프로그램입니다.                       =====\n");
	printf("=====                        선택하실 수 있는 알고리즘은 아래와 같습니다.                               =====\n");
	printf("=====            1. Optimal 2. FIFO 3. LIFO 4. LFU 5. LRU 6. SC 7. ESC 8. ALL 9. exit                   =====\n");
	printf("=====         각 번호는 띄어쓰기로 구분되며 최대 3개까지 입력을 받으며 9 입력시 종료됩니다.             =====\n");
	printf("=====             범위 외의 숫자는 무시되며 4개 이상의 입력은 3개까지의 번호만 유효합니다.              =====\n");
	printf("=====                      번호를 테스트할 알고리즘 번호를 입력하십시오 :                               =====\n");
	memset(selection,0,sizeof(selection));
	fgets(selection,100,stdin);
	strtok(selection,"\n");
	char* sp = strtok(selection," ");
	while(sp != NULL){
		int num = atoi(sp);
		if(num == 0){
			printf("잘못된 입력입니다. 프로그램을 종료합니다.\n");
			return 0;
		}
		else{
			if(idx >= 3){
				break;
			}
			if(num >=1 && num <= 8){
				algorithm[idx++] = num;
			}
			else if(num == 9){
				printf("프로그램을 종료합니다.\n");
				return 0;
			}
			else{
				printf("범위를 벗어나는  입력입니다 프로그램을 종료합니다...\n");
				return 0;
			}
		}
		sp = strtok(NULL," ");
	}
	for(int i=0, j=0; i<3; i++, j+=2){
		if(isdigit(selection[j])){
			if(selection[j] >= '1' && selection[j] <= '9'){
				algorithm[idx++] = selection[j] - '0';
			}
		else{
			printf("잘못된 입력입니다. 프로그램을 종료합니다.\n");
			exit(1);
			}
		}
	}
	//페이지 프레임 입력...
	printf("=====                         페이지 프레임의 수를 입력하십시오.                                        =====\n");
	printf("=====             페이지 프레임 수는 3~10까지의 수만 유효하며 그 외의 수와 입력은                       =====\n");
	printf("=====                        모두 프로그램 종료처리 됩니다. :                                           =====\n");
	char buf[100];
	fgets(buf,100,stdin);
	if(isdigit(buf[0])){
		if(buf[0] == '1' && buf[1] == '0'){
			page_frame_num = 10;
		}
		else{
			page_frame_num = buf[0] - '0';
			if(page_frame_num < 3){
				printf("잘못된 입력으로 프로그램이 종료됩니다.\n");
				exit(0);
			}
			if(isdigit(buf[1])){
				printf("잘못된 페이지 프레임 수 입력으로 프로그렘이 종료됩니다.\n");
				exit(0);
			}
		}
	}else{
		printf("잘못된 입력으로 프로그램을 종료합니다.\n");
		exit(0);
	}
	//페이지 스트림 입력 방식 선택..
	printf("=====                            데이터 입력 방식은                                                     =====\n");
	printf("=====            1. 프로그램 내부에서 임의로 생성 2. 파일에서 추출하기 중 입력하실 수 있습니다.         =====\n");
	printf("=====          파일의 형태는 개행 줄마다 페이지 번호 Read_bit write_bit 가 스페이스바 하나로 구분됩니다 =====\n");
	printf("=====                    페이지 참조 스트링은 (1~30)까지의 수로 가정합니다.                             =====\n");
	printf("=====                       또한 입력 스트링은 최대 500개를 인식합니다.                                 =====\n");
	printf("=====                데이터 입력 방식을 입력하십시오. 1,2 이외의 번호는 종료처리됩니다                  =====\n");
	memset(selection,0,sizeof(selection));
	fgets(selection,100,stdin);
	//printf("=====         결과를 저장할 파일 이름을 입력하세요. 동일한 파일의 이름이 있는 경우 덮어씌워집니다.      =====\n");
	//fgets(filename,100,stdin);
	//strtok(filename,"\n");
	//if((fd = fopen(filename, "w")) == NULL){
	//	printf("저장할 수 없는 파일 이름입니다. 프로그램을 종료합니다.\n");
	//	exit(0);
	//}
	if(selection[0] == '1'){
		data_size = make_data_string();
		printf("생성된 데이터는 아래와 같습니다.\n");
		printf("page_num Read Write\n");
		for(int i=0; i<data_size; i++){
			printf("%d         %d    %d\n",data_string[i],read_string[i],write_string[i]);
		}

	}
	else if(selection[0] == '2'){
		printf("데이터를 읽어들일 파일 이름을 입력하십시오, 현재 디렉토리에 없는 파일은 읽을 수 없습니다. : \n");
		char infile[100];
		fgets(infile,100,stdin);
		strtok(infile,"\n");
		FILE* in = NULL;
		if((in = fopen(infile, "r" )) == NULL){
			printf("데이터를 읽어들일 수 없는 파일입니다. 프로그램을 다시 실행하여주십시오.\n");
			exit(0);
		}
		else{
			data_size = read_file(in);
			printf("생성된 데이터는 아래와 같습니다.\n");
          printf("page_num Read Write\n");
          for(int i=0; i<data_size; i++){
              printf("%d         %d    %d\n",data_string[i],read_string[i],write_string[i]);
          }
		}
		fclose(in);
	}
	printf("=====         결과를 저장할 파일 이름을 입력하세요. 동일한 파일의 이름이 있는 경우 덮어씌워집니다.      =====\n");
    fgets(filename,100,stdin);
    strtok(filename,"\n");
	
	if((fd = fopen(filename, "w")) == NULL){
      printf("저장할 수 없는 파일 이름입니다. 프로그램을 종료합니다.\n");
      exit(0);
    }
	if(algorithm[0] == 8 || algorithm[1] == 8 || algorithm[2] == 8){
		for(int i=1; i<=7; i++){
	         replace_function[i](page_frame_num, data_string, data_size, read_string, write_string);
        }
     }
     else{
        for(int i=0; i<idx; i++){
              replace_function[algorithm[i]](page_frame_num, data_string, data_size, read_string, write_string);
         }
    }
	fclose(fd);
	return 0;
}

int read_file(FILE* in){
	fseek(in, 0, SEEK_SET);
	char buffer[1024];
	int cnt = 0;;
	int d[1024];
	int r[1024];
	int w[1024];
	memset(buffer,0,sizeof(buffer));
	while(fgets(buffer,1024,in) != NULL){
		printf("%s",buffer);
		strtok(buffer,"\n");
		char* c = strtok(buffer," ");
		d[cnt] = atoi(c);
		c = strtok(NULL, " ");
		r[cnt] = atoi(c);
		c = strtok(NULL, " ");
		w[cnt] = atoi(c);
		cnt++;
		if(cnt == 500){
			break;
		}
		memset(buffer,0,sizeof(buffer));
	}
	int* tmp = (int *)malloc(sizeof(int) * cnt);
	int* tmp_read = (int *)malloc(sizeof(int) * cnt);
	int* tmp_write = (int *)malloc(sizeof(int) * cnt);
	for(int i=0; i<cnt; i++){
		tmp[i] = d[i];
		tmp_read[i] = r[i];
		tmp_write[i] = w[i];
	}

	//전역 변수로 옮기기
	data_string = tmp;
	read_string = tmp_read;
	write_string = tmp_write;
	return cnt;
}


int make_data_string(){
	//랜덤하게 생성
	int tmp[1024];
	int tmp_read[1024];
	int tmp_write[1024];
	int cnt = 0;
	srand((unsigned int)time(NULL));
	int num = 0;
	//데이터 스트링을 얻어냄
	while(cnt<500){
		tmp[cnt] = (rand()%30)+1;
		tmp_read[cnt] = (rand()%2);
		tmp_write[cnt] = (rand()%2);
		cnt++;
	 }
	 //printf("생성된 데이터는 아래와 같습니다.\n");
     //     for(int i=0; i<cnt; i++){
     //          printf("%d : %d %d %d\n",i, tmp[i],tmp_read[i],tmp_write[i]);
     //}

	int* a = (int *)malloc(sizeof(int) * cnt);
    int* b = (int *)malloc(sizeof(int) * cnt);
    int* c = (int *)malloc(sizeof(int) * cnt);
	for(int i=0; i<cnt; i++){
		a[i] = tmp[i];
		b[i] = tmp_read[i];
		c[i] = tmp_write[i];
	}
	/*
	else if(mode == 2){
		int fd;
		char file[4096];
		while(1){
			printf("오픈할 파일의 이름을 입력하십시오 : ");
			scanf("%s", file);
			if((fd = open(file, O_RDONLY)) == -1){
				printf("열 수 없는 파일 이름입니다. 프로그램을 종료합니다.\n");
				exit(1);
			}
			else{
				break;
			}
		}
		memset(file, 0, sizeof(file));
		if(read(fd, file, sizeof(file)) == -1){
			printf("읽을 수 없는 파일 입니다... 다시 프로그램을 실행하십시오.\n");
			exit(1);
		}
		char* split = strtok(file, " ");
		while(split != NULL){
			int num = atoi(split);
			tmp[idx++] = num;
			split = strtok(NULL, " ");
		}
		printf("파일에서 데이터 스트링 읽기 성공!\n");
		close(fd);
	}
	*/
	data_string = a;
	read_string = b;
	write_string = c;
	return cnt;
}














