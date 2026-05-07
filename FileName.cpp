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
#include <cstring>
#include <queue>
#include <chrono>
using namespace std;
using namespace chrono;

const int SIZE = 15;
const int WIN_SCORE = 100000000;
const int MAX_DEPTH = 4;
const int TIME_LIMIT = 1000; // 时间限制1秒

// 棋盘状态
int board[SIZE][SIZE] = { 0 };  // 1:黑棋, -1:白棋, 0:空白
vector<pair<int, int>> black_list;
vector<pair<int, int>> white_list;
vector<pair<int, int>> all_list;

// 换手相关
bool swapped = false;
bool swap_decision_made = false;
int total_moves = 0;

// 搜索相关
pair<int, int> best_move;
int cut_count;
int search_count;
double attack_ratio = 0.8;  // 降低进攻系数，增加防守
int ai_player = 1;           // AI执子颜色：1黑棋，-1白棋
int search_root_depth;
steady_clock::time_point start_time;
bool time_out = false;

// 棋型定义
struct Pattern {
    int score;
    vector<int> shape;
    Pattern(int s, initializer_list<int> sh) : score(s), shape(sh) {}
};

vector<Pattern> patterns = {
    // 连五
    Pattern(WIN_SCORE, {1, 1, 1, 1, 1}),

    // 活四
    Pattern(100000, {0, 1, 1, 1, 1, 0}),

    // 冲四
    Pattern(50000, {1, 1, 1, 1, 0}),
    Pattern(50000, {0, 1, 1, 1, 1}),
    Pattern(50000, {1, 1, 1, 0, 1}),
    Pattern(50000, {1, 1, 0, 1, 1}),
    Pattern(50000, {1, 0, 1, 1, 1}),
    Pattern(30000, {0, 1, 1, 1, 0, 0}),
    Pattern(30000, {0, 0, 1, 1, 1, 0}),

    // 活三
    Pattern(5000, {0, 1, 1, 1, 0}),
    Pattern(5000, {0, 1, 1, 0, 1, 0}),
    Pattern(5000, {0, 1, 0, 1, 1, 0}),

    // 眠三
    Pattern(800, {1, 1, 1, 0, 0}),
    Pattern(800, {0, 0, 1, 1, 1}),
    Pattern(800, {1, 1, 0, 1, 0}),
    Pattern(800, {0, 1, 0, 1, 1}),
    Pattern(800, {1, 0, 1, 1, 0}),
    Pattern(600, {1, 0, 1, 0, 1}),

    // 活二
    Pattern(120, {0, 1, 1, 0, 0}),
    Pattern(120, {0, 0, 1, 1, 0}),
    Pattern(100, {0, 1, 0, 1, 0}),

    // 眠二
    Pattern(20, {1, 1, 0, 0, 0}),
    Pattern(20, {0, 0, 0, 1, 1}),
    Pattern(15, {1, 0, 1, 0, 0})
};

// 方向数组
const int dx[] = { 0, 1, 1, 1 };
const int dy[] = { 1, 0, 1, -1 };

// 快速获取棋子状态
int getStone(int x, int y) {
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return 2;
    return board[x][y];
}

// 检查超时
bool isTimeOut() {
    if (time_out) return true;
    auto now = steady_clock::now();
    if (duration_cast<milliseconds>(now - start_time).count() > TIME_LIMIT) {
        time_out = true;
        return true;
    }
    return false;
}

// 检查胜利
bool checkWin(int x, int y, int player) {
    for (int dir = 0; dir < 4; dir++) {
        int count = 1;

        // 正方向
        for (int step = 1; step <= 4; step++) {
            int nx = x + dx[dir] * step;
            int ny = y + dy[dir] * step;
            if (getStone(nx, ny) == player) count++;
            else break;
        }

        // 反方向
        for (int step = 1; step <= 4; step++) {
            int nx = x - dx[dir] * step;
            int ny = y - dy[dir] * step;
            if (getStone(nx, ny) == player) count++;
            else break;
        }

        if (count >= 5) return true;
    }
    return false;
}

