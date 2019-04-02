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

#define LIMIT_SEC 180;

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
const int FIELD_LENGTH = 16;
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
};

struct Player
{
    int think_time;
    int ojama_num;
    char skill;

    char field[FIELD_LENGTH*FIELD_WIDTH];

    Player() {}
    inline void input()
    {
        cin >> think_time >> ojama_num >> skill;
        int tmpb;
        for (int r = 0; r < FIELD_LENGTH; ++r) for (int c = 0; c < FIELD_WIDTH; ++c)
        {
            cin >> tmpb;
            field[idx(r, c)] = set_block(tmpb);
        }
        string tmp; cin >> tmp;
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

    State() : depth(0), score(0) {}
    State(shared_ptr<State> p) {
        depth = p->depth + 1;
        score = p->score;
        for (int r = 0; r < FIELD_LENGTH; ++r) for (int c = 0; c < FIELD_WIDTH; ++c) field[idx(r, c)] = p->field[idx(r, c)];
        parent = p;
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


class Solver
{
private:
    int turn;
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


    cout << "2 0" << endl;
}

int main()
{

    // State state;
    // int h, w;
    // cin >> h >> w;
    // for (int r = 0; r < h; ++r) for (int c = 0; c < w; ++c) {
    //     cin >> state.field[idx(r, c)];
    // }
    //
    // for (int r = 0; r < h; ++r) for (int c = 0; c < w; ++c) {
    //     cout << state.field[idx(r, c)] << " \n"[c==w-1];
    // }
    //
    // cout << endl;
    //
    // while (true) {
    //     bool ok = false;
    //
    //     ok = vanish(state, h, w);
    //     for (int r = 0; r < h; ++r) for (int c = 0; c < w; ++c) {
    //         cout << state.field[idx(r, c)] << " \n"[c==w-1];
    //     }
    //     cout << endl;
    //
    //     fall(state, h, w);
    //     for (int r = 0; r < h; ++r) for (int c = 0; c < w; ++c) {
    //         cout << state.field[idx(r, c)] << " \n"[c==w-1];
    //     }
    //     cout << endl;
    //
    //     if (!ok) break;
    // }

    // for (int r = 0; r < h; ++r) for (int c = 0; c < w; ++c) {
    //     cout << state.field[idx(r, c)] << " \n"[c==w-1];
    // }


    // Solver solver;
    // solver.run();
}
