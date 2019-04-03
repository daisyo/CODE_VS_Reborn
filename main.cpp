#include <bits/stdc++.h>
using namespace std;
#define rep(i,n) for (int (i)=(0);(i)<(int)(n);++(i))
using ll = long long;
using P = pair<ll, ll>;
using namespace std;

template<class T> void vin(vector<T>& v, int n) {
    v.resize(n);
    for (int i = 0; i < n; ++i) {
        cin >> v[i];
    }
}

#define LIMIT_SEC 180

// SEC
class Timer {
    chrono::high_resolution_clock::time_point start;
    double limit;
public:
    Timer() {
        start = chrono::high_resolution_clock::now();
        limit = LIMIT_SEC;
    }
    Timer(double limit) :limit(limit) {
        start = chrono::high_resolution_clock::now();
    }
    double get_time() {
        return chrono::duration<double>(chrono::high_resolution_clock::now() - start).count();
    }
    bool time_over() {
        if (get_time() > limit) {
            return true;
        }
        return false;
    }
    void set_start() {
        start = chrono::high_resolution_clock::now();
    }
    double get_limit() const {
        return limit;
    }
};


class XorShift {
public:
    unsigned long x, y, z, w;
    XorShift() {
        x = 123456789; y = 362436069; z = 521288629; w = 88675123;
    }
    XorShift(unsigned long seed) {
        XorShift();
        w = seed;
        for (int i = 0; i < 100; ++i) (*this)();
    }
    unsigned long operator()() {
        unsigned long t = x ^ (x << 11);
        x = y; y = z; z = w;
        return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    }
};

XorShift rnd;

// parameter
const string myname = "daisyo"; // 名前

const int MAX_TURN = 500;
const int PACK_SIZE = 2;
const int LENGTH_OFFSET = 2;
const int FIELD_LENGTH = 16 + LENGTH_OFFSET;
const int FIELD_WIDTH = 10;

const int EMPTY = 0;
const int VANISH = 10;
const int OJAMA = 11;
const char blocks[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'X', 'B' };

inline int idx(int r, int c) { return r * FIELD_WIDTH + c; }
inline char set_block(int b)
{
    return blocks[b];
}

// パック
struct Pack
{
    char pack[PACK_SIZE][PACK_SIZE];

    //
    Pack() {}
    Pack(char p1, char p2, char p3, char p4) {
        pack[0][0] = p1;
        pack[0][1] = p2;
        pack[1][0] = p3;
        pack[1][1] = p4;
    }

    void input() {
        for (int r = 0; r < PACK_SIZE; ++r) for (int c = 0; c < PACK_SIZE; ++c) cin >> pack[r][c];
        string tmp; cin >> tmp;
    }

    // 愚直
    void rotate(int r=1) {

        int tmp[PACK_SIZE][PACK_SIZE];
        for (int i = 0; i < r; ++i) {
            for (int a1=0; a1<PACK_SIZE; ++a1) for (int a2=0; a2<PACK_SIZE; ++a2) {
                tmp[a1][a2] = pack[PACK_SIZE - 1 - a2][a1];
            }
            for (int a1=0; a1<PACK_SIZE; ++a1) for (int a2=0; a2<PACK_SIZE; ++a2) {
                pack[a1][a2] = tmp[a1][a2];
            }
        }

        for (int a1=0; a1<PACK_SIZE; ++a1) for (int a2=0; a2<PACK_SIZE; ++a2) {
            pack[a1][a2] = tmp[a1][a2];
        }
    }
};

struct Player
{
    int think_time;
    int ojama_num;
    int skill;

    char field[FIELD_LENGTH*FIELD_WIDTH];

    Player() {
        for (int r = 0; r < FIELD_LENGTH; ++r)
        {
            for (int c = 0; c < FIELD_WIDTH; ++c)
            {
                field[idx(r, c)] = blocks[EMPTY];
            }
        }
    }
    inline void input()
    {
        cin >> think_time >> ojama_num >> skill;
        cin.ignore();
        cerr << think_time << ", " << ojama_num << ", " << skill << endl;
        int tmpb;
        for (int r = LENGTH_OFFSET; r < FIELD_LENGTH; ++r) for (int c = 0; c < FIELD_WIDTH; ++c)
        {
            cin >> tmpb;
            field[idx(r, c)] = set_block(tmpb);
        }
        cin.ignore();
        string tmp; cin >> tmp;
        cin.ignore();
    }
};