// 计算单方向分数（使用滑动窗口，支持跳棋型）
int evaluateDirection(int x, int y, int dir, int player) {
    int score = 0;
    vector<int> line;

    // 收集该方向上的完整线段（15个位置）
    for (int i = -7; i <= 7; i++) {
        int nx = x + i * dx[dir];
        int ny = y + i * dy[dir];
        int stone = getStone(nx, ny);

        if (stone == 2) {
            line.push_back(3);  // 边界标记
        }
        else if (stone == player) {
            line.push_back(1);
        }
        else if (stone == -player) {
            line.push_back(2);
        }
        else {
            line.push_back(0);
        }
    }

    // 滑动窗口匹配（长度为5和6）
    for (int i = 0; i <= (int)line.size() - 5; i++) {
        bool has_boundary = false;
        vector<int> pattern5(line.begin() + i, line.begin() + i + 5);

        // 检查是否包含边界
        for (int p : pattern5) {
            if (p == 3) {
                has_boundary = true;
                break;
            }
        }
        if (!has_boundary) {
            for (auto& p : patterns) {
                if (p.shape.size() == 5 && pattern5 == p.shape) {
                    score += p.score;
                    break;
                }
            }
        }

        // 检查6子棋型
        if (i <= (int)line.size() - 6) {
            vector<int> pattern6(line.begin() + i, line.begin() + i + 6);
            has_boundary = false;
            for (int p : pattern6) {
                if (p == 3) {
                    has_boundary = true;
                    break;
                }
            }
            if (!has_boundary) {
                for (auto& p : patterns) {
                    if (p.shape.size() == 6 && pattern6 == p.shape) {
                        score += p.score;
                        break;
                    }
                }
            }
        }
    }

    return score;
}

// 评估单个位置
int evaluatePosition(int x, int y, int player) {
    if (board[x][y] != 0) return 0;

    int total_score = 0;
    for (int dir = 0; dir < 4; dir++) {
        total_score += evaluateDirection(x, y, dir, player);
    }
    return total_score;
}

// 快速评估函数（使用增量方式）
map<pair<int, int>, int> pos_cache_black;
map<pair<int, int>, int> pos_cache_white;
int cached_black_score = -1;
int cached_white_score = -1;

void updateCache(int x, int y, int player, bool add) {
    if (player == 1) {
        pos_cache_black[{x, y}] = 0;
        // 重新计算受影响的位置
        for (int i = -3; i <= 3; i++) {
            for (int j = -3; j <= 3; j++) {
                int nx = x + i, ny = y + j;
                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                    if (board[nx][ny] == 0) {
                        if (add) {
                            pos_cache_black[{nx, ny}] = evaluatePosition(nx, ny, 1);
                            pos_cache_white[{nx, ny}] = evaluatePosition(nx, ny, -1);
                        }
                        else {
                            auto it_black = pos_cache_black.find({ nx, ny });
                            if (it_black != pos_cache_black.end()) pos_cache_black.erase(it_black);
                            auto it_white = pos_cache_white.find({ nx, ny });
                            if (it_white != pos_cache_white.end()) pos_cache_white.erase(it_white);
                        }
                    }
                }
            }
        }
        cached_black_score = -1;
        cached_white_score = -1;
    }
    else {
        pos_cache_white[{x, y}] = 0;
        for (int i = -3; i <= 3; i++) {
            for (int j = -3; j <= 3; j++) {
                int nx = x + i, ny = y + j;
                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                    if (board[nx][ny] == 0) {
                        if (add) {
                            pos_cache_black[{nx, ny}] = evaluatePosition(nx, ny, 1);
                            pos_cache_white[{nx, ny}] = evaluatePosition(nx, ny, -1);
                        }
                        else {
                            auto it_black = pos_cache_black.find({ nx, ny });
                            if (it_black != pos_cache_black.end()) pos_cache_black.erase(it_black);
                            auto it_white = pos_cache_white.find({ nx, ny });
                            if (it_white != pos_cache_white.end()) pos_cache_white.erase(it_white);
                        }
                    }
                }
            }
        }
        cached_black_score = -1;
        cached_white_score = -1;
    }
}

