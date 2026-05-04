#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <tuple>
using namespace std;

const int COLUMN = 15;
const int ROW = 15;
const int SIZE = 15;

vector<pair<int, int>> list1;  // AI棋子（我方）
vector<pair<int, int>> list2;  // 人类棋子（对方）
vector<pair<int, int>> list3;  // 所有棋子
set<pair<int, int>> all_positions;  // 所有棋盘位置
pair<int, int> next_point;  // AI下一步位置

double ratio = 1.0;  // 进攻系数
int depth = 3;  // 搜索深度
int cut_count;  // 剪枝次数
int search_count;  // 搜索次数

int board[SIZE][SIZE] = { 0 };  // 我方1，对方-1，空白0

// 棋型评估分数
struct ShapeScore {
    int score;
    vector<int> shape;
    ShapeScore(int s, initializer_list<int> sh) : score(s), shape(sh) {}
};

vector<ShapeScore> shape_score = {
    ShapeScore(50, {0, 1, 1, 0, 0}),
    ShapeScore(50, {0, 0, 1, 1, 0}),
    ShapeScore(200, {1, 1, 0, 1, 0}),
    ShapeScore(500, {0, 0, 1, 1, 1}),
    ShapeScore(500, {1, 1, 1, 0, 0}),
    ShapeScore(5000, {0, 1, 1, 1, 0}),
    ShapeScore(5000, {0, 1, 0, 1, 1, 0}),
    ShapeScore(5000, {0, 1, 1, 0, 1, 0}),
    ShapeScore(5000, {1, 1, 1, 0, 1}),
    ShapeScore(5000, {1, 1, 0, 1, 1}),
    ShapeScore(5000, {1, 0, 1, 1, 1}),
    ShapeScore(5000, {1, 1, 1, 1, 0}),
    ShapeScore(5000, {0, 1, 1, 1, 1}),
    ShapeScore(50000, {0, 1, 1, 1, 1, 0}),
    ShapeScore(99999999, {1, 1, 1, 1, 1})
};

// 初始化棋盘位置
void initPositions() {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            all_positions.insert({ i, j });
        }
    }
}

// 检查是否有邻居
bool hasNeighbor(pair<int, int> pt) {
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            pair<int, int> neighbor = { pt.first + i, pt.second + j };
            if (find(list3.begin(), list3.end(), neighbor) != list3.end()) {
                return true;
            }
        }
    }
    return false;
}

// 计算某个方向上的分数
int calScore(int m, int n, int x_direct, int y_direct,
    vector<pair<int, int>>& enemy_list,
    vector<pair<int, int>>& my_list,
    vector<tuple<int, vector<pair<int, int>>, pair<int, int>>>& score_all_arr) {
    int add_score = 0;
    tuple<int, vector<pair<int, int>>, pair<int, int>> max_score_shape = { 0, {}, {0, 0} };

    // 检查是否已计算过
    for (auto& item : score_all_arr) {
        for (auto& pt : get<1>(item)) {
            if (m == pt.first && n == pt.second &&
                x_direct == get<2>(item).first && y_direct == get<2>(item).second) {
                return 0;
            }
        }
    }

    // 查找形状
    for (int offset = -5; offset <= 0; offset++) {
        vector<int> pos;
        for (int i = 0; i < 6; i++) {
            int x = m + (i + offset) * x_direct;
            int y = n + (i + offset) * y_direct;
            pair<int, int> pt = { x, y };

            if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) {
                pos.push_back(3); // 边界标记
            }
            else if (find(enemy_list.begin(), enemy_list.end(), pt) != enemy_list.end()) {
                pos.push_back(2);
            }
            else if (find(my_list.begin(), my_list.end(), pt) != my_list.end()) {
                pos.push_back(1);
            }
            else {
                pos.push_back(0);
            }
        }

        // 跳过包含边界的形状
        bool has_boundary = false;
        for (int p : pos) {
            if (p == 3) {
                has_boundary = true;
                break;
            }
        }
        if (has_boundary) continue;

        vector<int> shape5(pos.begin(), pos.begin() + 5);
        vector<int> shape6(pos.begin(), pos.begin() + 6);

        for (auto& shape_score_item : shape_score) {
            if ((shape5.size() == shape_score_item.shape.size() && shape5 == shape_score_item.shape) ||
                (shape6.size() == shape_score_item.shape.size() && shape6 == shape_score_item.shape)) {
                if (shape_score_item.score > get<0>(max_score_shape)) {
                    vector<pair<int, int>> positions;
                    for (int i = 0; i < 5; i++) {
                        positions.push_back({ m + (i + offset) * x_direct,
                                            n + (i + offset) * y_direct });
                    }
                    max_score_shape = { shape_score_item.score, positions, {x_direct, y_direct} };
                }
            }
        }
    }

    // 计算相交得分
    if (!get<1>(max_score_shape).empty()) {
        for (auto& item : score_all_arr) {
            for (auto& pt1 : get<1>(item)) {
                for (auto& pt2 : get<1>(max_score_shape)) {
                    if (pt1 == pt2 && get<0>(max_score_shape) > 10 && get<0>(item) > 10) {
                        add_score += get<0>(item) + get<0>(max_score_shape);
                    }
                }
            }
        }
        score_all_arr.push_back(max_score_shape);
    }

    return add_score + get<0>(max_score_shape);
}