Pack packs[MAX_TURN];
Player players[2];

struct State
{
    char depth;
    int score;
    char field[FIELD_LENGTH * FIELD_WIDTH];
    shared_ptr<State> parent;
    string command;

    State() : depth(0), score(0), parent(nullptr), command("") {}
    State(shared_ptr<State> p) {
        depth = p->depth + 1;
        score = p->score;
        for (int r = 0; r < FIELD_LENGTH; ++r) for (int c = 0; c < FIELD_WIDTH; ++c) field[idx(r, c)] = p->field[idx(r, c)];
        parent = p;
        command = "";
    }

    // 置いてみる
    // だめだったらfalseを返す
    bool set(Pack& p, int x) {
        for (int c=x; c<x+PACK_SIZE; ++c) {
            for (int r=0; r<PACK_SIZE; ++r) {
                if (field[idx(r, c)] != blocks[EMPTY]) return false;
                field[idx(r, c)] = p.pack[r][c-x];
            }
        }
        return true;
    }

    // だめだったらtrue
    bool is_over() {
        for (int c = 0; c < FIELD_WIDTH; ++c) {
            for (int r = LENGTH_OFFSET-1; r >= 0; --r) {
                if (field[idx(r, c)] != blocks[EMPTY]) return true;
            }
        }
        return false;
    }

    bool operator<(const State& s) const {
        return score < s.score;
    }
};


// 落とす処理
inline void fall(State& s, int h=FIELD_LENGTH, int w=FIELD_WIDTH)
{
    for (int c = 0; c < w; ++c)
    {
        for (int r = h-2; r >= 0; --r)
        {
            // 今見ている場所にブロックがあって，その下にブロックがない場合
            // そのブロックを一番下に移動
            if (s.field[idx(r, c)] != blocks[EMPTY] and s.field[idx(r+1, c)] == blocks[EMPTY])
            {
                char tmp = s.field[idx(r, c)];
                s.field[idx(r, c)] = blocks[EMPTY];
                int cur = r+1;
                while (cur+1 < h and s.field[idx(cur+1, c)] == blocks[EMPTY]) cur++;
                s.field[idx(cur, c)] = tmp;
            }
        }
    }
}

inline bool vanish_ok(char s1, char s2)
{
    return (s1-'0') + (s2-'0') == VANISH;
}

// 消す処理（愚直）
inline bool vanish(State& s, int h=FIELD_LENGTH, int w=FIELD_WIDTH)
{
    bool update = false;
    bitset<FIELD_LENGTH*FIELD_WIDTH> vanishflag;
    int dc[] = { 1, 0, -1, 0, 1, 1, -1, -1 };
    int dr[] = { 0, 1, 0, -1, 1, -1, 1, -1 };

    auto ok = [&](int r, int c) {
        return 0 <= r and r < h and 0 <= c and c < w;
    };

    // 何が消えるのか見る
    for (int r = 0; r < h; ++r)
    {
        for (int c = 0; c < w; ++c)
        {
            for (int d = 0; d < 8; ++d)
            {
                int nr = r + dr[d], nc = c + dc[d];
                if (ok(nr, nc) and vanish_ok(s.field[idx(r, c)], s.field[idx(nr, nc)]))
                {
                    update = true;
                    vanishflag[idx(r, c)] = vanishflag[idx(nr, nc)] = true;
                }
            }
        }
    }

    // 消す
    for (int r = 0; r < h; ++r)
    {
        for (int c = 0; c < w; ++c)
        {
            if (vanishflag[idx(r, c)]) s.field[idx(r, c)] = blocks[EMPTY];
        }
    }

    return update;
}