// 评估整个棋盘（使用缓存优化）
int evaluateBoard() {
    if (cached_black_score != -1 && cached_white_score != -1) {
        int ai_score = (ai_player == 1) ? cached_black_score : cached_white_score;
        int opp_score = (ai_player == 1) ? cached_white_score : cached_black_score;
        return ai_score - (int)(opp_score * attack_ratio);
    }

    int black_score = 0;
    int white_score = 0;

    // 使用缓存评估空位置
    for (auto& item : pos_cache_black) {
        black_score += item.second;
    }
    for (auto& item : pos_cache_white) {
        white_score += item.second;
    }

    // 评估已有棋子
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == 1) {
                for (int dir = 0; dir < 4; dir++) {
                    black_score += evaluateDirection(i, j, dir, 1);
                }
            }
            else if (board[i][j] == -1) {
                for (int dir = 0; dir < 4; dir++) {
                    white_score += evaluateDirection(i, j, dir, -1);
                }
            }
        }
    }

    cached_black_score = black_score;
    cached_white_score = white_score;

    int ai_score = (ai_player == 1) ? black_score : white_score;
    int opp_score = (ai_player == 1) ? white_score : black_score;

    return ai_score - (int)(opp_score * attack_ratio);
}

// 生成候选位置（优化版）
vector<pair<int, int>> generateMoves() {
    vector<pair<int, int>> moves;

    if (all_list.empty()) {
        moves.push_back({ 7, 7 });
        return moves;
    }

    set<pair<int, int>> candidates;
    for (auto& pos : all_list) {
        int range = (total_moves < 20) ? 2 : 1;
        for (int i = -range; i <= range; i++) {
            for (int j = -range; j <= range; j++) {
                if (i == 0 && j == 0) continue;
                int nx = pos.first + i;
                int ny = pos.second + j;
                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[nx][ny] == 0) {
                    candidates.insert({ nx, ny });
                }
            }
        }
    }

    for (auto& pos : candidates) {
        moves.push_back(pos);
    }

    // 按启发式价值排序（安全访问map）
    sort(moves.begin(), moves.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        auto it_a1 = pos_cache_black.find(a);
        auto it_a2 = pos_cache_white.find(a);
        auto it_b1 = pos_cache_black.find(b);
        auto it_b2 = pos_cache_white.find(b);

        int score_a = (it_a1 != pos_cache_black.end() ? it_a1->second : 0) +
            (it_a2 != pos_cache_white.end() ? it_a2->second : 0);
        int score_b = (it_b1 != pos_cache_black.end() ? it_b1->second : 0) +
            (it_b2 != pos_cache_white.end() ? it_b2->second : 0);
        return score_a > score_b;
        });

    // 动态限制搜索范围
    int max_moves = (total_moves < 30) ? 30 : 20;
    if (moves.size() > max_moves) moves.resize(max_moves);

    return moves;
}

// 换手决策
bool shouldConsiderSwap() {
    if (swapped || swap_decision_made) return false;
    // 黑棋第三手刚下完（总棋子数为3），白棋还没有下第4步
    return (total_moves == 3 && !swap_decision_made);
}

// 评估是否换手
bool evaluateSwap() {
    int current_score = evaluateBoard();
    // 如果对手优势超过阈值，换手
    return (current_score < -3000);
}

// 执行换手
bool makeSwapDecision() {
    if (!shouldConsiderSwap()) return false;
    swap_decision_made = true;

    if (evaluateSwap()) {
        // 交换双方棋子集合
        swap(black_list, white_list);
        swapped = true;

        // 换手后AI成为白棋
        ai_player = -ai_player;

        // 更新棋盘
        memset(board, 0, sizeof(board));
        for (auto& pos : black_list) {
            board[pos.first][pos.second] = 1;
        }
        for (auto& pos : white_list) {
            board[pos.first][pos.second] = -1;
        }

        // 清空缓存
        pos_cache_black.clear();
        pos_cache_white.clear();
        cached_black_score = -1;
        cached_white_score = -1;

        return true;
    }
    return false;
}

