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
//#include <iostream>
//#include <vector>
//#include <algorithm>
//#include <cstring>
//#include <cstdint>
//#include <ctime>
//#include <cmath>
//
//using namespace std;
//
//const int BOARDSIZE = 15;
//const int WIN_LEN = 5;
//const double TIME_LIMIT = 0.95;
//
//struct MCTSNode {
//    uint32_t visit_count = 0;
//    int32_t value = 0;
//    MCTSNode* parent = nullptr;
//    MCTSNode** children = nullptr;
//    uint16_t child_count = 0;
//    uint16_t move_x = 0, move_y = 0;
//    bool is_leaf = true;
//
//    double uct_value(double explore = 1.414213562) const {
//        if (visit_count == 0) return 1e9;
//        uint32_t pvc = parent ? parent->visit_count : 1;
//        double exploit = (double)value / visit_count;
//        double explore_term = explore * sqrt(log((double)pvc) / visit_count);
//        return exploit + explore_term;
//    }
//};
//
//class SimpleNodePool {
//    vector<MCTSNode*> nodes;
//public:
//    MCTSNode* allocate() {
//        MCTSNode* node = new MCTSNode();
//        nodes.push_back(node);
//        return node;
//    }
//
//    void clear() {
//        for (auto n : nodes) {
//            if (n->children) delete[] n->children;
//            delete n;
//        }
//        nodes.clear();
//    }
//
//    ~SimpleNodePool() { clear(); }
//};
//
//struct GameState {
//    int8_t board[BOARDSIZE][BOARDSIZE];
//    int last_x = -1, last_y = -1;
//    bool is_black = true;
//    int move_count = 0;
//
//    GameState() {
//        memset(board, 0, sizeof(board));
//    }
//
//    GameState(const GameState& other) {
//        memcpy(board, other.board, sizeof(board));
//        last_x = other.last_x;
//        last_y = other.last_y;
//        is_black = other.is_black;
//        move_count = other.move_count;
//    }
//};
//
//bool is_valid(int x, int y) {
//    return x >= 0 && x < BOARDSIZE && y >= 0 && y < BOARDSIZE;
//}
//
//bool cell_empty(const GameState& state, int x, int y) {
//    return is_valid(x, y) && state.board[x][y] == 0;
//}
//
//bool check_win(const GameState& state, int x, int y, int8_t player) {
//    if (!is_valid(x, y) || state.board[x][y] != player) return false;
//
//    int dx[] = { 0, 1, 1, 1 };
//    int dy[] = { 1, 0, 1, -1 };
//
//    for (int d = 0; d < 4; ++d) {
//        int count = 1;
//        for (int i = 1; i < WIN_LEN; ++i) {
//            int nx = x + dx[d] * i, ny = y + dy[d] * i;
//            if (is_valid(nx, ny) && state.board[nx][ny] == player) count++;
//            else break;
//        }
//        for (int i = 1; i < WIN_LEN; ++i) {
//            int nx = x - dx[d] * i, ny = y - dy[d] * i;
//            if (is_valid(nx, ny) && state.board[nx][ny] == player) count++;
//            else break;
//        }
//        if (count >= WIN_LEN) return true;
//    }
//    return false;
//}
//
//bool make_move(GameState& state, int x, int y) {
//    if (!cell_empty(state, x, y)) return false;
//    state.board[x][y] = state.is_black ? -1 : 1;
//    state.last_x = x;
//    state.last_y = y;
//    state.is_black = !state.is_black;
//    state.move_count++;
//    return true;
//}
//
//void generate_moves(const GameState& state, vector<pair<int, int>>& moves) {
//    moves.clear();
//    if (state.move_count == 0) {
//        moves.push_back({ 7, 7 });
//        return;
//    }
//    if (state.move_count == 1) {
//        for (int i = 5; i < 10; ++i)
//            for (int j = 5; j < 10; ++j)
//                if (cell_empty(state, i, j)) moves.push_back({ i, j });
//        return;
//    }
//
//    bool visited[BOARDSIZE][BOARDSIZE] = {};
//    for (int i = 0; i < BOARDSIZE; ++i) {
//        for (int j = 0; j < BOARDSIZE; ++j) {
//            if (state.board[i][j] != 0) {
//                for (int dx = -2; dx <= 2; ++dx) {
//                    for (int dy = -2; dy <= 2; ++dy) {
//                        int nx = i + dx, ny = j + dy;
//                        if (is_valid(nx, ny) && state.board[nx][ny] == 0 && !visited[nx][ny]) {
//                            visited[nx][ny] = true;
//                            moves.push_back({ nx, ny });
//                        }
//                    }
//                }
//            }
//        }
//    }
//
//    if (moves.empty()) {
//        for (int i = 0; i < BOARDSIZE; ++i)
//            for (int j = 0; j < BOARDSIZE; ++j)
//                if (cell_empty(state, i, j)) moves.push_back({ i, j });
//    }
//}
//
//int fast_rollout(GameState state, uint32_t seed) {
//    while (state.move_count < BOARDSIZE * BOARDSIZE) {
//        vector<pair<int, int>> moves;
//        generate_moves(state, moves);
//        if (moves.empty()) return 0;
//
//        seed = seed * 1103515245 + 12345;
//        int idx = (seed >> 16) % moves.size();
//        int x = moves[idx].first, y = moves[idx].second;
//
//        int8_t player = state.is_black ? -1 : 1;
//        if (!make_move(state, x, y)) return 0;
//        if (check_win(state, x, y, player)) return player;
//    }
//    return 0;
//}
//
//class MCTSEngine {
//public:
//    SimpleNodePool node_pool;
//    MCTSNode* root = nullptr;
//    uint32_t rng_seed = 12345;
//
//    MCTSEngine() { root = node_pool.allocate(); }
//
//    void clear() {
//        node_pool.clear();
//        root = node_pool.allocate();
//    }
//
//    void search(const GameState& state, double time_limit) {
//        clock_t start_time = clock();
//        clock_t threshold = (clock_t)(time_limit * CLOCKS_PER_SEC);
//
//        while (clock() - start_time < threshold) {
//            single_iteration(state);
//        }
//    }
//
//    pair<int, int> best_move(const GameState& state) {
//        if (!root || root->child_count == 0) {
//            return fallback_move(state);
//        }
//
//        MCTSNode* best = nullptr;
//        uint32_t max_visits = 0;
//
//        for (int i = 0; i < root->child_count; ++i) {
//            MCTSNode* child = root->children[i];
//            if (child->visit_count > max_visits) {
//                max_visits = child->visit_count;
//                best = child;
//            }
//        }
//
//        if (best && best->visit_count > 5 && cell_empty(state, best->move_x, best->move_y)) {
//            return { best->move_x, best->move_y };
//        }
//
//        if (best && cell_empty(state, best->move_x, best->move_y)) {
//            return { best->move_x, best->move_y };
//        }
//
//        for (int i = 0; i < root->child_count; ++i) {
//            MCTSNode* child = root->children[i];
//            if (cell_empty(state, child->move_x, child->move_y)) {
//                return { child->move_x, child->move_y };
//            }
//        }
//
//        return fallback_move(state);
//    }
//
//    pair<int, int> fallback_move(const GameState& state) {
//        vector<pair<int, int>> moves;
//        generate_moves(state, moves);
//        if (!moves.empty()) return moves[0];
//
//        for (int i = 0; i < BOARDSIZE; ++i) {
//            for (int j = 0; j < BOARDSIZE; ++j) {
//                if (cell_empty(state, i, j)) return { i, j };
//            }
//        }
//        return { 7, 7 };
//    }
//
//private:
//    void single_iteration(const GameState& init_state) {
//        GameState state = init_state;
//        vector<MCTSNode*> path;
//        MCTSNode* current = root;
//        path.push_back(current);
//
//        while (!current->is_leaf) {
//            MCTSNode* best_child = select_child(current);
//            if (!best_child) break;
//
//            if (!make_move(state, best_child->move_x, best_child->move_y)) break;
//            current = best_child;
//            path.push_back(current);
//        }
//
//        if (current->is_leaf) {
//            expand(current, state);
//            if (current->child_count > 0) {
//                uint32_t seed = rng_seed++;
//                MCTSNode* random_child = current->children[seed % current->child_count];
//                if (make_move(state, random_child->move_x, random_child->move_y)) {
//                    current = random_child;
//                    path.push_back(current);
//                }
//            }
//        }
//
//        int result = fast_rollout(state, rng_seed++);
//        backpropagate(path, result, init_state.is_black);
//    }
//
//    MCTSNode* select_child(MCTSNode* parent) {
//        double best_uct = -1e9;
//        MCTSNode* best_child = nullptr;
//
//        for (int i = 0; i < parent->child_count; ++i) {
//            MCTSNode* child = parent->children[i];
//            double uct = child->uct_value();
//            if (uct > best_uct) {
//                best_uct = uct;
//                best_child = child;
//            }
//        }
//        return best_child;
//    }
//
//    void expand(MCTSNode* node, const GameState& state) {
//        vector<pair<int, int>> moves;
//        generate_moves(state, moves);
//
//        if (moves.empty()) {
//            node->is_leaf = true;
//            return;
//        }
//
//        int limit = min((int)moves.size(), 100);
//        node->child_count = limit;
//        node->children = new MCTSNode * [limit];
//
//        for (int i = 0; i < limit; ++i) {
//            MCTSNode* child = node_pool.allocate();
//            child->parent = node;
//            child->move_x = moves[i].first;
//            child->move_y = moves[i].second;
//            child->is_leaf = true;
//            node->children[i] = child;
//        }
//
//        node->is_leaf = false;
//    }
//
//    void backpropagate(vector<MCTSNode*>& path, int result, bool is_black) {
//        for (MCTSNode* node : path) {
//            node->visit_count++;
//            if (result != 0) {
//                int reward = (result == (is_black ? -1 : 1)) ? 1 : -1;
//                node->value += reward;
//            }
//            is_black = !is_black;
//        }
//    }
//};
//
//int main() {
//    int k;
//    cin >> k;
//
//    vector<pair<int, int>> history;
//    for (int i = 0; i < 2 * k; ++i) {
//        int x, y;
//        cin >> x >> y;
//        history.push_back({ x, y });
//    }
//
//    GameState state;
//    bool is_black = true;   // 当前我方是否为黑方（-1）
//    int my_color, opp_color;
//
//    if (k == 0) {
//        is_black = true;
//    }
//    else if (history.size() >= 2 &&
//        history[0].first == -1 && history[0].second == -1 &&
//        history[1].first != -1) {
//        is_black = true;
//    }
//    else if (history.size() >= 2 &&
//        history[0].first != -1 &&
//        history[1].first == -1 && history[1].second == -1) {
//        is_black = false;
//    }
//    else {
//        is_black = (history[0].first == -1 && history[0].second == -1);
//    }
//
//    my_color = is_black ? -1 : 1;
//    opp_color = -my_color;
//
//    for (size_t i = 0; i < history.size(); ++i) {
//        int x = history[i].first, y = history[i].second;
//        if (x == -1 && y == -1) continue;
//        if (i % 2 == 0) state.board[x][y] = opp_color;
//        else            state.board[x][y] = my_color;
//        state.last_x = x;
//        state.last_y = y;
//        state.move_count++;
//    }
//    state.is_black = is_black;
//
//    int new_x = -1, new_y = -1;
//
//    if (k == 0) {
//        new_x = 7; new_y = 7;
//    }
//    else if (k == 1 && history.size() >= 2 &&
//        history[1].first == -1 && history[1].second == -1 &&
//        history[0].first != -1) {
//        new_x = -1; new_y = -1;
//    }
//    else {
//        MCTSEngine engine;
//        engine.search(state, TIME_LIMIT);
//        pair<int, int> move = engine.best_move(state);
//        int x = move.first, y = move.second;
//
//        if (is_valid(x, y) && cell_empty(state, x, y)) {
//            new_x = x; new_y = y;
//        }
//        else {
//            vector<pair<int, int>> moves;
//            generate_moves(state, moves);
//            if (!moves.empty()) {
//                new_x = moves[0].first;
//                new_y = moves[0].second;
//            }
//        }
//    }
//
//    cout << "{\"x\": " << new_x << ", \"y\": " << new_y << "}" << endl;
//    return 0;
//}
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <cmath>
#include <random>
#include <chrono>

