#include<iostream>
using namespace std;
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstdio>
using namespace std;

const int SIZE = 15;
int board[SIZE][SIZE] = { 0 };//本方1，对方-1，空白0

int main()
{
	int x, y, n;
	//恢复目前的棋盘信息
	cin >> n;
	for (int i = 0; i < n - 1; i++) {
		cin >> x >> y; if (x != -1) board[x][y] = -1;	//对方
		cin >> x >> y; if (x != -1) board[x][y] = 1;	//我方
	}
	cin >> x >> y;
	if (x != -1) board[x][y] = -1;	//对方

	//此时board[][]里存储的就是当前棋盘的所有棋子信息,x和y存的是对方最近一步下的棋

	/************************************************************************************/
	/***********在下面填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	//下面仅为随机策略的示例代码，可删除
	int new_x, new_y;
	if (x != -1 && n == 1) {  //第一回合，我方后手，能换手一定换手
		new_x = -1; new_y = -1;
	}
	else {  //非第一回合我方后手，或第一回合我方先手，不能换手
		int avail_x[SIZE * SIZE], avail_y[SIZE * SIZE]; //合法位置表
		int cnt = 0;
		for (int i = 0; i < SIZE; i++)
			for (int j = 0; j < SIZE; j++)
				if (board[i][j] == 0) {
					avail_x[cnt] = i;
					avail_y[cnt] = j;
					cnt++;
				}
		srand(time(0));
		int rand_pos = rand() % cnt;
		new_x = avail_x[rand_pos];
		new_y = avail_y[rand_pos];

	}
	/***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	/************************************************************************************/

	// 向平台输出决策结果
	printf("%d %d\n", new_x, new_y);
	return 0;
}