// 负值极大搜索（带Alpha-Beta剪枝和超时控制）
int negamax(int depth_left, int alpha, int beta, bool is_ai) {
    if (isTimeOut()) return 0;

    // 检查当前局面是否已经胜利（上一步对手是否赢了）
    if (!all_list.empty()) {
        auto& last = all_list.back();
        int last_player = board[last.first][last.second];
        if (checkWin(last.first, last.second, last_player)) {
            return (last_player == ai_player) ? WIN_SCORE - depth_left : -WIN_SCORE + depth_left;
        }
    }

    // 达到搜索深度
    if (depth_left == 0) {
        return evaluateBoard();
    }

    vector<pair<int, int>> moves = generateMoves();
    if (moves.empty()) return 0;

    search_count++;

    for (auto& move : moves) {
        int x = move.first, y = move.second;
        int player = is_ai ? ai_player : -ai_player;

        // 落子
        board[x][y] = player;
        all_list.push_back(move);
        if (player == 1) black_list.push_back(move);
        else white_list.push_back(move);
        updateCache(x, y, player, true);

        int value;
        // 检查胜利（当前落子后）
        if (checkWin(x, y, player)) {
            value = (player == ai_player) ? WIN_SCORE - depth_left : -WIN_SCORE + depth_left;
        }
        else {
            value = -negamax(depth_left - 1, -beta, -alpha, !is_ai);
        }

        // 撤销落子
        if (player == 1) black_list.pop_back();
        else white_list.pop_back();
        all_list.pop_back();
        board[x][y] = 0;
        updateCache(x, y, player, false);

        // 更新最佳位置
        if (value > alpha) {
            alpha = value;
            if (depth_left == search_root_depth && !isTimeOut()) {
                best_move = move;
            }

            // Alpha-Beta剪枝
            if (alpha >= beta) {
                cut_count++;
                return alpha;
            }
        }
    }

    return alpha;
}

// 迭代加深搜索
pair<int, int> iterativeDeepening() {
    best_move = { -1, -1 };
    int best_score = -WIN_SCORE;

    // 动态调整最大深度
    int max_depth = MAX_DEPTH;
    if (total_moves < 10) max_depth = 3;
    if (total_moves > 40) max_depth = 5;
    if (total_moves > 60) max_depth = 6;

    for (int d = 1; d <= max_depth; d++) {
        if (isTimeOut()) break;

        search_root_depth = d;
        int score = negamax(d, -WIN_SCORE, WIN_SCORE, true);

        if (isTimeOut()) break;

        if (score > best_score && best_move.first != -1) {
            best_score = score;
        }

        // 如果找到必胜走法，提前结束
        if (score >= WIN_SCORE / 2) {
            break;
        }

        // 时间控制
        auto now = steady_clock::now();
        int elapsed = duration_cast<milliseconds>(now - start_time).count();
        if (elapsed > TIME_LIMIT * 0.8) {
            break;
        }
    }

    return best_move;
}

// AI决策
pair<int, int> aiDecision() {
    cut_count = 0;
    search_count = 0;
    time_out = false;
    start_time = steady_clock::now();

    // 开局优化
    if (all_list.empty()) {
        return { 7, 7 };
    }

    // 首步优化
    if (total_moves == 1) {
        // 如果对手第一步不是天元，可以考虑下天元
        if (all_list[0].first != 7 || all_list[0].second != 7) {
            return { 7, 7 };
        }
    }

    // 使用迭代加深
    pair<int, int> move = iterativeDeepening();

    // 如果没有找到合适位置，选择第一个候选位置
    if (move.first == -1) {
        auto moves = generateMoves();
        if (!moves.empty()) move = moves[0];
    }

    return move;
}

int main() {
    int n, x, y;

    // 读取历史步数
    cin >> n;

    // 读取历史落子（成对读取）
    for (int i = 0; i < n - 1; i++) {
        // 读取对方落子（白棋）
        cin >> x >> y;
        if (x != -1) {
            board[x][y] = -1;
            white_list.push_back({ x, y });
            all_list.push_back({ x, y });
            total_moves++;
            updateCache(x, y, -1, true);
        }

        // 读取AI落子（黑棋）
        cin >> x >> y;
        if (x != -1) {
            board[x][y] = 1;
            black_list.push_back({ x, y });
            all_list.push_back({ x, y });
            total_moves++;
            updateCache(x, y, 1, true);
        }
    }

    // 读取对方最后一步
    cin >> x >> y;
    if (x != -1) {
        board[x][y] = -1;
        white_list.push_back({ x, y });
        all_list.push_back({ x, y });
        total_moves++;
        updateCache(x, y, -1, true);
    }

    // 检查换手（白棋在黑棋第三手后的决策）
    if (shouldConsiderSwap()) {
        if (makeSwapDecision()) {
            printf("-1 -1\n");
            return 0;
        }
    }

    // AI决策
    pair<int, int> move = aiDecision();
    printf("%d %d\n", move.first, move.second);

    return 0;
}