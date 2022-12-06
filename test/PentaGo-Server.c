#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
//#include <stdio_ext.h>
#define PORTNUM 9000

char arr[15][15];	// 펜타고 보드 배열
int fd;	// 파일 디스크립터 , 기보 저장을 위함

/* 클라이언트 플레이 함수 */
void send_board(int ns);	// 현재 보드를 문자열로 보내는 함수
void fix_board(int ns);	// 현재 보드의 원하는 위치에 돌을 놓는 함수
void rotate_board(int ns);	// 현재 보드에 원하는 사분면에 원하는 방향으로 회전시키는 함수
int is_finish(int ns);

/* 서버 플레이 함수 */
void print_board(); // 현재 보드의 상태를 출력해주는 함수
int my_turn(int ns, char dol);
void init_board();	// 보드를 깨끗한 상태로 초기화 하는 함수
int my_fix_board(int col, int row, char dol); // 현재 보드의 원하는 위치에 돌을 놓는 함수
int check_pentago(); // 게임이 끝났는지 확인하는 함수

int main(void) {
    signal(SIGINT, SIG_IGN);
    char type[2]; // 수행할 서비스의 종류를 저장하기위한 변수
    char file_name[128];
    struct sockaddr_in sin, cli;
    int sd, ns, clientlen = sizeof(cli);
    struct tm *tm;
    time_t start_time = time(NULL), end_time;
    int play_time;
    int is_end = 0;	// 게임이 끝난것을 확인하는 변수,
    // 0이면 게임이 끝나지 않은 상태,
    // 1이면 흑돌 win, 2이면, 백돌 win

// 기보를 저장하기 위해 파일 디스크립터 지정
    tm = localtime(&start_time);
    sprintf(file_name, "./Pentagologs/%d%02d%02d_%02d_%02d_%02d.txt", (int)tm->tm_year + 1900,
            (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour, (int)tm->tm_min, (int)tm->tm_sec);
    mkdir("./Pentagologs", 0777);
    fd = open(file_name, O_CREAT | O_WRONLY | O_APPEND ,0664);
    if (fd == -1) {
        perror("Creat");
        exit(1);
    }

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // 소켓 생성하기
        perror("socket");
        exit(1);
    }

    memset((char*)&sin, '\0', sizeof(int));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORTNUM);
    //sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr *)&sin, sizeof(sin))) {
        perror("bind");
        exit(1);
    }

    system("clear");
    printf("도전자를 기다리는중...\n"); // 소켓 바인드 되기 전까지 대기

    if(listen(sd, 1)) {
        perror("listen");
        exit(1);
    }

    if ((ns = accept(sd, (struct sockaddr *)&cli, &clientlen)) == -1) {
        perror("accept");
        exit(1);
    }

    system("clear");
    printf("게임이 시작됩니다.\n");
    sleep(1);

    init_board(); // 게임을 시작하기 전 보드의 상태를 초기화 한다.

    start_time = time(NULL); // 게임 시작 시각
    while(is_end == 0) { // 플레이 하는 일련의 과정
        system("clear");
        print_board(); // 보드를 출력함

        if (recv(ns, type, sizeof(type), 0) == -1) { // 수행할 서비스 종류 받는다
            perror("recv");
            exit(1);
        }

        if( strcmp(type, "1") == 0) { // send_board를 실행
            send_board(ns);
        } else if(strcmp(type, "2") == 0) { // fix_board를 실행
            fix_board(ns);
        } else if (strcmp(type, "3") == 0) { // rotate_board를 실행
            rotate_board(ns);

        } else if (strcmp(type, "4") == 0) { // rotate_board를 실행
            is_end = is_finish(ns);
            if(is_end != 0) printf("패배\n");

        } else if (strcmp(type, "5") == 0) { // 클라이언트측 턴이 끝나고 서버 측 턱
            is_end = my_turn(ns, 'X');
            if(is_end != 0) printf("승리\n");
        }
    }

    end_time = time(NULL); // 게임 끝나는 시각
    play_time = end_time - start_time;	// 총 게임 시간
    printf("플레이 시간 : %02d:%02d:%02d\n", (play_time) / 3600, (play_time / 60) % 60, play_time % 60);

    close(ns); // 소켓을 닫음
    close(sd);
    close(fd); // 파일 디스크립터 닫음
    return 0;
}

