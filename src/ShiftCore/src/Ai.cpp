#include "shift/Ai.h"

#include <algorithm>
#include <array>
#include <cstdlib>

namespace shift {

namespace {

using Clock = std::chrono::steady_clock;
constexpr int kInf = 1'000'000'000;
constexpr int kWin = 1'000'000;

thread_local AiStats g_stats;

// 位置評価。回り込みスライドで端・角は固定されないため、通常リバーシより角の重みを抑える。
int squareWeight(int x, int y) {
    const auto edgeDist = [](int v) { return std::min(v, kSize - 1 - v); };
    const int dx = edgeDist(x);
    const int dy = edgeDist(y);
    if (dx == 0 && dy == 0) return 30;   // 角
    if (dx == 1 && dy == 1) return -12;  // 角の斜め隣
    if ((dx == 0 && dy == 1) || (dx == 1 && dy == 0)) return -6;  // 角の隣
    if (dx == 0 || dy == 0) return 5;    // 辺
    if (dx == 1 || dy == 1) return -2;   // 辺の内側
    return 1;
}

const std::array<int, kCells>& weights() {
    static const std::array<int, kCells> w = [] {
        std::array<int, kCells> a{};
        for (int i = 0; i < kCells; ++i) a[static_cast<size_t>(i)] = squareWeight(i % kSize, i / kSize);
        return a;
    }();
    return w;
}

// 探索用の局面。pending=true の間は予告イベントが counter==4 で発動する。
struct Node {
    Board board;
    Color turn = Cell::Black;
    int counter = 0;
    bool pending = false;
    BoardEvent event;
};

void advanceTurn(Node& n) {
    n.turn = opponent(n.turn);
    if (n.board.isFull()) return;  // 満杯で終局（イベントなし）
    if (++n.counter >= Game::kEventInterval) {
        if (n.pending) {
            n.event.apply(n.board);
            n.pending = false;  // 予告より先の未知イベントは発生しない扱い
        }
        n.counter = 0;
    }
}

int finalScore(const Node& n) {
    const int diff = n.board.count(n.turn) - n.board.count(opponent(n.turn));
    return diff * kWin / 100;
}

int evaluate(const Node& n) {
    const Color me = n.turn;
    const Color op = opponent(me);
    const auto& w = weights();
    int pos = 0;
    int discs = 0;
    for (int i = 0; i < kCells; ++i) {
        const Cell c = n.board.at(fromIndex(i));
        if (c == me) {
            pos += w[static_cast<size_t>(i)];
            ++discs;
        } else if (c == op) {
            pos -= w[static_cast<size_t>(i)];
            --discs;
        }
    }
    const int mobility = static_cast<int>(n.board.legalMoves(me).size()) -
                         static_cast<int>(n.board.legalMoves(op).size());
    const int empties = n.board.emptyCount();
    int score = pos + 4 * mobility;
    if (empties <= 20) score += (20 - empties) * discs / 2;  // 終盤ほど石数重視
    return score;
}

void orderMoves(std::vector<Pos>& moves) {
    const auto& w = weights();
    std::stable_sort(moves.begin(), moves.end(), [&w](Pos a, Pos b) {
        return w[static_cast<size_t>(toIndex(a))] > w[static_cast<size_t>(toIndex(b))];
    });
}

class Searcher {
public:
    Searcher(const AiOptions& opt, bool useDeadline)
        : opt_(opt), useDeadline_(useDeadline), deadline_(Clock::now() + opt.timeLimit) {}

    bool aborted() const { return aborted_; }
    long long nodes() const { return nodes_; }

