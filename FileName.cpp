//#include <iostream>
//#include <string>
//#include <ctime>
//#include <cstdlib>
//#include <cstdio>
//#include <vector>
//#include <algorithm>
//#include <cmath>
//#include <limits>
//#include <map>
//#include <set>
//#include <tuple>
//#include <cstring>
//#include <queue>
//#include <chrono>
//using namespace std;
//using namespace chrono;
//
//const int SIZE = 15;
//const int WIN_SCORE = 100000000;
//const int MAX_DEPTH = 4;
//const int TIME_LIMIT = 1000; // 时间限制1秒
//
//// 棋盘状态
//int board[SIZE][SIZE] = { 0 };  // 1:黑棋, -1:白棋, 0:空白
//vector<pair<int, int>> black_list;
//vector<pair<int, int>> white_list;
//vector<pair<int, int>> all_list;
//
//// 换手相关
//bool swapped = false;
//bool swap_decision_made = false;
//int total_moves = 0;
//
//// 搜索相关
//pair<int, int> best_move;
//int cut_count;
//int search_count;
//double attack_ratio = 0.8;  // 降低进攻系数，增加防守
//int ai_player = 1;           // AI执子颜色：1黑棋，-1白棋
//int search_root_depth;
//steady_clock::time_point start_time;
//bool time_out = false;
//
//// 棋型定义
//struct Pattern {
//    int score;
//    vector<int> shape;
//    Pattern(int s, initializer_list<int> sh) : score(s), shape(sh) {}
//};
//
//vector<Pattern> patterns = {
//    // 连五
//    Pattern(WIN_SCORE, {1, 1, 1, 1, 1}),
//
//    // 活四
//    Pattern(100000, {0, 1, 1, 1, 1, 0}),
//
//    // 冲四
//    Pattern(50000, {1, 1, 1, 1, 0}),
//    Pattern(50000, {0, 1, 1, 1, 1}),
//    Pattern(50000, {1, 1, 1, 0, 1}),
//    Pattern(50000, {1, 1, 0, 1, 1}),
//    Pattern(50000, {1, 0, 1, 1, 1}),
//    Pattern(30000, {0, 1, 1, 1, 0, 0}),
//    Pattern(30000, {0, 0, 1, 1, 1, 0}),
//
//    // 活三
//    Pattern(5000, {0, 1, 1, 1, 0}),
//    Pattern(5000, {0, 1, 1, 0, 1, 0}),
//    Pattern(5000, {0, 1, 0, 1, 1, 0}),
//
//    // 眠三
//    Pattern(800, {1, 1, 1, 0, 0}),
//    Pattern(800, {0, 0, 1, 1, 1}),
//    Pattern(800, {1, 1, 0, 1, 0}),
//    Pattern(800, {0, 1, 0, 1, 1}),
//    Pattern(800, {1, 0, 1, 1, 0}),
//    Pattern(600, {1, 0, 1, 0, 1}),
//
//    // 活二
//    Pattern(120, {0, 1, 1, 0, 0}),
//    Pattern(120, {0, 0, 1, 1, 0}),
//    Pattern(100, {0, 1, 0, 1, 0}),
//
//    // 眠二
//    Pattern(20, {1, 1, 0, 0, 0}),
//    Pattern(20, {0, 0, 0, 1, 1}),
//    Pattern(15, {1, 0, 1, 0, 0})
//};
//
//// 方向数组
//const int dx[] = { 0, 1, 1, 1 };
//const int dy[] = { 1, 0, 1, -1 };
//
//// 快速获取棋子状态
//int getStone(int x, int y) {
//    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE) return 2;
//    return board[x][y];
//}
//
//// 检查超时
//bool isTimeOut() {
//    if (time_out) return true;
//    auto now = steady_clock::now();
//    if (duration_cast<milliseconds>(now - start_time).count() > TIME_LIMIT) {
//        time_out = true;
//        return true;
//    }
//    return false;
//}
//
//// 检查胜利
//bool checkWin(int x, int y, int player) {
//    for (int dir = 0; dir < 4; dir++) {
//        int count = 1;
//
//        // 正方向
//        for (int step = 1; step <= 4; step++) {
//            int nx = x + dx[dir] * step;
//            int ny = y + dy[dir] * step;
//            if (getStone(nx, ny) == player) count++;
//            else break;
//        }
//
//        // 反方向
//        for (int step = 1; step <= 4; step++) {
//            int nx = x - dx[dir] * step;
//            int ny = y - dy[dir] * step;
//            if (getStone(nx, ny) == player) count++;
//            else break;
//        }
//
//        if (count >= 5) return true;
//    }
//    return false;
//}
//
//// 计算单方向分数（使用滑动窗口，支持跳棋型）
//int evaluateDirection(int x, int y, int dir, int player) {
//    int score = 0;
//    vector<int> line;
//
//    // 收集该方向上的完整线段（15个位置）
//    for (int i = -7; i <= 7; i++) {
//        int nx = x + i * dx[dir];
//        int ny = y + i * dy[dir];
//        int stone = getStone(nx, ny);
//
//        if (stone == 2) {
//            line.push_back(3);  // 边界标记
//        }
//        else if (stone == player) {
//            line.push_back(1);
//        }
//        else if (stone == -player) {
//            line.push_back(2);
//        }
//        else {
//            line.push_back(0);
//        }
//    }
//
//    // 按连续段扫描，每个段只计分一次
//    for (int i = 0; i < (int)line.size(); i++) {
//        if (line[i] != 1) continue;
//
//        int len = 0;
//        while (i + len < (int)line.size() && line[i + len] == 1)
//            len++;
//
//        int left = (i - 1 >= 0) ? line[i - 1] : 3;
//        int right = (i + len < (int)line.size()) ? line[i + len] : 3;
//
//        // 构造标准棋型
//        vector<int> shape;
//        if (left == 0) shape.push_back(0);
//        for (int k = 0; k < len; k++) shape.push_back(1);
//        if (right == 0) shape.push_back(0);
//
//        // 匹配棋型
//        for (auto& p : patterns) {
//            if (shape == p.shape) {
//                score += p.score;
//                break;
//            }
//        }
//
//        i += len - 1; // 跳过已处理的连续段
//    }
//
//    return score;
//}
//// 评估单个位置
//int evaluatePosition(int x, int y, int player) {
//    if (board[x][y] != 0) return 0;
//
//    int total_score = 0;
//    for (int dir = 0; dir < 4; dir++) {
//        total_score += evaluateDirection(x, y, dir, player);
//    }
//    return total_score;
//}
//
//// 快速评估函数（使用增量方式）
//map<pair<int, int>, int> pos_cache_black;
//map<pair<int, int>, int> pos_cache_white;
//int cached_black_score = -1;
//int cached_white_score = -1;
//
//void updateCache(int x, int y, int player, bool add) {
//    if (player == 1) {
//        pos_cache_black[{x, y}] = 0;
//        // 重新计算受影响的位置
//        for (int i = -3; i <= 3; i++) {
//            for (int j = -3; j <= 3; j++) {
//                int nx = x + i, ny = y + j;
//                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
//                    if (board[nx][ny] == 0) {
//                        if (add) {
//                            pos_cache_black[{nx, ny}] = evaluatePosition(nx, ny, 1);
//                            pos_cache_white[{nx, ny}] = evaluatePosition(nx, ny, -1);
//                        }
//                        else {
//                            auto it_black = pos_cache_black.find({ nx, ny });
//                            if (it_black != pos_cache_black.end()) pos_cache_black.erase(it_black);
//                            auto it_white = pos_cache_white.find({ nx, ny });
//                            if (it_white != pos_cache_white.end()) pos_cache_white.erase(it_white);
//                        }
//                    }
//                }
//            }
//        }
//        cached_black_score = -1;
//        cached_white_score = -1;
//    }
//    else {
//        pos_cache_white[{x, y}] = 0;
//        for (int i = -3; i <= 3; i++) {
//            for (int j = -3; j <= 3; j++) {
//                int nx = x + i, ny = y + j;
//                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
//                    if (board[nx][ny] == 0) {
//                        if (add) {
//                            pos_cache_black[{nx, ny}] = evaluatePosition(nx, ny, 1);
//                            pos_cache_white[{nx, ny}] = evaluatePosition(nx, ny, -1);
//                        }
//                        else {
//                            auto it_black = pos_cache_black.find({ nx, ny });
//                            if (it_black != pos_cache_black.end()) pos_cache_black.erase(it_black);
//                            auto it_white = pos_cache_white.find({ nx, ny });
//                            if (it_white != pos_cache_white.end()) pos_cache_white.erase(it_white);
//                        }
//                    }
//                }
//            }
//        }
//        cached_black_score = -1;
//        cached_white_score = -1;
//    }
//}
//
//// 评估整个棋盘（使用缓存优化）
//int evaluateBoard() {
//    if (cached_black_score != -1 && cached_white_score != -1) {
//        int ai_score = (ai_player == 1) ? cached_black_score : cached_white_score;
//        int opp_score = (ai_player == 1) ? cached_white_score : cached_black_score;
//        return (int)(ai_score * attack_ratio) - opp_score;
//    }
//
//    int black_score = 0;
//    int white_score = 0;
//
//    // 使用缓存评估空位置
//    for (auto& item : pos_cache_black) {
//        black_score += item.second;
//    }
//    for (auto& item : pos_cache_white) {
//        white_score += item.second;
//    }
//
//    // 评估已有棋子
//    for (int i = 0; i < SIZE; i++) {
//        for (int j = 0; j < SIZE; j++) {
//            if (board[i][j] == 1) {
//                for (int dir = 0; dir < 4; dir++) {
//                    black_score += evaluateDirection(i, j, dir, 1);
//                }
//            }
//            else if (board[i][j] == -1) {
//                for (int dir = 0; dir < 4; dir++) {
//                    white_score += evaluateDirection(i, j, dir, -1);
//                }
//            }
//        }
//    }
//
//    cached_black_score = black_score;
//    cached_white_score = white_score;
//
//    int ai_score = (ai_player == 1) ? black_score : white_score;
//    int opp_score = (ai_player == 1) ? white_score : black_score;
//
//    return (int)(ai_score * attack_ratio) - opp_score;
//}
//// 生成候选位置（优化版）
//vector<pair<int, int>> generateMoves() {
//    vector<pair<int, int>> moves;
//
//    if (all_list.empty()) {
//        moves.push_back({ 7, 7 });
//        return moves;
//    }
//
//    set<pair<int, int>> candidates;
//    for (auto& pos : all_list) {
//        int range = (total_moves < 20) ? 2 : 1;
//        for (int i = -range; i <= range; i++) {
//            for (int j = -range; j <= range; j++) {
//                if (i == 0 && j == 0) continue;
//                int nx = pos.first + i;
//                int ny = pos.second + j;
//                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[nx][ny] == 0) {
//                    candidates.insert({ nx, ny });
//                }
//            }
//        }
//    }
//
//    for (auto& pos : candidates) {
//        moves.push_back(pos);
//    }
//
//    // 按启发式价值排序（安全访问map）
//    sort(moves.begin(), moves.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
//        auto it_a1 = pos_cache_black.find(a);
//        auto it_a2 = pos_cache_white.find(a);
//        auto it_b1 = pos_cache_black.find(b);
//        auto it_b2 = pos_cache_white.find(b);
//
//        int score_a = (it_a1 != pos_cache_black.end() ? it_a1->second : 0) +
//            (it_a2 != pos_cache_white.end() ? it_a2->second : 0);
//        int score_b = (it_b1 != pos_cache_black.end() ? it_b1->second : 0) +
//            (it_b2 != pos_cache_white.end() ? it_b2->second : 0);
//        return score_a > score_b;
//        });
//
//    // 动态限制搜索范围
//    int max_moves = (total_moves < 30) ? 30 : 20;
//    if (moves.size() > max_moves) moves.resize(max_moves);
//
//    return moves;
//}
//
//// 换手决策
//bool shouldConsiderSwap() {
//    if (swapped || swap_decision_made) return false;
//    // 黑棋第三手刚下完（总棋子数为3），白棋还没有下第4步
//    return (total_moves == 3 && !swap_decision_made);
//}
//
//// 评估是否换手
//bool evaluateSwap() {
//    int current_score = evaluateBoard();
//    // 如果对手优势超过阈值，换手
//    return (current_score < -3000);
//}
//
//// 执行换手
//bool makeSwapDecision() {
//    if (!shouldConsiderSwap()) return false;
//    swap_decision_made = true;
//
//    if (evaluateSwap()) {
//        // 交换双方棋子集合
//        swap(black_list, white_list);
//        swapped = true;
//
//        // 换手后AI成为白棋
//        ai_player = -ai_player;
//
//        // 更新棋盘
//        memset(board, 0, sizeof(board));
//        for (auto& pos : black_list) {
//            board[pos.first][pos.second] = 1;
//        }
//        for (auto& pos : white_list) {
//            board[pos.first][pos.second] = -1;
//        }
//
//        // 清空缓存
//        pos_cache_black.clear();
//        pos_cache_white.clear();
//        cached_black_score = -1;
//        cached_white_score = -1;
//
//        return true;
//    }
//    return false;
//}
//
//// 负值极大搜索（带Alpha-Beta剪枝和超时控制）
//int negamax(int depth_left, int alpha, int beta, bool is_ai) {
//    if (isTimeOut()) return 0;
//
//    // 达到搜索深度
//    if (depth_left == 0) {
//        return evaluateBoard();
//    }
//
//    vector<pair<int, int>> moves = generateMoves();
//    if (moves.empty()) return 0;
//
//    search_count++;
//
//    for (auto& move : moves) {
//        int x = move.first, y = move.second;
//        int player = is_ai ? ai_player : -ai_player;
//
//        // 落子
//        board[x][y] = player;
//        all_list.push_back(move);
//        if (player == 1) black_list.push_back(move);
//        else white_list.push_back(move);
//        updateCache(x, y, player, true);
//
//        int value;
//        // 检查胜利（当前落子后）
//        if (checkWin(x, y, player)) {
//            value = (player == ai_player) ? WIN_SCORE - depth_left : -WIN_SCORE + depth_left;
//        }
//        else {
//            value = -negamax(depth_left - 1, -beta, -alpha, !is_ai);
//        }
//
//        // 撤销落子
//        if (player == 1) black_list.pop_back();
//        else white_list.pop_back();
//        all_list.pop_back();
//        board[x][y] = 0;
//        updateCache(x, y, player, false);
//
//        // 更新最佳位置
//        if (value > alpha) {
//            alpha = value;
//            if (depth_left == search_root_depth && !isTimeOut()) {
//                best_move = move;
//            }
//
//            // Alpha-Beta剪枝
//            if (alpha >= beta) {
//                cut_count++;
//                return alpha;
//            }
//        }
//    }
//
//    return alpha;
//}
//
//// 迭代加深搜索
//pair<int, int> iterativeDeepening() {
//    best_move = { -1, -1 };
//    int best_score = -WIN_SCORE;
//
//    // 动态调整最大深度
//    int max_depth = MAX_DEPTH;
//    if (total_moves < 10) max_depth = 3;
//    if (total_moves > 40) max_depth = 5;
//    if (total_moves > 60) max_depth = 6;
//
//    for (int d = 1; d <= max_depth; d++) {
//        if (isTimeOut()) break;
//
//        search_root_depth = d;
//        int score = negamax(d, -WIN_SCORE, WIN_SCORE, true);
//
//        if (isTimeOut()) break;
//
//        if (score > best_score && best_move.first != -1) {
//            best_score = score;
//        }
//
//        // 如果找到必胜走法，提前结束
//        if (score >= WIN_SCORE / 2) {
//            break;
//        }
//
//        // 时间控制
//        auto now = steady_clock::now();
//        int elapsed = duration_cast<milliseconds>(now - start_time).count();
//        if (elapsed > TIME_LIMIT * 0.8) {
//            break;
//        }
//    }
//
//    return best_move;
//}
//
//// AI决策
//pair<int, int> aiDecision() {
//    cut_count = 0;
//    search_count = 0;
//    time_out = false;
//    start_time = steady_clock::now();
//
//    // 开局优化
//    if (all_list.empty()) {
//        return { 7, 7 };
//    }
//
//    // 首步优化
//    if (total_moves == 1) {
//        // 如果对手第一步不是天元，可以考虑下天元
//        if (all_list[0].first != 7 || all_list[0].second != 7) {
//            return { 7, 7 };
//        }
//    }
//
//    // 使用迭代加深
//    pair<int, int> move = iterativeDeepening();
//
//    // 如果没有找到合适位置，选择第一个候选位置
//    if (move.first == -1) {
//        auto moves = generateMoves();
//        if (!moves.empty()) move = moves[0];
//    }
//
//    return move;
//}
//
//int main() {
//    int n, x, y;
//
//    // 读取历史步数
//    cin >> n;
//
//    // 读取历史落子（成对读取）
//    for (int i = 0; i < n - 1; i++) {
//        // 读取对方落子（白棋）
//        cin >> x >> y;
//        if (x != -1) {
//            board[x][y] = -1;
//            white_list.push_back({ x, y });
//            all_list.push_back({ x, y });
//            total_moves++;
//            updateCache(x, y, -1, true);
//        }
//
//        // 读取AI落子（黑棋）
//        cin >> x >> y;
//        if (x != -1) {
//            board[x][y] = 1;
//            black_list.push_back({ x, y });
//            all_list.push_back({ x, y });
//            total_moves++;
//            updateCache(x, y, 1, true);
//        }
//    }
//
//    // 读取对方最后一步
//    cin >> x >> y;
//    if (x != -1) {
//        board[x][y] = -1;
//        white_list.push_back({ x, y });
//        all_list.push_back({ x, y });
//        total_moves++;
//        updateCache(x, y, -1, true);
//    }
//
//    // 检查换手（白棋在黑棋第三手后的决策）
//    if (shouldConsiderSwap()) {
//        if (makeSwapDecision()) {
//            printf("-1 -1\n");
//            return 0;
//        }
//    }
//
//    // AI决策
//    pair<int, int> move = aiDecision();
//    printf("%d %d\n", move.first, move.second);
//
//    return 0;
//}
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <cmath>