using namespace std;

const int BOARDSIZE = 15;
const int WIN_LEN = 5;
const double TIME_LIMIT = 0.95;

// ============ MCTS 节点结构 ============
struct MCTSNode {
    uint32_t visit_count = 0;
    int32_t value_sum = 0;           // 从该节点落子方视角的累计奖励
    MCTSNode* parent = nullptr;
    vector<MCTSNode*> children;
    uint16_t move_x = 0, move_y = 0;
    int8_t player_color = 0;         // 在该节点执行落子动作的颜色
    bool is_terminal = false;
    bool is_fully_expanded = false;

    double uct_value(double explore = 1.414213562) const {
        if (visit_count == 0) return 1e9;
        if (is_terminal) return -1e9;
        uint32_t parent_visits = parent ? parent->visit_count : 1;
        if (parent_visits == 0) parent_visits = 1;
        double exploit = static_cast<double>(value_sum) / static_cast<int>(visit_count);
        double explore_term = explore * sqrt(log(static_cast<double>(parent_visits)) / visit_count);
        return exploit + explore_term;
    }
};

// ============ 节点池 ============
class NodePool {
    vector<MCTSNode> pool;
    size_t used_count = 0;
public:
    NodePool(size_t reserve_size = 500000) {
        pool.reserve(reserve_size);
    }
    MCTSNode* allocate() {
        if (used_count < pool.size()) {
            MCTSNode* node = &pool[used_count++];
            *node = MCTSNode();
            return node;
        }
        pool.emplace_back();
        return &pool[used_count++];
    }
    void clear() {
        used_count = 0;
        for (auto& node : pool) node.children.clear();
    }
    size_t size() const { return used_count; }
};

