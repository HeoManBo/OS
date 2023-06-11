#include "types.h"
#include "stat.h"
#include "user.h"

#define PNUM 5
#define PRINT_CYCLE 10000000
#define TOTAL_COUNTER 500000000 

int weight = 1;

void sdebug_func(void)
{
	int n, pid[PNUM];

    printf(1, "start sdebug command\n");

    for(n = 0; n < PNUM; n++){
        pid[n] = fork();
        if(pid < 0){
            printf(1,"fork error!\n");
            exit();
        }
        else if(pid[n]==0)
        {
			//fork직후 바로 시간을 측정함.
            int start_ticks = uptime();
			//counter 변수와 print_counter 변수를 선언하고 weightset 시스템 콜을 호출하여 
			//생성된 프로세스의 wegiht 값을 sdebug의 weight으로 설정하고 weightset 시스템 콜의 리턴값은 설정하려는 weight값임.
			//또한 Pid를 출력하기 위해 getpid로 fork된 프로세스의 Pid를 얻어온다. 
            int end_ticks,  counter=0;
            int child_weight = weightset(weight);
            int child_pid = getpid();
			//TOTAL_COUNTER에 도달할 때까지 counter 값을 증가함.
            while(counter <= TOTAL_COUNTER)
            {
                counter++;
				//counter가 print_cycle이 되면 프로세스 정보를 출력함
				//이때 uptime으로 종료되는 시간을 측정한다 (ticks)
				//uptime는 ticks 값으로 반환되는데 ticks 값은 10ms마다 1씩 증가하므로 
				//종료 tick에서 시작 tick을 빼고 10를 곱해주면 프로세스 정보를 출력하기까지의 시간이 된다. 
                //if(print_cycle == 0){
					if(counter == PRINT_CYCLE){
                        end_ticks = uptime();
                        printf(1,"PID: %d, WEIGHT: %d, ",child_pid, child_weight);
						printf(1,"TIMES : %dms\n", (end_ticks-start_ticks)*10);
					}
            }
			//counter가 0, 즉 주어진 시간을 모두 소요하면 프로세스를 종료시킴
            printf(1,"PID %d terminated \n", child_pid);
            exit();
        }
        //sdebug프로세스는 weight를 증가시키면서 계속적으로 fork함
        else{
            weight++;
        }
    }
    //sdebug프로세스는 모든 자식 프로세스가 fork된 만큼 exit을 기다림. <0 인 경우 Wait 시스템콜 >함수 실행 중 자식 프로세스가 없는 경우임
    for(; n>0; n--){
        if(wait() < 0){
            printf(1," wait stooped early\n");
            exit();
        }
    }
	printf(1,"end of sdebug command\n");
}

int main(int argc, char* argv[])
{
    if(argc <= 1){
        sdebug_func();
    }
    else
    {
        weight = atoi(argv[1]);
		if(weight == 0){
        	printf(1,"input weight is 0 !! not invaild number.. \n");
			exit();
		}
        sdebug_func();
    }
    exit();
}