using namespace std;

const int BOARDSIZE = 15;
const int WIN_LEN = 5;
const double TIME_LIMIT = 0.95;

struct MCTSNode {
    uint32_t visit_count = 0;
    int32_t value = 0;
    MCTSNode* parent = nullptr;
    MCTSNode** children = nullptr;
    uint16_t child_count = 0;
    uint16_t move_x = 0, move_y = 0;
    bool is_leaf = true;

    double uct_value(double explore = 1.414213562) const {
        if (visit_count == 0) return 1e9;
        uint32_t pvc = parent ? parent->visit_count : 1;
        double exploit = (double)value / visit_count;
        double explore_term = explore * sqrt(log((double)pvc) / visit_count);
        return exploit + explore_term;
    }
};

class SimpleNodePool {
    vector<MCTSNode*> nodes;
public:
    MCTSNode* allocate() {
        MCTSNode* node = new MCTSNode();
        nodes.push_back(node);
        return node;
    }

    void clear() {
        for (auto n : nodes) {
            if (n->children) delete[] n->children;
            delete n;
        }
        nodes.clear();
    }

    ~SimpleNodePool() { clear(); }
};

struct GameState {
    int8_t board[BOARDSIZE][BOARDSIZE];
    int last_x = -1, last_y = -1;
    bool is_black = true;
    int move_count = 0;