// ============ 游戏状态 ============
struct GameState {
    int8_t board[BOARDSIZE][BOARDSIZE];
    int last_x = -1, last_y = -1;
    bool is_black_turn = true;
    int move_count = 0;
    GameState() { memset(board, 0, sizeof(board)); }
    GameState(const GameState& other) {
        memcpy(board, other.board, sizeof(board));
        last_x = other.last_x; last_y = other.last_y;
        is_black_turn = other.is_black_turn;
        move_count = other.move_count;
    }
    bool isEmpty(int x, int y) const {
        return x >= 0 && x < BOARDSIZE && y >= 0 && y < BOARDSIZE && board[x][y] == 0;
    }
    int8_t currentPlayer() const { return is_black_turn ? -1 : 1; }
};

inline bool is_valid(int x, int y) {
    return x >= 0 && x < BOARDSIZE && y >= 0 && y < BOARDSIZE;
}

bool check_win(const GameState& state, int x, int y, int8_t player) {
    if (!is_valid(x, y) || state.board[x][y] != player) return false;
    const int dx[] = { 0, 1, 1, 1 };
    const int dy[] = { 1, 0, 1, -1 };
    for (int d = 0; d < 4; ++d) {
        int count = 1;
        for (int i = 1; i < WIN_LEN; ++i) {
            int nx = x + dx[d] * i, ny = y + dy[d] * i;
            if (is_valid(nx, ny) && state.board[nx][ny] == player) ++count;
            else break;
        }
        for (int i = 1; i < WIN_LEN; ++i) {
            int nx = x - dx[d] * i, ny = y - dy[d] * i;
            if (is_valid(nx, ny) && state.board[nx][ny] == player) ++count;
            else break;
        }
        if (count >= WIN_LEN) return true;
    }
    return false;
}