// 보드(판)을 ' '로 초기화 해주는 함수
void init_board() {
    for (int i = 0; i < 6; i++ )
        for(int j = 0; j < 6; j++ )
            arr[i][j] = ' ';
}

// 보드의 현재 상태를 출력해주는 함수
// 가로, 세로축에 A~F, 1~6 을 추가로 출력해준다.

void print_board() {
    printf(" │ A │ B │ C │ D │ E │ F │\n");
    printf("─┼───┼───┼───┼───┼───┼───┼\n");
    int i=0, j=0;
    for(int l = 0; l < 6; l++) {
        printf("%d│", l+1);
        for (int m = 0; m < 6; m++) {
            printf(" %c │", arr[l][m]);
        }
        printf("\n─┼───┼───┼───┼───┼───┼───┼\n");
    }
}

void send_board(int ns) {
    char buf[365];
    memset(buf, 0, sizeof(buf));
    int i, j;
    print_board();
    for(i = 0; i < 14; i++) {
        for(j = 0; j < 26; j++) {
            if ( i == 0 ) {
                if ( j % 2 == 0) buf[26*i+j] = ' ';
                else if ( j % 4 == 1) buf[26*i+j] = '|';
                else if ( j % 4 == 3) buf[26*i+j] = j/4 + 'A';
            }
            else if (i % 2 == 1) {
                if ( j % 4 == 1) buf[26*i+j] = '+';
                else buf[26*i+j] = '-';
            }
            else {
                if (j==0) buf[26*i+j] = i/2+'0';
                else if ( j % 4 == 1) buf[26*i+j] = '|';
                else if ( j % 2 == 0) buf[26*i+j] = ' ';
                else buf[26*i+j] = arr[i/2-1][j/4];
            }
        }
    }
    buf[364] = '\0';
    if(send(ns, buf, strlen(buf) + 1, 0) == -1) {
        perror("send");
        exit(1);
    }
    buf[364] = '\n';
    buf[365] = '\0';
    write(fd, buf, 365); // 로그로 쓴다.
}

// 보드에 돌을 놓는 함수
// row, col 에다가 dol을 놓는다.
void fix_board(int ns) {
    char rowcol[4];
    int row, col;
    if(send(ns, "OK", strlen("OK") + 1, 0) == -1) { // 의미 없음
        perror("send");
        exit(1);
    }

    if (recv(ns, rowcol, sizeof(rowcol), 0) == -1) {
        perror("recv");
        exit(1);
    }

    row = rowcol[1] - '0' - 1;
    col = rowcol[0] - 'A';

    printf("%d %d\n", row, col);
    if (arr[row][col] == ' ') {
        arr[row][col] = rowcol[2];
        if(send(ns, "0", strlen("0") + 1, 0) == -1) {
            perror("send");
            exit(1);
        }
    } else {
        if(send(ns, "-1", strlen("-1") + 1, 0) == -1) {
            perror("send");
            exit(1);
        }
    }
}

// 보드의 한 사분면을 회전하는 함수, is_clock_wise 가 y이거나 Y이면 시계방향 회전이다.
//  화면 출력 기준으로

//   1 2
//   3 4

//	사분면이다.
void rotate_board(int ns) {
    int row, col;
    char qc[3];
    if(send(ns, "1", strlen("1") + 1, 0) == -1) { // 의미 없음
        perror("send");
        exit(1);
    }

    if (recv(ns, qc, sizeof(qc), 0) == -1) {
        perror("recv");
        exit(1);
    }
    if (qc[0] == '1') {
        row = 0;
        col = 0;
    } else if (qc[0] == '2') {
        row = 0;
        col = 3;
    } else if (qc[0] == '3') {
        row = 3;
        col = 0;
    } else if (qc[0] == '4') {
        row = 3;
        col = 3;
    }
    for (int i = 0; i < qc[1]-'0'; i++) {
        char tmp = arr[0+row][0+col];
        arr[0+row][0+col] = arr[2+row][0+col];
        arr[2+row][0+col] = arr[2+row][2+col];
        arr[2+row][2+col] = arr[0+row][2+col];
        arr[0+row][2+col] = tmp;
        tmp = arr[0+row][1+col];
        arr[0+row][1+col] = arr[1+row][0+col];
        arr[1+row][0+col] = arr[2+row][1+col];
        arr[2+row][1+col] = arr[1+row][2+col];
        arr[1+row][2+col] = tmp;
    }

    if(send(ns, "1", strlen("1") + 1, 0) == -1) { // 의미 없음
        perror("send");
        exit(1);
    }

}