    GameState() {
        memset(board, 0, sizeof(board));
    }

    GameState(const GameState& other) {
        memcpy(board, other.board, sizeof(board));
        last_x = other.last_x;
        last_y = other.last_y;
        is_black = other.is_black;
        move_count = other.move_count;
    }
};

bool is_valid(int x, int y) {
    return x >= 0 && x < BOARDSIZE && y >= 0 && y < BOARDSIZE;
}

bool cell_empty(const GameState& state, int x, int y) {
    return is_valid(x, y) && state.board[x][y] == 0;
}

bool check_win(const GameState& state, int x, int y, int8_t player) {
    if (!is_valid(x, y) || state.board[x][y] != player) return false;

    int dx[] = { 0, 1, 1, 1 };
    int dy[] = { 1, 0, 1, -1 };

    for (int d = 0; d < 4; ++d) {
        int count = 1;
        for (int i = 1; i < WIN_LEN; ++i) {
            int nx = x + dx[d] * i, ny = y + dy[d] * i;
            if (is_valid(nx, ny) && state.board[nx][ny] == player) count++;
            else break;
        }
        for (int i = 1; i < WIN_LEN; ++i) {
            int nx = x - dx[d] * i, ny = y - dy[d] * i;
            if (is_valid(nx, ny) && state.board[nx][ny] == player) count++;
            else break;
        }
        if (count >= WIN_LEN) return true;
    }
    return false;
}