// ============ 评估结构体 ============
struct EvalResult {
    int score = 0;
    bool is_forced_win = false;
    bool is_forced_defend = false;
};

EvalResult evaluate_position(const GameState& state, int x, int y, int8_t player) {
    EvalResult res;
    if (!state.isEmpty(x, y)) return res;
    const int dx[] = { 0, 1, 1, 1 };
    const int dy[] = { 1, 0, 1, -1 };
    for (int d = 0; d < 4; ++d) {
        // 评估己方连子
        int count = 1, empty_ends = 0;
        for (int dir = -1; dir <= 1; dir += 2) {
            int empty_found = 0;
            for (int i = 1; i < WIN_LEN; ++i) {
                int nx = x + dx[d] * i * dir, ny = y + dy[d] * i * dir;
                if (!is_valid(nx, ny)) break;
                if (state.board[nx][ny] == player) ++count;
                else if (state.board[nx][ny] == 0) { ++empty_found; break; }
                else break;
            }
            empty_ends += empty_found;
        }
        if (count >= 5) { res.score += 100000; res.is_forced_win = true; }
        else if (count == 4 && empty_ends == 2) { res.score += 10000; res.is_forced_win = true; }
        else if (count == 4 && empty_ends == 1) res.score += 1000;
        else if (count == 3 && empty_ends == 2) res.score += 500;
        else if (count == 3 && empty_ends == 1) res.score += 50;
        else if (count == 2 && empty_ends == 2) res.score += 10;

        // 评估对手连子（用于防守）
        int opp_count = 1, opp_empty = 0;
        for (int dir = -1; dir <= 1; dir += 2) {
            int empty_found = 0;
            for (int i = 1; i < WIN_LEN; ++i) {
                int nx = x + dx[d] * i * dir, ny = y + dy[d] * i * dir;
                if (!is_valid(nx, ny)) break;
                if (state.board[nx][ny] == -player) ++opp_count;
                else if (state.board[nx][ny] == 0) { ++empty_found; break; }
                else break;
            }
            opp_empty += empty_found;
        }
        // 必守局面：对手下一步能直接获胜（连五或活四）或冲四
        if (opp_count >= 5) res.is_forced_defend = true;
        else if (opp_count == 4 && opp_empty == 2) res.is_forced_defend = true;
        else if (opp_count == 4 && opp_empty >= 1) res.is_forced_defend = true; // 冲四
    }
    return res;
}

