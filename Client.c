#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdio_ext.h>
#define PORTNUM 9000

void get_board(int sd); // 현재 보드의 상태를 출력해주는 함수
int send_fix_board(int sd, char dol); // 현재 보드의 원하는 위치에 돌을 놓는 함수
void rotate_board(int sd); // 현재 보드에 원하는 사분면에 원하는 방향으로 회전시키는 함수
int check_pentago(int sd); // 게임이 끝났는지 확인하는 함수
int end_turn(int sd);

int main(void) {
    signal(SIGINT, SIG_IGN); // 불계승/패 막기위한 시그널 이그노어
    int is_end = 0;					// 게임이 끝난것을 확인하는 변수,
    int sd;	// 소켓파일기술자 위한 변수
    struct sockaddr_in sin, cli; // 소켓통신 위한 변수
    time_t start_time, end_time;
    int play_time;

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {	// 소켓 생성하기
        perror("socket");
        exit(1);
    }

    memset((char*)&sin, '\0', sizeof(int));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORTNUM);
//    sin.sin_addr.s_addr = inet_addr("222.236.11.238");	// 채선이 서버 연결시
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");	//	로컬 서버 연결시

    if(connect(sd, (struct sockaddr *)&sin, sizeof(sin))) { // 서버에 접속 요청
        perror("connect");
        exit(1);
    }

    start_time = time(NULL); // 게임 시작 시각

    while (is_end == 0) {	// 플레이 하는 일련의 과정
        get_board(sd);	// 보드를 받아온다

        while (send_fix_board(sd, '0') != 0);	// 돌 없는곳에 돌 두기
        get_board(sd);	// 보드를 받아온다

        if(is_end = check_pentago(sd))
            break;	// 게임이 끝났다면, 게임을 끝낸다.
        else
            is_end = end_turn(sd);	// 턴을 넘긴다.
    }
    end_time = time(NULL); // 게임 끝나는 시각
    play_time = end_time - start_time; // 총 게임 시간
    printf("플레이 시간 : %02d:%02d:%02d\n", (play_time) / 3600, (play_time / 60) % 60, play_time % 60);

    if (is_end == 2) printf("패배\n"); // 승리, 패배를 보여준다.
    else printf("승리\n");
    close(sd); // 소켓 닫기
    return 0;
}

// 보드 상태를 받아오는 함수, buf에 문자열 형태로 받아오고,
// 적절히 잘라서 출력해준다.

void get_board(int sd) {
    char buf[365];
    if(send(sd, "1", strlen("1")+1, 0) == -1) {
        perror("send");
        exit(1);
    }

    system("clear");

    if(recv(sd, buf, sizeof(buf), 0) == -1) {
        perror("recv");
        exit(1);
    }

    for(int i = 0; i < 14; i++) {
        for(int j = 0; j < 26; j++) {
            fflush(stdout);
            printf("%c", buf[26*i + j]);
        }
        printf("\n");
    }
}


// 보드에 돌을 놓는 함수
// row, col 에다가 dol을 놓는다.
int send_fix_board(int sd, char dol) {
    char x, y;
    char buf[128];
    char str[4];
    char rcv[4];
    if(send(sd, "2", strlen("2")+1, 0) == -1) {
        perror("send");
        exit(1);
    }

    if(recv(sd, buf, sizeof(buf), 0) == -1) {
        perror("recv");
        exit(1);
    }
    fflush(stdout);
    printf("좌표 (ex, A1) :\n");

    while(1) {
        // 플레이어가 원하는 좌표를 입력받는다.
        __fpurge(stdin);
        x = getc(stdin);
        y = getc(stdin);
        __fpurge(stdin);
        if (( (x >= 'A' && x <= 'F') || (x >= 'a' && x <= 'f')) && y >= '1' && y <= '6') {
            if (x >= 'a' && x <= 'f') x -= 32; // 'a' - 'A' = 32 소문자를 대문자로
            break;
            printf("잘못 입력하셨습니다. 다시 입력하세요 :");
        } else {
            printf("잘못 입력하셨습니다. 다시 입력하세요 :");
        }
    }
    str[0] = x;
    str[1] = y;
    str[2] = dol;
    str[3] = '\0';

    if(send(sd, str, strlen(str)+1, 0) == -1) {
        perror("send");
        exit(1);
    }
    if(recv(sd, rcv, sizeof(rcv), 0) == -1) {
        perror("recv");
        exit(1);
    }
    if (strcmp(rcv, "0") == 0)	return 0;
    else return -1;
}


// 게임이 끝났는지 확인하는 함수
int check_pentago(int sd) {
    char buf[2];
    if(send(sd, "4", strlen("4")+1, 0) == -1) {
        perror("send");
        exit(1);
    }

    if(recv(sd, buf, sizeof(buf), 0) == -1) {
        perror("recv");
        exit(1);
    }

    if(strcmp(buf, "0") == 0) return 0;
    else return 1;
}
int end_turn(int sd) {
    char buf[2];
    if(send(sd, "5", strlen("5")+1, 0) == -1) {
        perror("send");
        exit(1);
    }

    if(recv(sd, buf, sizeof(buf), 0) == -1) {
        perror("recv");
        exit(1);
    }
    // 0이 반환되면 게임이 안끝남
    if(strcmp(buf, "0") == 0) return 0;
    else return 2;
}