// 클라이언트가 턴을 넘기면, 서버가 게임을 진행하는 함수
// 순서는 클라이언트 코드랑 같다.
// 놀 놓기 -> 돌리기 -> 검증 -> 결과 반환

int my_turn(int ns, char dol) {
    char str[2];
    int ret = check_pentago();
    char x, y, quad, c;

    printf("좌표 (ex, A1) : ");
    while(1) {
        fpurge(stdin);
        x = getc(stdin);
        y = getc(stdin);
        fpurge(stdin);
        if (( (x >= 'A' && x <= 'F') || (x >= 'a' && x <= 'f'))
            && y >= '1' && y <= '6') break;
        printf("잘못 입력하셨습니다. 다시 입력하세요 :");
    }
    if (x >= 'a' && x <= 'f') x -= 32; // 'a' - 'A' = 32 소문자를 대문자로
    while (my_fix_board(x-'A', y-'0' - 1, dol)!=0) {
        printf("이미 두신곳에 두셨습니다. 다시 두세요 :");
        while(1) {
            fpurge(stdin);
            x = getc(stdin);
            y = getc(stdin);
            fpurge(stdin);
            if (( (x >= 'A' && x <= 'F') || (x >= 'a' && x <= 'f')) && y >= '1' && y <= '6') break;
            printf("잘못 입력하셨습니다. 다시 입력하세요 :");
        }
    }

    system("clear");
    print_board();
    if (ret = check_pentago()) {
        str[0] = ret + '0';
        str[1] = '\0';
        if(send(ns, str, strlen(str) + 1, 0) == -1) {
            perror("send");
        }
        if( ret == 1 ) {
            return 1; // 게임 끝
        }
        else return 0; // 게임 계속 진행
    };

//    printf("┌───┬───┐\n");
//    printf("│ 1 │ 2 │\n");
//    printf("├───┼───┤\n");
//    printf("│ 3 │ 4 │\n");
//    printf("└───┴───┘\n");
//    printf("회전할 사분면\n");
//
//    while (1) {
//        fpurge(stdin);
//        quad = getc(stdin);
//        fpurge(stdin);
//        if (quad >= '1' && quad <= '4') break;
//        printf("잘못 입력하셨습니다. 다시 입력하세요 :");
//    }
//
//    printf("시계방향?(y/n)\n");
//    while (1) {
//        fpurge(stdin);
//        c = getc(stdin);
//        fpurge(stdin);
//        if (c == 'y' || c == 'Y' || c == 'n' || c == 'N') break;
//        printf("잘못 입력하셨습니다. 다시 입력하세요 :");
//    }
//    if (c == 'y' || c == 'Y') my_rotate_board(quad -'0', 1);
//    else my_rotate_board(quad - '0', 3);

    system("clear");
    print_board();
    ret = check_pentago();

    str[0] = ret + '0';
    str[1] = '\0';
    if(send(ns, str, strlen(str) + 1, 0) == -1) {
        perror("send");
    }
    if( ret == 1 ) {
        return 1; // 게임 끝
    }
    else return 0; // 게임 계속 진행
}

// col, row 위치에 dol을 놓는다.
int my_fix_board(int col, int row, char dol) {

    if (arr[row][col] == ' ') {
        arr[row][col] = dol;
        return 0;
    } else return -1;
}


// 게임이 끝나면 1을 반환한다.
int is_finish(int ns) {
    char str[2];
    int ret = check_pentago();
    str[0] = ret + '0';
    str[1] = '\0';
    if(send(ns, str, strlen(str) + 1, 0) == -1) {
        perror("send");
    }
    if( ret == 1 ) {
        return 1; // 게임 끝
    }
    else return 0; // 게임 계속 진행
}