bool make_move(GameState& state, int x, int y) {
    if (!state.isEmpty(x, y)) return false;
    state.board[x][y] = state.currentPlayer();
    state.last_x = x; state.last_y = y;
    state.is_black_turn = !state.is_black_turn;
    state.move_count++;
    return true;
}

// ============ 着法生成 ============
struct MoveCandidate {
    int x, y, score;
    bool is_forced_win, is_forced_defend;
};

void generate_moves(const GameState& state, vector<pair<int, int>>& moves,
    vector<int>* priorities = nullptr,
    vector<pair<int, int>>* forced_wins = nullptr,
    vector<pair<int, int>>* forced_defends = nullptr) {
    moves.clear();
    if (priorities) priorities->clear();
    if (forced_wins) forced_wins->clear();
    if (forced_defends) forced_defends->clear();

    if (state.move_count == 0) {
        moves.push_back({ 7, 7 });
        if (priorities) priorities->push_back(0);
        return;
    }

    bool visited[BOARDSIZE][BOARDSIZE] = {};
    vector<MoveCandidate> candidates;

    for (int i = 0; i < BOARDSIZE; ++i) {
        for (int j = 0; j < BOARDSIZE; ++j) {
            if (state.board[i][j] != 0) {
                for (int dx = -2; dx <= 2; ++dx) {
                    for (int dy = -2; dy <= 2; ++dy) {
                        int nx = i + dx, ny = j + dy;
                        if (is_valid(nx, ny) && state.board[nx][ny] == 0 && !visited[nx][ny]) {
                            visited[nx][ny] = true;
                            auto my = evaluate_position(state, nx, ny, state.currentPlayer());
                            auto opp = evaluate_position(state, nx, ny, -state.currentPlayer());
                            if (my.is_forced_win && forced_wins) forced_wins->push_back({ nx, ny });
                            if (opp.is_forced_defend && forced_defends) forced_defends->push_back({ nx, ny });
                            int total = max(my.score, opp.score * 4 / 5);
                            candidates.push_back({ nx, ny, total, my.is_forced_win, opp.is_forced_defend });
                        }
                    }
                }
            }
        }
    }

    if (candidates.empty()) {
        for (int i = 0; i < BOARDSIZE; ++i)
            for (int j = 0; j < BOARDSIZE; ++j)
                if (state.isEmpty(i, j)) {
                    moves.push_back({ i, j });
                    if (priorities) priorities->push_back(0);
                }
        return;
    }

    sort(candidates.begin(), candidates.end(), [](const MoveCandidate& a, const MoveCandidate& b) {
        return a.score > b.score;
        });

    int limit = min((int)candidates.size(), 80);
    for (int i = 0; i < limit; ++i) {
        moves.push_back({ candidates[i].x, candidates[i].y });
        if (priorities) priorities->push_back(candidates[i].score);
    }
}

