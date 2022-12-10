#include <locale.h>
#include <ncurses.h>

int main(){
	setlocale(LC_ALL,"ko_KR.utf-8");	   
        initscr();
	
	keypad(stdscr, TRUE);
        
	start_color();
        init_pair(1,COLOR_BLACK,COLOR_YELLOW); //바둑판 색깔임
	init_pair(2,COLOR_RED,COLOR_BLACK);	//타이틀 색깔임
	init_pair(3,COLOR_BLUE,COLOR_BLACK);	//player 색깔임
	init_pair(4,COLOR_BLACK,COLOR_YELLOW);	//흑돌 전용 색
	init_pair(5,COLOR_WHITE,COLOR_YELLOW);	//흰돌 전용 색
        //-타이틀
	attron(COLOR_PAIR(2));
        printw("ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ\n");
        printw("          오목게임을 시작합니다       \n");
        printw("ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ\n");
	attroff(COLOR_PAIR(2));
	//타이틀-
	
	//-바둑판
	attron(COLOR_PAIR(1));
	printw("┏");
        

	for(int i=0;i<=14;i++){
		printw(" ┳");
	}
	printw(" ┓\n");
	
	for(int a=0;a<=14;a++){
		printw("┠");
		for(int b=0;b<=14;b++){
			printw(" ╂");
		}
		printw(" ┫");
		printw("\n");		
	}
	
	printw("┗");

	for(int i=0;i<=14;i++){
		printw(" ┻");
	}

	printw(" ┛\n");
	attroff(COLOR_PAIR(1));
	//바둑판-
	
	//-플레이어표시
	attron(COLOR_PAIR(3));
	mvprintw(3,33,"\t--------------");
	mvprintw(4,33,"\t Melbua:흑돌");
	mvprintw(5,33,"\t 아뭐킹:흰돌");
	mvprintw(6,33,"\t--------------");
	attroff(COLOR_PAIR(3));
	//플레이어표시-
	move(11,16);
	
	//바둑판에서 키보드i/o
	int ch;
	int cp_x=11;
	int cp_y=16;
	while(1){
		if((ch = getch()) != KEY_F(1)){
			switch(ch){
				case KEY_LEFT:
					cp_y-=2;
					move(cp_x,cp_y);
					break;
				
				case KEY_RIGHT:
					cp_y+=2;
					move(cp_x,cp_y);
					break;
				
				case KEY_UP:
					cp_x-=1;
					move(cp_x,cp_y);
					break;

				case KEY_DOWN:
					cp_x+=1;
					move(cp_x,cp_y);
					break;
				case 32:
					move(cp_x,cp_y);
					attron(COLOR_PAIR(4));
					printw("● ");
					attroff(COLOR_PAIR(4));
					break;
				case 'a':
					move(cp_x,cp_y);
					attron(COLOR_PAIR(5));
					printw("● ");
					attroff(COLOR_PAIR(5));
					break;
			}
		}	
	}
	//바둑판 키보드i/o
	
        refresh();
        endwin();
        return 0;
}