// 评估函数
int evaluation(bool is_ai) {
    int total_score = 0;

    vector<pair<int, int>>& my_list = is_ai ? list1 : list2;
    vector<pair<int, int>>& enemy_list = is_ai ? list2 : list1;

    // 计算自己得分
    vector<tuple<int, vector<pair<int, int>>, pair<int, int>>> score_all_arr;
    int my_score = 0;
    for (auto& pt : my_list) {
        int m = pt.first, n = pt.second;
        my_score += calScore(m, n, 0, 1, enemy_list, my_list, score_all_arr);
        my_score += calScore(m, n, 1, 0, enemy_list, my_list, score_all_arr);
        my_score += calScore(m, n, 1, 1, enemy_list, my_list, score_all_arr);
        my_score += calScore(m, n, -1, 1, enemy_list, my_list, score_all_arr);
    }

    // 计算敌人得分
    vector<tuple<int, vector<pair<int, int>>, pair<int, int>>> score_all_arr_enemy;
    int enemy_score = 0;
    for (auto& pt : enemy_list) {
        int m = pt.first, n = pt.second;
        enemy_score += calScore(m, n, 0, 1, my_list, enemy_list, score_all_arr_enemy);
        enemy_score += calScore(m, n, 1, 0, my_list, enemy_list, score_all_arr_enemy);
        enemy_score += calScore(m, n, 1, 1, my_list, enemy_list, score_all_arr_enemy);
        enemy_score += calScore(m, n, -1, 1, my_list, enemy_list, score_all_arr_enemy);
    }

    total_score = my_score - enemy_score * ratio * 0.1;
    return total_score;
}

// 搜索顺序排序
void order(vector<pair<int, int>>& blank_list) {
    if (list3.empty()) return;
    pair<int, int> last_pt = list3.back();
    vector<pair<int, int>> neighbors;

    for (auto it = blank_list.begin(); it != blank_list.end();) {
        bool is_neighbor = false;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue;
                pair<int, int> neighbor = { last_pt.first + i, last_pt.second + j };
                if (*it == neighbor) {
                    is_neighbor = true;
                    neighbors.push_back(*it);
                    it = blank_list.erase(it);
                    goto next;
                }
            }
        }
        ++it;
    next:;
    }

    // 将邻居移到前面
    for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
        blank_list.insert(blank_list.begin(), *it);
    }
}