    int negamax(const Node& n, int depth, int alpha, int beta) {
        if (checkAbort()) return 0;
        if (n.board.isFull()) return finalScore(n);
        if (depth <= 0) return evaluate(n);

        std::vector<Pos> moves = n.board.legalMoves(n.turn);
        if (moves.empty()) {
            if (!n.board.hasLegalMove(opponent(n.turn))) {
                if (!n.pending) return finalScore(n);
                Node c = n;  // 両者合法手なし → 予告イベント前倒し（パス非加算）
                c.event.apply(c.board);
                c.pending = false;
                c.counter = 0;
                return negamax(c, depth - 1, alpha, beta);
            }
            Node c = n;  // パス
            advanceTurn(c);
            return -negamax(c, depth - 1, -beta, -alpha);
        }

        orderMoves(moves);
        int best = -kInf;
        for (const Pos m : moves) {
            Node c = n;
            c.board.place(m, n.turn);
            advanceTurn(c);
            const int v = -negamax(c, depth - 1, -beta, -alpha);
            if (aborted_) return 0;
            best = std::max(best, v);
            alpha = std::max(alpha, v);
            if (alpha >= beta) break;
        }
        return best;
    }

    // ルートで最善手を選ぶ。打ち切られたら nullopt。
    std::optional<Pos> searchRoot(const Node& root, std::vector<Pos>& moves, int depth) {
        int alpha = -kInf;
        Pos best = moves.front();
        for (const Pos m : moves) {
            Node c = root;
            c.board.place(m, root.turn);
            advanceTurn(c);
            const int v = -negamax(c, depth - 1, -kInf, -alpha);
            if (aborted_) return std::nullopt;
            if (v > alpha) {
                alpha = v;
                best = m;
            }
        }
        return best;
    }

private:
    bool checkAbort() {
        if ((++nodes_ & 255) == 0) {
            if (opt_.cancel && opt_.cancel->load()) aborted_ = true;
            if (useDeadline_ && Clock::now() >= deadline_) aborted_ = true;
        }
        return aborted_;
    }

    const AiOptions& opt_;
    bool useDeadline_;
    Clock::time_point deadline_;
    long long nodes_ = 0;
    bool aborted_ = false;
};

std::optional<Pos> chooseWeak(const Game& game, Random& rng) {
    const std::vector<Pos> moves = game.legalMoves();
    if (rng.chance(80)) return moves[rng.below(static_cast<std::uint32_t>(moves.size()))];
    size_t best = 0;
    std::vector<Pos> bests;
    for (const Pos m : moves) {
        const size_t f = game.flipsFor(m).size();
        if (f > best) {
            best = f;
            bests.clear();
        }
        if (f == best) bests.push_back(m);
    }
    return bests[rng.below(static_cast<std::uint32_t>(bests.size()))];
}

}  // namespace

const AiStats& lastAiStats() { return g_stats; }

std::optional<Pos> chooseMove(const Game& game, const AiOptions& options, Random& rng) {
    g_stats = {};
    std::vector<Pos> moves = game.legalMoves();
    if (moves.empty()) return std::nullopt;
    if (options.difficulty == Difficulty::Weak) return chooseWeak(game, rng);
    if (moves.size() == 1) return moves.front();

    orderMoves(moves);
    Node root;
    root.board = game.board();
    root.turn = game.turn();
    root.counter = game.counter();
    root.event = game.nextEvent();

    if (options.difficulty == Difficulty::Normal) {
        root.pending = false;  // 中: 盤面イベントは考慮しない
        Searcher s(options, false);
        const auto r = s.searchRoot(root, moves, options.normalDepth);
        g_stats.completedDepth = r ? options.normalDepth : 0;
        g_stats.nodes = s.nodes();
        return r ? r : moves.front();
    }

    // 強: 予告イベントを読みに含む反復深化
    root.pending = true;
    Searcher s(options, true);
    Pos best = moves.front();
    // パス・前倒しイベントも深さを消費するため余裕を持たせる
    const int maxDepth = root.board.emptyCount() * 2 + 2;
    for (int depth = 1; depth <= maxDepth; ++depth) {
        const auto r = s.searchRoot(root, moves, depth);
        if (!r) break;
        best = *r;
        g_stats.completedDepth = depth;
        // 前回の最善手を先頭に（αβの効率化）
        const auto it = std::find(moves.begin(), moves.end(), best);
        std::rotate(moves.begin(), it, it + 1);
    }
    g_stats.nodes = s.nodes();
    return best;
}

}  // namespace shift