//5개의 돌이 이어졌는지 체크한다.
int check_pentago() {
    int count = 0;
    int isNull = 0;
    //가로
    for(int i = 0 ; i < 6 ; i++) {
        for(int j = 1 ; j < 5 ; j++) {
            if(arr[i][j] == ' ') {
                isNull = 1;
                break;
            }
        }
        if((isNull != 1 && arr[i][0] != ' ') || (isNull != 1 && arr[i][5] != ' ')) {
            for(int j = 0 ; j < 6 ; j += 5) {
                for(int k = 1 ; k < 5 ; k++) {
                    if(arr[i][j] == arr[i][k]) {
                        count++;
                    }
                }
                if(count == 4) {
                    return 1;
                }
                else {
                    count = 0 ;
                }
            }
        }
        isNull = 0;
    }

//세로
    count = 0;
    isNull = 0;
    for(int i = 0 ; i < 6 ; i++) {
        for(int j = 1 ; j < 5 ; j++) {
            if(arr[j][i] == ' ') {
                isNull = 1;
                break;
            }
        }
        if((isNull != 1 && arr[0][i] != ' ') || (isNull != 1 && arr[5][i] != ' ')) {
            for(int j = 0 ; j < 6 ; j += 5 ) {
                for(int k = 1 ; k < 5 ; k++) {
                    if(arr[j][i] == arr[k][i]) {
                        count++;
                    }
                }
                if(count == 4) {
                    return 1;
                }
                else {
                    count = 0;
                }
            }
        }
        isNull = 0;
    }

    //왼쪽 위 오른쪽 아래 1
    count = 0;
    isNull = 0;
    {
        for(int i = 1 ; i < 5 ; i++) {
            if(arr[i][i] == ' ') {
                isNull = 1;
                break;
            }
        }
        if((isNull != 1 && arr[0][0] != ' ') || (isNull != 1 && arr[5][5] != ' ')) {
            for(int i = 0 ; i < 6 ; i += 5) {
                for(int j = 1 ; j < 5 ; j++) {
                    if(arr[i][i] == arr[j][j]) {
                        count++;
                    }
                }
                if(count == 4) {
                    return 1;
                }
                else {
                    count = 0 ;
                }
            }
        }
        isNull = 0;
    }
    //왼쪽 아래 오른쪽 위 2
    count = 0;
    isNull = 0;
    {
        for(int i = 1, j = 4 ; i < 5 ; i++, j--) {
            if(arr[i][j] == ' ') {
                isNull = 1;
                break;
            }
        }
        if((isNull != 1 && arr[0][5] != ' ') || (isNull != 1 && arr[5][0] != ' ')) {
            for(int i = 0 , j = 5 ; i < 6 ; i += 5, j -= 5) {
                for(int k = 1, l = 4 ; k < 5 ; k++, l--) {
                    if(arr[i][j] == arr[k][l]) {
                        count++;
                    }
                }
                if(count == 4) {
                    return 1;
                }
                else {
                    count = 0;
                }
            }
        }
        isNull = 0;
    }
    //왼쪽 위 오른쪽 아래 4개 위 3
    count = 0;
    if(arr[0][1] != ' ') {
        for(int i = 1 ; i < 5 ; i++) {
            if(arr[0][1] == arr[i][i+1]) {
                count++;
            }
        }
        if(count == 4) {
            return 1;
        }
        else {
            count = 0 ;
        }
    }
    //왼쪽 위 오른쪽 아래 4개 아래 4
    count = 0;
    if(arr[1][0] != ' ') {
        for(int i = 1 ; i < 5 ; i++) {
            if(arr[1][0] == arr[i+1][i]) {
                count++;
            }
        }
        if(count == 4) {
            return 1;
        }
        else {
            count = 0;
        }
    }
    //왼쪽 아래 오른쪽 위 4개 위 5
    count = 0;
    if(arr[0][4] != ' ') {
        for(int i = 1, j = 3; i < 5 ; i++, j--) {
            if(arr[0][4] == arr[i][j]) {
                count++;
            }
        }
        if(count == 4) {
            return 1;
        }
        else {
            count = 0;
        }
    }
    //왼쪽 아래 오른쪽 위 4개 아래 6
    count = 0;
    if(arr[1][5] != ' ') {
        for(int i = 2, j = 4 ; i < 6 ; i++, j--) {
            if(arr[1][5] == arr[i][j]) {
                count++;
            }
        }
        if(count == 4) {
            return 1;
        }
        else {
            count = 0;
        }
    }
    return 0;
}