// 检查胜利
bool gameWin(vector<pair<int, int>>& list) {
    for (int m = 0; m < SIZE; m++) {
        for (int n = 0; n < SIZE; n++) {
            pair<int, int> pt = { m, n };
            if (find(list.begin(), list.end(), pt) == list.end()) continue;

            // 水平方向
            if (n <= SIZE - 5) {
                bool win = true;
                for (int k = 1; k <= 4; k++) {
                    if (find(list.begin(), list.end(), make_pair(m, n + k)) == list.end()) {
                        win = false;
                        break;
                    }
                }
                if (win) return true;
            }

            // 垂直方向
            if (m <= SIZE - 5) {
                bool win = true;
                for (int k = 1; k <= 4; k++) {
                    if (find(list.begin(), list.end(), make_pair(m + k, n)) == list.end()) {
                        win = false;
                        break;
                    }
                }
                if (win) return true;
            }

            // 正对角线
            if (m <= SIZE - 5 && n <= SIZE - 5) {
                bool win = true;
                for (int k = 1; k <= 4; k++) {
                    if (find(list.begin(), list.end(), make_pair(m + k, n + k)) == list.end()) {
                        win = false;
                        break;
                    }
                }
                if (win) return true;
            }

            // 反对角线
            if (m <= SIZE - 5 && n >= 4) {
                bool win = true;
                for (int k = 1; k <= 4; k++) {
                    if (find(list.begin(), list.end(), make_pair(m + k, n - k)) == list.end()) {
                        win = false;
                        break;
                    }
                }
                if (win) return true;
            }
        }
    }
    return false;
}

// 负值极大搜索
int negamax(bool is_ai, int depth_left, int alpha, int beta) {
    if (gameWin(list1) || gameWin(list2) || depth_left == 0) {
        return evaluation(is_ai);
    }

    // 生成空白位置列表
    vector<pair<int, int>> blank_list;
    for (auto& pos : all_positions) {
        if (find(list3.begin(), list3.end(), pos) == list3.end()) {
            blank_list.push_back(pos);
        }
    }
    order(blank_list);

    // 遍历候选步
    for (auto& next_step : blank_list) {
        // 限制搜索范围，提高效率
        if (depth_left == depth && !hasNeighbor(next_step)) continue;

        search_count++;

        if (is_ai) {
            list1.push_back(next_step);
        }
        else {
            list2.push_back(next_step);
        }
        list3.push_back(next_step);

        int value = -negamax(!is_ai, depth_left - 1, -beta, -alpha);

        if (is_ai) {
            list1.pop_back();
        }
        else {
            list2.pop_back();
        }
        list3.pop_back();

        if (value > alpha) {
            if (depth_left == depth) {
                next_point = next_step;
            }

            if (value >= beta) {
                cut_count++;
                return beta;
            }
            alpha = value;
        }
    }

    return alpha;
}

// AI决策
pair<int, int> ai() {
    cut_count = 0;
    search_count = 0;
    next_point = { -1, -1 };
    negamax(true, depth, -99999999, 99999999);

    // 如果没有找到合适的位置（开局时），返回中心点
    if (next_point.first == -1 && list3.empty()) {
        next_point = { 7, 7 };
    }
    else if (next_point.first == -1) {
        // 返回第一个空白位置
        for (auto& pos : all_positions) {
            if (find(list3.begin(), list3.end(), pos) == list3.end()) {
                next_point = pos;
                break;
            }
        }
    }

    return next_point;
}

int main()
{
	int x, y, n;
	//恢复目前的棋盘信息
    initPositions();
	cin >> n;
	for (int i = 0; i < n - 1; i++) {
        cin >> x >> y; if (x != -1) { board[x][y] = -1; 
        list2.push_back({ x, y });
        list3.push_back({ x, y });
        }	//对方
        cin >> x >> y; if (x != -1) { board[x][y] = 1; 
        list1.push_back({ x, y });
        list3.push_back({ x, y });
        }	//我方
	}
	cin >> x >> y;
    if (x != -1) {
        board[x][y] = -1;
        list2.push_back({ x, y });
        list3.push_back({ x, y });
    }	//对方

	//此时board[][]里存储的就是当前棋盘的所有棋子信息,x和y存的是对方最近一步下的棋

	/************************************************************************************/
	/***********在下面填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
  
    pair<int, int> decision = ai();
    int new_x = decision.first;
    int new_y = decision.second;
	/***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	/************************************************************************************/

	// 向平台输出决策结果
	printf("%d %d\n", new_x, new_y);
	return 0;
}