bool make_move(GameState& state, int x, int y) {
    if (!cell_empty(state, x, y)) return false;
    state.board[x][y] = state.is_black ? -1 : 1;
    state.last_x = x;
    state.last_y = y;
    state.is_black = !state.is_black;
    state.move_count++;
    return true;
}

void generate_moves(const GameState& state, vector<pair<int, int>>& moves) {
    moves.clear();
    if (state.move_count == 0) {
        moves.push_back({ 7, 7 });
        return;
    }
    if (state.move_count == 1) {
        for (int i = 5; i < 10; ++i)
            for (int j = 5; j < 10; ++j)
                if (cell_empty(state, i, j)) moves.push_back({ i, j });
        return;
    }

    bool visited[BOARDSIZE][BOARDSIZE] = {};
    for (int i = 0; i < BOARDSIZE; ++i) {
        for (int j = 0; j < BOARDSIZE; ++j) {
            if (state.board[i][j] != 0) {
                for (int dx = -2; dx <= 2; ++dx) {
                    for (int dy = -2; dy <= 2; ++dy) {
                        int nx = i + dx, ny = j + dy;
                        if (is_valid(nx, ny) && state.board[nx][ny] == 0 && !visited[nx][ny]) {
                            visited[nx][ny] = true;
                            moves.push_back({ nx, ny });
                        }
                    }
                }
            }
        }
    }

    if (moves.empty()) {
        for (int i = 0; i < BOARDSIZE; ++i)
            for (int j = 0; j < BOARDSIZE; ++j)
                if (cell_empty(state, i, j)) moves.push_back({ i, j });
    }
}