// ============ Rollout ============
int fast_rollout(GameState state, uint32_t& seed) {
    // 检查上一步是否已获胜
    if (state.last_x >= 0) {
        int8_t last_player = state.is_black_turn ? 1 : -1;
        if (check_win(state, state.last_x, state.last_y, last_player))
            return last_player;
    }

    while (state.move_count < BOARDSIZE * BOARDSIZE) {
        vector<pair<int, int>> moves;
        vector<int> scores;
        vector<pair<int, int>> forced_wins, forced_defends;
        generate_moves(state, moves, &scores, &forced_wins, &forced_defends);
        if (moves.empty()) return 0;

        int idx = 0;
        if (!forced_wins.empty()) {
            seed = seed * 1103515245 + 12345;
            int r = (seed >> 16) % forced_wins.size();
            // 找到对应 moves 中的索引（简单线性搜索，规模小）
            for (size_t i = 0; i < moves.size(); ++i)
                if (moves[i] == forced_wins[r]) { idx = i; break; }
        }
        else if (!forced_defends.empty()) {
            seed = seed * 1103515245 + 12345;
            int r = (seed >> 16) % forced_defends.size();
            for (size_t i = 0; i < moves.size(); ++i)
                if (moves[i] == forced_defends[r]) { idx = i; break; }
        }
        else {
            seed = seed * 1103515245 + 12345;
            if ((seed & 0xFFFF) < 6554) { // 10% 随机探索
                idx = (seed >> 16) % moves.size();
            }
            else {
                // 安全 softmax
                double max_score = 0;
                for (int s : scores) max_score = max(max_score, (double)s);
                double total = 0;
                vector<double> weights;
                for (int s : scores) {
                    double w = exp((s - max_score) / 50.0);
                    weights.push_back(w);
                    total += w;
                }
                if (total < 1e-9) idx = 0;
                else {
                    seed = seed * 1103515245 + 12345;
                    double r = (seed & 0x7FFFFFFF) / 2147483648.0 * total;
                    idx = 0;
                    while (r > weights[idx] && idx < (int)weights.size() - 1)
                        r -= weights[idx++];
                }
            }
        }

        int x = moves[idx].first, y = moves[idx].second;
        int8_t player = state.currentPlayer();
        if (!make_move(state, x, y)) return 0;
        if (check_win(state, x, y, player)) return player;
    }
    return 0;
}

