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

// parameter
const string myname = "daisyo"; // 名前

const int MAX_TURN = 500;
const int PACK_SIZE = 2;
const int FIELD_LENGTH = 16;
const int FIELD_WIDTH = 10;

const int EMPTY = 0;
const int OJAMA = 11;
const char blocks[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'X', 'B' };

inline int idx(int r, int c) { return r * FIELD_WIDTH + c; }

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

    char field[FIELD_LENGTH][FIELD_WIDTH];

    Player() {}
    void input()
    {
        cin >> think_time >> ojama_num >> skill;
        for (int r = 0; r < FIELD_LENGTH; ++r) for (int c = 0; c < FIELD_WIDTH; ++c) cin >> field[r][c];
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

    State() : depth(-1), score(0) {}
    // State(shared_ptr<State> prev)
    // {
    //     parent = prev;
    //     depth = perv->depth + 1;
    //     score = prev->score;
    //     for (char i=0; i<FIELD_LENGTH*FIELD_WIDTH; ++i) field[i] = prev->field[i];
    // }
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

// 今の状態から次のターンの状態を返す
// State simulator(Staet& now, Pack& p, int& r) {
//
//
//
// }

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

    cout << "2 0" << endl;
}

int main()
{
    // Solver solver;
    // solver.run();
}