int fast_rollout(GameState state, uint32_t seed) {
    while (state.move_count < BOARDSIZE * BOARDSIZE) {
        vector<pair<int, int>> moves;
        generate_moves(state, moves);
        if (moves.empty()) return 0;

        seed = seed * 1103515245 + 12345;
        int idx = (seed >> 16) % moves.size();
        int x = moves[idx].first, y = moves[idx].second;

        int8_t player = state.is_black ? -1 : 1;
        if (!make_move(state, x, y)) return 0;
        if (check_win(state, x, y, player)) return player;
    }
    return 0;
}

class MCTSEngine {
public:
    SimpleNodePool node_pool;
    MCTSNode* root = nullptr;
    uint32_t rng_seed = 12345;

    MCTSEngine() { root = node_pool.allocate(); }

    void clear() {
        node_pool.clear();
        root = node_pool.allocate();
    }

    void search(const GameState& state, double time_limit) {
        clock_t start_time = clock();
        clock_t threshold = (clock_t)(time_limit * CLOCKS_PER_SEC);

        while (clock() - start_time < threshold) {
            single_iteration(state);
        }
    }

    pair<int, int> best_move(const GameState& state) {
        if (!root || root->child_count == 0) {
            return fallback_move(state);
        }

        MCTSNode* best = nullptr;
        uint32_t max_visits = 0;

        for (int i = 0; i < root->child_count; ++i) {
            MCTSNode* child = root->children[i];
            if (child->visit_count > max_visits) {
                max_visits = child->visit_count;
                best = child;
            }
        }

        if (best && best->visit_count > 5 && cell_empty(state, best->move_x, best->move_y)) {
            return { best->move_x, best->move_y };
        }

        if (best && cell_empty(state, best->move_x, best->move_y)) {
            return { best->move_x, best->move_y };
        }

        for (int i = 0; i < root->child_count; ++i) {
            MCTSNode* child = root->children[i];
            if (cell_empty(state, child->move_x, child->move_y)) {
                return { child->move_x, child->move_y };
            }
        }

        return fallback_move(state);
    }