// ============ MCTS 引擎 ============
class MCTSEngine {
public:
    NodePool node_pool;
    MCTSNode* root = nullptr;
    uint32_t rng_seed;
    int my_color;           // 我方颜色（当前走棋方）

    MCTSEngine(int color) : my_color(color) {
        rng_seed = chrono::steady_clock::now().time_since_epoch().count();
        root = node_pool.allocate();
        root->player_color = color;   // 根节点颜色设定为当前走棋方
    }

    void clear() {
        node_pool.clear();
        root = node_pool.allocate();
        root->player_color = my_color;
    }

    void search(const GameState& state, double time_limit) {
        clock_t start_time = clock();
        clock_t threshold = static_cast<clock_t>(time_limit * CLOCKS_PER_SEC);
        int iterations = 0;
        while (clock() - start_time < threshold) {
            single_iteration(state);
            ++iterations;
            if ((iterations & 1023) == 0 && clock() - start_time >= threshold) break;
        }
    }

    // 最佳着法：考虑子节点胜率相对于我方颜色的转换
    pair<int, int> best_move(const GameState& state) {
        if (!root || root->children.empty()) return fallback_move(state);

        MCTSNode* best = nullptr;
        double best_score = -1e9;

        for (auto* child : root->children) {
            if (child->visit_count == 0) continue;
            // child->value_sum/visit_count 是从 child 落子方视角的胜率
            double child_winrate = static_cast<double>(child->value_sum) / child->visit_count;
            // 转换为根节点（当前方）的胜率
            double winrate_for_root = (child->player_color == my_color) ? child_winrate : (1.0 - child_winrate);
            // 加入访问次数鼓励探索
            double explore_bonus = 0.05 * sqrt(static_cast<double>(child->visit_count));
            double score = winrate_for_root + explore_bonus;
            if (score > best_score) {
                best_score = score;
                best = child;
            }
        }

        if (best && state.isEmpty(best->move_x, best->move_y))
            return { best->move_x, best->move_y };
        for (auto* child : root->children)
            if (state.isEmpty(child->move_x, child->move_y))
                return { child->move_x, child->move_y };
        return fallback_move(state);
    }

    pair<int, int> fallback_move(const GameState& state) {
        vector<pair<int, int>> moves;
        vector<int> scores;
        vector<pair<int, int>> forced_wins, forced_defends;
        generate_moves(state, moves, &scores, &forced_wins, &forced_defends);
        if (!forced_wins.empty()) return forced_wins[0];
        if (!forced_defends.empty()) return forced_defends[0];
        if (!moves.empty()) {
            int best_idx = 0;
            for (size_t i = 1; i < scores.size(); ++i)
                if (scores[i] > scores[best_idx]) best_idx = i;
            return moves[best_idx];
        }
        for (int i = 0; i < BOARDSIZE; ++i)
            for (int j = 0; j < BOARDSIZE; ++j)
                if (state.isEmpty(i, j)) return { i, j };
        return { 7, 7 };
    }

private:
    void single_iteration(const GameState& init_state) {
        GameState state = init_state;
        vector<MCTSNode*> path;
        MCTSNode* current = root;
        path.push_back(current);

        // Selection
        while (!current->is_fully_expanded && !current->children.empty()) {
            MCTSNode* best_child = select_child(current);
            if (!best_child) break;

            int8_t moved_player = state.currentPlayer();
            if (!make_move(state, best_child->move_x, best_child->move_y)) break;

            if (check_win(state, best_child->move_x, best_child->move_y, moved_player))
                best_child->is_terminal = true;

            current = best_child;
            path.push_back(current);
        }

        // Expansion
        if (!current->is_fully_expanded && !current->is_terminal) {
            expand(current, state);
            if (!current->children.empty()) {
                rng_seed = rng_seed * 1103515245 + 12345;
                int idx = (rng_seed >> 16) % current->children.size();
                MCTSNode* child = current->children[idx];
                int8_t moved_player = state.currentPlayer();
                if (make_move(state, child->move_x, child->move_y)) {
                    if (check_win(state, child->move_x, child->move_y, moved_player))
                        child->is_terminal = true;
                    current = child;
                    path.push_back(current);
                }
            }
        }

        // Simulation
        int result = 0;
        if (current->is_terminal) {
            result = current->player_color;   // 终局时的获胜方
        }
        else {
            result = fast_rollout(state, rng_seed);
        }

        // Backpropagation
        backpropagate(path, result);
    }

