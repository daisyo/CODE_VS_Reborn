#include <bits/stdc++.h>

using namespace std;
#define rep(i,n) for (int (i)=(0);(i)<(int)(n);++(i))
using ll = long long;
using P = pair<ll, ll>;
using namespace std;

//#define LOCAL

#define LIMIT_SEC 180

#ifdef LOCAL
ofstream fout("/Users/k16039kk/git/CODE_VS_Rebon/out.txt");
#endif


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

inline int get_ojama(double d) {
    return d / 2;
}

inline int get_chain_score(int sc) {
    int ret = 0;
    double d = 1.3;
    rep(i, sc) {
        ret += floor(d);
        d *= d;
    }
    return ret;
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
        if (r == 0) return;
        char tmp[PACK_SIZE][PACK_SIZE];
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
    int score;

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
        cin >> think_time >> ojama_num >> skill >> score;
        cin.ignore();
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
    int depth;
    double score;
    char field[FIELD_LENGTH * FIELD_WIDTH];
    shared_ptr<State> parent;
    string command;

    int rensa;
    int real_score;

    State() : depth(0), score(0), parent(nullptr), command(""), rensa(0), real_score(0) {}
    State(shared_ptr<State> p) {
        depth = p->depth + 1;
        score = p->score;
        for (int r = 0; r < FIELD_LENGTH; ++r) for (int c = 0; c < FIELD_WIDTH; ++c) field[idx(r, c)] = p->field[idx(r, c)];
        parent = p;
        command = "";
        rensa = p->rensa;
        real_score = p->real_score;
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

            if (field[idx(r, c)] != blocks[EMPTY] and field[idx(r, c)] != blocks[OJAMA])
            for (int d = 0; d < 8; ++d) {
                int nr = r + dr[d], nc = c + dc[d];
                int nnr = nr + 1, nnc = nc;

                // 一個消してみる
                // 8近傍に空のセルがあって，そのマスはお邪魔ではないブロックがある
                if (ok(nr, nc) and field[idx(nr, nc)] == blocks[EMPTY] and field[idx(r, c)] != blocks[EMPTY] and field[idx(r, c)] != blocks[OJAMA]) {

                    if (!ok(nnr, nnc)) {}
                    else if (field[idx(nnr, nnc)] == blocks[EMPTY]) {
                        continue;
                    }

                    copy();
                    s.field[idx(r, c)] = blocks[EMPTY];

                    // fout << "(" << r << ", " << c << ")" << endl;
                    // rep(r, FIELD_LENGTH) rep(c, FIELD_WIDTH) {
                    //     fout << s.field[idx(r,c)] << " \n"[c == FIELD_WIDTH-1];
                    // }
                    // fout << "___" << endl;

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

                    // rep(r, FIELD_LENGTH) rep(c, FIELD_WIDTH) {
                    //     fout << s.field[idx(r,c)] << " \n"[c == FIELD_WIDTH-1];
                    // }
                    // fout << endl;

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
    int cur;
    int turn;
    Timer timer;
    vector<string> bestFirst;
public:
    void initialize();
    void run();
    void loopinput();
    void think();
};

void Solver::initialize()
{
    // cin.tie(0);
    // ios_base::sync_with_stdio(false);

    cout << myname << endl;
    cout.flush();

    for (int turn = 0; turn < MAX_TURN; ++turn)
    {
        packs[turn].input();
    }
    cur = 0;
}

void Solver::loopinput()
{
    cin >> turn;        // 現在のターン(0-index)
    cin.ignore();
    //cerr << "turn: " << turn << endl;
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

    if (cur < bestFirst.size() and players[0].ojama_num < FIELD_WIDTH) {
        cout << bestFirst[cur++] << endl;
        cout.flush();
        //swap(bestFirst[0], bestFirst[bestFirst.size()-1]);
        //bestFirst.pop_back();
        return;
    }
    // else if (turn != 0) {
    //     cout << "10 10" << endl;
    //     return;
    // }

    // 最初の状態をコピー
    State firstState;
    for (int r = 0; r < FIELD_LENGTH; ++r) for (int c = 0; c < FIELD_WIDTH; ++c) firstState.field[idx(r, c)] = players[0].field[idx(r, c)];

    if (players[0].ojama_num >= FIELD_WIDTH) {
        for (int x=0; x<FIELD_WIDTH; ++x) {
            firstState.field[idx(0, x)] = blocks[OJAMA];
        }
    }

    // #ifdef LOCAL
    // fout << "prev:" << endl;
    // rep(r, FIELD_LENGTH) rep(c, FIELD_WIDTH) fout << firstState.field[idx(r, c)] << " \n"[c == FIELD_WIDTH-1];
    // fout << endl;
    // #endif

    fall(firstState);

    //
    int maxturn = 17;
    priority_queue<State> hstate[maxturn + 1];
    hstate[0].push(firstState);

    //
    double diff = 10;//(double)(LIMIT_SEC - timer.get_time()) / (MAX_TURN - turn);
    if (turn != 0) {
        diff = 1;//(double)(LIMIT_SEC - timer.get_time()) / (MAX_TURN - turn);;
        maxturn = 13;
    }
    const double limit = timer.get_time() + diff;
    const int chokudai_width = 1;


    if (turn + maxturn >= 500) {
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

                    int tmp = 0;
                    for (int c = 0; c < FIELD_WIDTH; ++c) {
                        for (int r = 0; r < FIELD_LENGTH; ++r) {
                            if (next.field[idx(r, c)] != blocks[EMPTY]) {
                                tmp += pow(FIELD_LENGTH-r, 2);
                                break;
                            }
                        }
                    }
                    int tmp2 = 0;
                    for (int c = 0; c < FIELD_WIDTH; ++c) {
                        for (int r = 0; r < FIELD_LENGTH; ++r) {
                            if (next.field[idx(r, c)] != blocks[EMPTY] and next.field[idx(r, c)] != blocks[OJAMA]) {
                                tmp2++;
                            }
                        }
                    }
                    int tmp3 = 0;
                    for (int c = 0; c < FIELD_WIDTH; ++c) {
                        for (int r = 0; r < FIELD_LENGTH; ++r) {
                            if (now.field[idx(r, c)] != blocks[EMPTY] and now.field[idx(r, c)] != blocks[OJAMA]) {
                                tmp3++;
                            }
                        }
                    }

                    int tmp5 = 0;
                    bitset<FIELD_LENGTH*FIELD_WIDTH+1> cnt;
                    rep(r, FIELD_LENGTH) rep(c, FIELD_WIDTH) {
                        if (next.field[idx(r, c)] == blocks[5]) {
                            tmp5++;

                            int dr[] = { 1, 0, -1, 0, 1, 1, -1, -1};
                            int dc[] = { 0, 1, 0, -1, -1, 1, 1, -1};
                            rep(d, 8) {
                                int nr = r + dr[d], nc = c + dc[d];
                                if (0 <= nr and nr < FIELD_LENGTH and 0 <= nc and nc < FIELD_WIDTH) {
                                    if (next.field[idx(nr, nc)] != blocks[OJAMA] and next.field[idx(nr, nc)] != blocks[EMPTY]) {
                                        cnt[idx(nr, nc)] = 1;
                                    }
                                }
                            }
                        }
                    }

                    int nc = calc_score(next);
                    next.rensa = max({next.rensa, chain});

                    if (chain >= 10) next.score += chain*100000;
                    else next.score += 1000 * nc - tmp*10 - chain * 10 + tmp2 * 50;
                    next.real_score += get_chain_score(chain);
                    next.command = to_string(x) + " " + to_string(r);

                    hstate[t + 1].push(next);
                }
            }
        }
    }

    if (players[0].skill >= 80) {
        int num = 0;
        for (int r = 0; r < FIELD_LENGTH; ++r) for (int c = 0; c < FIELD_WIDTH; ++c) {
            if (players[0].field[idx(r, c)] == '5') num++;
        }
        if (num >= 1) {
            cout << "S" << endl;
            cout.flush();
            return;
        }
    }

    if (maxchain >= 10) {
        cout << fireCommand << endl;
        cout.flush();
        return;
    }

    cerr << "max chain: " << maxchain << endl;

    if (hstate[maxturn].empty()) {
        cerr << "DAME" << endl;
        cout << "2 0" << endl;
        cout.flush();
        return;
    }
    //
    State bestState = hstate[maxturn].top();
    cerr << "depth: " <<  bestState.depth << endl;
    cerr << bestState.score << endl;
    cerr << "best rensa: " << bestState.rensa << endl;

    // rep(r, FIELD_LENGTH) rep(c, FIELD_WIDTH) {
    //     fout << bestState.field[idx(r, c)] << " \n"[c == FIELD_WIDTH-1];
    // }
    // fout << endl;


    // if (turn == 0) {
    //     cerr << "Real Score = " << bestState.real_score << endl;
    // }

    // 0と1の両方の可能性があります
    // bestFirstを使うときは0です
    while (bestState.parent != nullptr and ( (turn == 0 and bestState.depth > 0) or (turn != 0 and bestState.depth > 1))) {
        if (turn == 0) bestFirst.push_back(bestState.command);
        bestState = *bestState.parent;
    }

    if (bestFirst.size() and turn == 0) {
        if (turn == 0) reverse(bestFirst.begin(), bestFirst.end());
        cerr << "num = " << bestFirst.size() << endl;
        cout << bestFirst[cur++] << endl;
        cout.flush();
        //swap(bestFirst[0], bestFirst[bestFirst.size()-1]);
        //bestFirst.pop_back();

        //fout << "max chain " << bestState.rensa << endl;


        return;
    }

    //エラー用
    if (bestState.command == "") {
        //cerr << "DAME" << endl;
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

// // debug用
// int main() {
//     auto sc = [](int sc) {
//         ll ret = 0;
//         double tmp = 1.3;
//         rep(i, sc) {
//             ret += floor(tmp);
//             tmp *= 1.3;
//         }
//         return ret;
//     };
//
//     auto tmp = [](ll b) {
//         return floor(25LL * 1.0 * powl(2, b*1.0/12.0));
//     };
//
//     for (int i=0; i<100; ++i) {
//         cout << i << "blocks: " << (ll)floor(tmp(i)*1.0/2.0) << endl;
//     }
// }