    pair<int, int> fallback_move(const GameState& state) {
        vector<pair<int, int>> moves;
        generate_moves(state, moves);
        if (!moves.empty()) return moves[0];

        for (int i = 0; i < BOARDSIZE; ++i) {
            for (int j = 0; j < BOARDSIZE; ++j) {
                if (cell_empty(state, i, j)) return { i, j };
            }
        }
        return { 7, 7 };
    }

private:
    void single_iteration(const GameState& init_state) {
        GameState state = init_state;
        vector<MCTSNode*> path;
        MCTSNode* current = root;
        path.push_back(current);

        while (!current->is_leaf) {
            MCTSNode* best_child = select_child(current);
            if (!best_child) break;

            if (!make_move(state, best_child->move_x, best_child->move_y)) break;
            current = best_child;
            path.push_back(current);
        }

        if (current->is_leaf) {
            expand(current, state);
            if (current->child_count > 0) {
                uint32_t seed = rng_seed++;
                MCTSNode* random_child = current->children[seed % current->child_count];
                if (make_move(state, random_child->move_x, random_child->move_y)) {
                    current = random_child;
                    path.push_back(current);
                }
            }
        }

        int result = fast_rollout(state, rng_seed++);
        backpropagate(path, result, init_state.is_black);
    }

    MCTSNode* select_child(MCTSNode* parent) {
        double best_uct = -1e9;
        MCTSNode* best_child = nullptr;

        for (int i = 0; i < parent->child_count; ++i) {
            MCTSNode* child = parent->children[i];
            double uct = child->uct_value();
            if (uct > best_uct) {
                best_uct = uct;
                best_child = child;
            }
        }
        return best_child;
    }