int calc_score(State s) {
    int field[FIELD_LENGTH*FIELD_WIDTH];
    int max_chain = 0;

    for (int r = 0; r < FIELD_LENGTH; ++r)
    {
        for (int c = 0; c < FIELD_WIDTH; ++c)
        {
            field[idx(r, c)] = s.field[idx(r, c)];
        }
    }

    // copy
    auto copy = [&]() {
        for (int r = 0; r < FIELD_LENGTH; ++r) {
            for (int c = 0; c < FIELD_WIDTH; ++c) {
                s.field[idx(r, c)] = field[idx(r, c)];
            }
        }
    };

    int dr[] = { 1, 0, -1, 0, 1, 1, -1, -1 };
    int dc[] = { 0, 1, 0, -1, 1, -1, 1, -1 };

    auto ok = [&](int r, int c) {
        return 0 <= r and r < FIELD_LENGTH and 0 <= c and c < FIELD_WIDTH;
    };

    for (int r = 0; r < FIELD_LENGTH; ++r) {
        for (int c = 0; c < FIELD_WIDTH; ++c) {
            for (int d = 0; d < 8; ++d) {
                int nr = r + dr[d], nc = c + dc[d];

                // 一個消してみる
                if (ok(nr, nc) and field[idx(nr, nc)] == blocks[EMPTY] and field[idx(r, c)] != blocks[EMPTY] and field[idx(r, c)] != blocks[OJAMA]) {

                    copy();
                    s.field[idx(r, c)] = blocks[EMPTY];

                    bool update = true;
                    fall(s);
                    int chain = 1;
                    while (update)
                    {
                        update = vanish(s);
                        if (update) {
                            chain++;
                            fall(s);
                        }
                    }

                    max_chain = max(max_chain, chain);
                }
            }
        }
    }

    return max_chain;
}

class Solver
{
private:
    int turn;
    Timer timer;
public:
    void initialize();
    void run();
    void loopinput();
    void think();
};

void Solver::initialize()
{
    cin.tie(0);
    ios_base::sync_with_stdio(false);

    cout << myname << endl;

    for (int turn = 0; turn < MAX_TURN; ++turn)
    {
        packs[turn].input();
    }
}

void Solver::loopinput()
{
    cin >> turn;        // 現在のターン(0-index)
    cin.ignore();
    cerr << "turn: " << turn << endl;
    players[0].input();
    players[1].input();
}

void Solver::run()
{
    // 初期化
    initialize();

    // ゲームループ
    while (true)
    {
        loopinput();
        think();
    }
}

// メインロジック
void Solver::think()
{

    // 最初の状態をコピー
    State firstState;
    for (int r = 0; r < FIELD_LENGTH; ++r) for (int c = 0; c < FIELD_WIDTH; ++c) firstState.field[idx(r, c)] = players[0].field[idx(r, c)];
    //
    int maxturn = 10;
    priority_queue<State> hstate[maxturn + 1];
    hstate[0].push(firstState);
    //
    const double diff = (double)(LIMIT_SEC - timer.get_time()) / (MAX_TURN - turn);
    const double limit = timer.get_time() + diff;
    const int chokudai_width = 1;

    if (turn + 10 >= 500) {
        maxturn = MAX_TURN - turn;
    }

    double current_time = timer.get_time();

    string fireCommand = "";
    int maxchain = 0;

    while ((current_time = timer.get_time()) < limit) {
        for (int t = 0; t < maxturn; ++t)
        {
            for (int i = 0; i < chokudai_width; ++i)
            {
                if (hstate[t].empty()) break;

                State now = hstate[t].top(); hstate[t].pop();

                for (int x = 0; x <= FIELD_WIDTH-PACK_SIZE; ++x) for (int r = 0; r < 4; ++r) {

                    Pack pack = packs[turn + t]; pack.rotate(r);
                    State next(make_shared<State>(now));

                    bool ok = next.set(pack, x);
                    if (!ok) continue;

                    fall(next);
                    if (next.is_over()) {
                        continue;
                    }

                    // 落とせた
                    bool update = true;
                    int chain = 0;
                    while (update)
                    {
                        update = vanish(next);
                        if (update) {
                            fall(next);
                            chain++;
                        }
                    }

                    if (t == 0 and maxchain < chain) {
                        maxchain = chain;
                        fireCommand = to_string(x) + " " + to_string(r);
                    }

                    //
                    next.score += calc_score(next);
                    next.command = to_string(x) + " " + to_string(r);
                    hstate[t + 1].push(next);
                }
            }
        }
    }

    if (maxchain >= 5) {
        cout << fireCommand << endl;
        cout.flush();
        return;
    }
    if (players[0].skill >= 80) {
        cout << "S" << endl;
        cout.flush();
        return;
    }

    cerr << "max chain: " << maxchain << endl;

    if (hstate[maxturn].empty()) {
        cout << "2 0" << endl;
        cout.flush();
        return;
    }

    State bestState = hstate[maxturn].top();
    while (bestState.parent != nullptr and bestState.depth > 1) {
        bestState = *bestState.parent;
    }

    //エラー用
    if (bestState.command == "") {
        cout << "2 0" << endl;
        cout.flush();
        return;
    }

    cout << bestState.command << endl;
    cout.flush();
}

int main()
{

    Solver solver;
    solver.run();
}