    MCTSNode* select_child(MCTSNode* parent) {
        double best_uct = -1e9;
        MCTSNode* best_child = nullptr;
        for (auto* child : parent->children) {
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
            node->is_fully_expanded = true;
            return;
        }
        int8_t next_player = state.currentPlayer();
        for (auto& move : moves) {
            MCTSNode* child = node_pool.allocate();
            child->parent = node;
            child->move_x = move.first;
            child->move_y = move.second;
            child->player_color = next_player;
            node->children.push_back(child);
        }
        node->is_fully_expanded = true;
    }

    void backpropagate(vector<MCTSNode*>& path, int result) {
        for (auto* node : path) {
            node->visit_count++;
            if (result != 0 && node->player_color != 0) {
                int reward = (result == node->player_color) ? 1 : -1;
                node->value_sum += reward;
            }
        }
    }
};

// ============ 主函数 ============
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int k;
    if (!(cin >> k)) return 0;

    vector<pair<int, int>> history;
    for (int i = 0; i < 2 * k; ++i) {
        int x, y;
        cin >> x >> y;
        history.push_back({ x, y });
    }

    GameState state;
    // 重建棋盘，忽略 (-1,-1)
    for (size_t i = 0; i < history.size(); ++i) {
        int x = history[i].first, y = history[i].second;
        if (x == -1 && y == -1) continue;
        int8_t color = (i % 2 == 0) ? -1 : 1;   // 偶数索引 = 黑棋回合
        state.board[x][y] = color;
        state.last_x = x;
        state.last_y = y;
        state.move_count++;
    }
    // 根据实际落子数确定当前轮到谁
    state.is_black_turn = (state.move_count % 2 == 0);

    // 关键修复：程序总是当前走棋方，因此我方颜色即为当前要走的颜色
    int my_color = state.currentPlayer();

    int new_x = -1, new_y = -1;
    // 处理开局特例（k==0 或交换开局）
    if (k == 0) {
        new_x = 7; new_y = 7;
    }
    else if (k == 1 && history.size() >= 2 &&
        history[0].first == -1 && history[0].second == -1 &&
        history[1].first != -1) {
        // 黑方放弃，白方已走第二手，此时轮到黑方（如果是我方），简单选中心
        new_x = 7; new_y = 7;
        if (!state.isEmpty(new_x, new_y)) {
            new_x = 6; new_y = 6;
        }
    }
    else {
        MCTSEngine engine(my_color);
        engine.search(state, TIME_LIMIT);
        pair<int, int> move = engine.best_move(state);
        int x = move.first, y = move.second;
        if (is_valid(x, y) && state.isEmpty(x, y)) {
            new_x = x; new_y = y;
        }
        else {
            auto fb = engine.fallback_move(state);
            new_x = fb.first; new_y = fb.second;
        }
    }

    cout << "{\"x\": " << new_x << ", \"y\": " << new_y << "}" << endl;
    return 0;
}