    void expand(MCTSNode* node, const GameState& state) {
        vector<pair<int, int>> moves;
        generate_moves(state, moves);

        if (moves.empty()) {
            node->is_leaf = true;
            return;
        }

        int limit = min((int)moves.size(), 100);
        node->child_count = limit;
        node->children = new MCTSNode * [limit];

        for (int i = 0; i < limit; ++i) {
            MCTSNode* child = node_pool.allocate();
            child->parent = node;
            child->move_x = moves[i].first;
            child->move_y = moves[i].second;
            child->is_leaf = true;
            node->children[i] = child;
        }

        node->is_leaf = false;
    }

    void backpropagate(vector<MCTSNode*>& path, int result, bool is_black) {
        for (MCTSNode* node : path) {
            node->visit_count++;
            if (result != 0) {
                int reward = (result == (is_black ? -1 : 1)) ? 1 : -1;
                node->value += reward;
            }
            is_black = !is_black;
        }
    }
};

int main() {
    int k;
    cin >> k;

    vector<pair<int, int>> history;
    for (int i = 0; i < 2 * k; ++i) {
        int x, y;
        cin >> x >> y;
        history.push_back({ x, y });
    }

    GameState state;
    bool is_black = true;   // 当前我方是否为黑方（-1）
    int my_color, opp_color;

    if (k == 0) {
        is_black = true;
    }
    else if (history.size() >= 2 &&
        history[0].first == -1 && history[0].second == -1 &&
        history[1].first != -1) {
        is_black = true;
    }
    else if (history.size() >= 2 &&
        history[0].first != -1 &&
        history[1].first == -1 && history[1].second == -1) {
        is_black = false;
    }
    else {
        is_black = (history[0].first == -1 && history[0].second == -1);
    }

    my_color = is_black ? -1 : 1;
    opp_color = -my_color;

    for (size_t i = 0; i < history.size(); ++i) {
        int x = history[i].first, y = history[i].second;
        if (x == -1 && y == -1) continue;
        if (i % 2 == 0) state.board[x][y] = opp_color;
        else            state.board[x][y] = my_color;
        state.last_x = x;
        state.last_y = y;
        state.move_count++;
    }
    state.is_black = is_black;

    int new_x = -1, new_y = -1;

    if (k == 0) {
        new_x = 7; new_y = 7;
    }
    else if (k == 1 && history.size() >= 2 &&
        history[1].first == -1 && history[1].second == -1 &&
        history[0].first != -1) {
        new_x = -1; new_y = -1;
    }
    else {
        MCTSEngine engine;
        engine.search(state, TIME_LIMIT);
        pair<int, int> move = engine.best_move(state);
        int x = move.first, y = move.second;

        if (is_valid(x, y) && cell_empty(state, x, y)) {
            new_x = x; new_y = y;
        }
        else {
            vector<pair<int, int>> moves;
            generate_moves(state, moves);
            if (!moves.empty()) {
                new_x = moves[0].first;
                new_y = moves[0].second;
            }
        }
    }

    cout << "{\"x\": " << new_x << ", \"y\": " << new_y << "}" << endl;
    return 0;
}