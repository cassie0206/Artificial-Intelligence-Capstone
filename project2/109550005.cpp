#include "STcpClient.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <time.h>
#include <utility>
#include <random>
#include <ctime>
#include <algorithm>

using namespace std;

clock_t start;

/*
    input position (x,y) and direction
    output next node position on this direction
*/
vector<int> Next_Node(int pos_x, int pos_y, int direction)
{
    vector<int> result; // 建立一個大小為 2 的 vector，用來儲存座標
    result.resize(2);
    if (pos_y % 2 == 1)
    {
        if (direction == 1)
        {
            result[0] = pos_x;
            result[1] = pos_y - 1;
        }
        else if (direction == 2)
        {
            result[0] = pos_x + 1;
            result[1] = pos_y - 1;
        }
        else if (direction == 3)
        {
            result[0] = pos_x - 1;
            result[1] = pos_y;
        }
        else if (direction == 4)
        {
            result[0] = pos_x + 1;
            result[1] = pos_y;
        }
        else if (direction == 5)
        {
            result[0] = pos_x;
            result[1] = pos_y + 1;
        }
        else if (direction == 6)
        {
            result[0] = pos_x + 1;
            result[1] = pos_y + 1;
        }
    }
    else
    {
        if (direction == 1)
        {
            result[0] = pos_x - 1;
            result[1] = pos_y - 1;
        }
        else if (direction == 2)
        {
            result[0] = pos_x;
            result[1] = pos_y - 1;
        }
        else if (direction == 3)
        {
            result[0] = pos_x - 1;
            result[1] = pos_y;
        }
        else if (direction == 4)
        {
            result[0] = pos_x + 1;
            result[1] = pos_y;
        }
        else if (direction == 5)
        {
            result[0] = pos_x - 1;
            result[1] = pos_y + 1;
        }
        else if (direction == 6)
        {
            result[0] = pos_x;
            result[1] = pos_y + 1;
        }
    }
    return result;
}

struct Step
{
    int x;
    int y;
    int numOfStep;
    int dir;
    Step() : x(-1), y(-1), numOfStep(-1), dir(-1) {}
    Step(int x, int y, int numOfStep, int dir) : x(x), y(y), numOfStep(numOfStep), dir(dir) {}
};

bool checkMoveValidation(int state[12][12], Step move)
{
    // move = [x, y, move # of step, move direction]
    if (state[move.x][move.y] != 0)
        return false;
    if (move.numOfStep < 1 || move.numOfStep > 3)
        return false;

    vector<int> result(2);
    result[0] = move.x;
    result[1] = move.y;

    for (int i = 0; i < move.numOfStep - 1; i++)
    {
        result = Next_Node(result[0], result[1], move.dir);
        if (result[0] < 0 || result[0] > 11 || result[1] < 0 || result[1] > 11 || state[result[0]][result[1]] != 0)
            return false;
    }
    // cout << move.x << " " << move.y << " " << move.numOfStep << " " << move.dir << "\n";
    return true;
}

class MCTS
{
public:
    struct Node
    {
        int Tn;
        int x;
        int raveTn;
        int raveX;
        int state[12][12];                  // current board state
        Step parent_move;                   // (x, y)
        vector<vector<Node*>> child2board; // keep action according to the board position
        vector<Step> legal;                 // keep remain empty coordinate(x, y)
        vector<Node*> children;

        Node(int s[12][12], std::default_random_engine& engine, bool isRoot, vector<int>& res) : Tn(0), x(0), raveTn(0), raveX(0)
        {
            memcpy(state, s, 12 * 12 * sizeof(int));
            child2board.resize(12, vector<Node*>(12, nullptr));
            Step keep;
            int num = 0, numOf2 = 0, numOf3 = 0;

            for (int i = 0; i < 12; i++)
            {
                for (int j = 0; j < 12; j++)
                {
                    if (state[i][j] != 0)
                    {
                        continue;
                    }
                    else
                    {
                        Step move = Step(i, j, 1, 1);
                        legal.push_back(move);
                        num++;
                    }
                    for (int k = 1; k < 7; k++)
                    {
                        int l = 2;
                        for (; l < 4; l++)
                        {
                            Step newMove = Step(i, j, l, k);
                            if (checkMoveValidation(state, newMove))
                            {
                                // cout << newMove.x << " " << newMove.y << " " << newMove.numOfStep << " " << newMove.dir << "\n";
                                legal.push_back(newMove);
                                keep = newMove;
                            }
                            else
                            {
                                break;
                            }
                        }
                        if (l == 3) {
                            numOf2++;
                        }
                        if (l == 4)
                            numOf3++;
                    }
                }
            }

            if (isRoot && ((numOf2 + numOf3 == 2) || (numOf2 + numOf3 == 4)))
            {
                res.resize(4);
                if (numOf2 + numOf3 == 2)
                {
                    if (num % 2 == 0)
                    {
                        res[0] = keep.x;
                        res[1] = keep.y;
                        res[2] = 1;
                        res[3] = 1;
                    }
                    else
                    {
                        res[0] = keep.x;
                        res[1] = keep.y;
                        res[2] = keep.numOfStep;
                        res[3] = keep.dir;
                    }
                    //cout<<"222222\n";
                }
                else
                {
                    if (keep.numOfStep == 2 && num > 4) {
                        for (Step move : legal) {
                            if (move.numOfStep != 1)
                                continue;
                            bool flag = false;
                            for (int k = 1; k < 7; k++) {
                                Step newStep = Step(move.x, move.y, 2, k);
                                if (checkMoveValidation(state, newStep)) {
                                    flag = true;
                                    break;
                                }
                            }
                            if (!flag) {
                                res[0] = move.x;
                                res[1] = move.y;
                                res[2] = move.numOfStep;
                                res[3] = move.dir;
                                break;
                            }
                        }
                        //cout << "legal: " << res[0] << " " << res[1] << " " << res[2] << " " << res[3] << "\n";
                    }
                    else if(keep.numOfStep == 2 && num == 4){
                        res[0] = keep.x;
                        res[1] = keep.y;
                        res[2] = 1;
                        res[3] = keep.dir;
                        //cout<<"legal num==4\n";
                    }
                    else {
                        //cout << "num2: " << num % 2 << "\n";
                        if (num % 2 == 0)
                        {
                            res[0] = keep.x;
                            res[1] = keep.y;
                            res[2] = keep.numOfStep;
                            res[3] = keep.dir;
                        }
                        else
                        {
                            res[0] = keep.x;
                            res[1] = keep.y;
                            res[2] = 2;
                            res[3] = keep.dir;
                        }
                    }
                }
                /*for (int i = 0; i < 4; i++) {
                    cout << res[i] << " ";
                }*/
            }
            /*if (isRoot)
                cout << "2 + 3: " << numOf2 + numOf3 << "size: " << res.size() << "\n";*/

            std::shuffle(legal.begin(), legal.end(), engine);
        }
    };

    MCTS()
    {
        srand(time(NULL));
        engine.seed(rand() % 100000);
        // engine.seed(1234);
        visited.resize(12, vector<int>(12, 0));
    }

    int getTn(int x, int y)
    {
        if (root->child2board[x][y])
        {
            // cout << root->child2board[x][y]->parent_move.x << " " << root->child2board[x][y]->parent_move.y << endl;
            return root->child2board[x][y]->Tn;
        }
        else
            return 0;
    }

    void getBestChild(vector<int>& step)
    {
        if (res.size() == 4)
        {
            step = res;
            res.clear();
            return;
        }

        int max_count = -1;
        Node* bestChild = nullptr;
        for (Node* child : root->children)
        {
            int Tn = child->Tn;
            //cout << child->parent_move.x << " " << child->parent_move.y << " " << Tn << "\n";
            if (max_count < Tn)
            {
                max_count = Tn;
                bestChild = child;
            }
        }

        step[0] = bestChild->parent_move.x;
        step[1] = bestChild->parent_move.y;
        step[2] = bestChild->parent_move.numOfStep;
        step[3] = bestChild->parent_move.dir;
    }

    void print_check()
    {
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 12; j++)
            {
                if (root->child2board[i][j])
                {
                    cout << "x: " << i << "y: " << j << endl;
                    cout << "x: " << root->child2board[i][j]->parent_move.x << "y: " << root->child2board[i][j]->parent_move.y << "double check x: " << root->child2board[i][j]->parent_move.x << "numOfStep: " << root->child2board[i][j]->parent_move.numOfStep << "dir: " << root->child2board[i][j]->parent_move.dir << endl;
                }
            }
        }
    }

    void run(int state[12][12], clock_t start, int time_limit)
    {
        root = new Node(state, engine, true, res);
        if (res.size() == 4) {
            return;
        }

        clock_t end;

        while (1)
        {
            traverse(root);
            end = clock();

            if (((double)(end - start)) / CLOCKS_PER_SEC >= time_limit)
                break;
        }
    }

    int traverse(Node* node, bool isOpponent = false)
    {
        while (!node->legal.empty() && !checkMoveValidation(node->state, node->legal.back()))
            node->legal.pop_back();
        if (!node->legal.empty())
        {
            // cout << "before next expand:\n";
            // print_check();
            Node* leaf = expand(node);
            // cout << "after next expand:\n";
            // print_check();
            int result = simulate(leaf->state, !isOpponent);
            backpropagate(leaf, result);
            backpropagate(node, result);
            return result;
        }
        else
        {
            int result;
            if (node->children.empty())
            {
                result = simulate(node->state, isOpponent);
            }
            else
            {
                Node* nextNode = select(node, isOpponent);
                result = traverse(nextNode, !isOpponent);
                visited[nextNode->parent_move.x][nextNode->parent_move.y] = 1;
            }
            backpropagate(node, result);
            return result;
        }
    }

    Node* select(Node* node, bool isOpponent)
    {
        double max_UCT = -1;
        Node* bestChild = nullptr;

        for (Node* child : node->children)
        {
            if (child->Tn == 0)
                return child;
            double val;
            calculate_UCT(*child, node->Tn, isOpponent, val);
            if (max_UCT < val)
            {
                max_UCT = val;
                bestChild = child;
            }
        }

        if (max_UCT == -1)
        {
            cout << "selection exit\n";
            exit(0);
        }

        return bestChild;
    }

    Node* expand(Node* node)
    {
        // randomly pick up empty position (x, y)
        auto it = node->legal.back();
        node->legal.pop_back();

        // place the legal move
        int after[12][12];
        memcpy(after, node->state, 12 * 12 * sizeof(int));
        vector<int> result(2);
        result[0] = it.x;
        result[1] = it.y;
        // let player = 3 be MCTS simualtion player
        after[result[0]][result[1]] = 3;

        for (int j = 0; j < it.numOfStep - 1; j++)
        {
            result = Next_Node(result[0], result[1], it.dir);
            after[result[0]][result[1]] = 3;
        }
        // if (keep > 1)
        //     cout << "expand length: " << keep << "\n";
        /*cout << "check after expand board:\n";
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 12; j++) {
                cout << tmp[j][i] << " ";
            }
            cout << "\n";
        }*/
        // cout << "expand step: " << it.first << " " << it.second << " " << cur_length << " " << legal_dir[0] << "\n";
        Node* newNode = new Node(after, engine, false, res);
        Step newStep = Step(it.x, it.y, it.numOfStep, it.dir);
        newNode->parent_move = newStep;
        node->children.push_back(newNode);
        node->child2board[it.x][it.y] = newNode;

        return node->children.back();
    }

    int simulate(const int state[12][12], bool isOpponent)
    {
        vector<pair<int, int>> empty;
        for (int i = 0; i < 12; i++)
        {
            for (int j = 0; j < 12; j++)
            {
                if (state[i][j] == 0)
                    empty.push_back(pair<int, int>(i, j));
            }
        }

        int after[12][12];
        memcpy(after, state, 12 * 12 * sizeof(int));
        int n = empty.size();
        if (n == 0)
        {
            return isOpponent;
        }

        while (1)
        {
            int i = 0;
            // int tmp[12][12];
            // memcpy(tmp, after, 12 * 12 * sizeof(int));
            // Step* nextMove = nullptr;

            while (i < n)
            {
                // randomly pick up an empty space
                std::uniform_int_distribution<int> uniform(i, n - 1);
                int index = uniform(engine);
                auto it = empty[index];

                if (after[it.first][it.second] != 0)
                {
                    // maybe be occupied by 2 or 3 step
                    swap(empty[index], empty[i]);
                    i++;
                }
                else
                {
                    vector<int> result(2);
                    result[0] = it.first;
                    result[1] = it.second;
                    // let player = 3 be MCTS simualtion player
                    after[result[0]][result[1]] = 3;
                    // randomly pick max distance
                    uniform_int_distribution<int> uniform(1, 3);
                    int max_step = uniform(engine);
                    // randomly pick direction
                    uniform_int_distribution<int> uniform_dir(1, 6);
                    int legal_dir = uniform_dir(engine);
                    int cur_length = 1;

                    for (int j = 0; j < max_step - 1; j++)
                    {
                        result = Next_Node(result[0], result[1], legal_dir);
                        if (result[0] >= 0 && result[0] < 12 && result[1] >= 0 && result[1] < 12 && after[result[0]][result[1]] == 0)
                        {
                            after[result[0]][result[1]] = 3;
                            cur_length++;
                        }
                        else
                            break;
                    }

                    swap(empty[index], empty[n - 1]);

                    break;
                }
            }

            if (i >= n - 1)
                return isOpponent;

            isOpponent = !isOpponent;
            n--;
        }
    }

    void backpropagate(Node* node, int result)
    {
        node->Tn++;
        node->x += result;
        /*for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 12; j++) {
                if (visited[i][j] == 1 && node->child2board[i][j] != nullptr) {
                    node->child2board[i][j]->raveTn++;
                    node->child2board[i][j]->raveX += result;
                }
            }
        }*/
    }

    void calculate_UCT(Node& node, int N, bool isOpponent, double& UCT_val)
    {
        if (node.Tn == 0)
            return;
        UCT_val = (double)((double)node.x / node.Tn) + 0.5 * (double)sqrt((double)log((double)N) / node.Tn);
    }

private:
    Node* root;
    double c = 0.5;
    // vector<Node> nodePool;
    vector<vector<int>> visited;
    default_random_engine engine;
    uniform_int_distribution<int> uniform;
    vector<int> res;
};

/*
    輪到此程式移動棋子
    mapStat : 棋盤狀態為 12*12矩陣, 0=可移動區域, -1=障礙, 1~2為玩家1~2佔領區域
    gameStat : 棋盤歷史順序
    return Step
    Step : 4 elements, [x, y, l, dir]
            x, y 表示要畫線起始座標
            l = 線條長度(1~3)
            dir = 方向(1~6),對應方向如下圖所示
              1  2
            3  x  4
              5  6
    x -> col, y -> row
*/
vector<int> GetStep(int mapStat[12][12], int gameStat[12][12])
{
    vector<int> step;
    step.resize(4);
    /*Please write your code here*/
    // TODO
    //cout << "109550005_MCTS\n";
    clock_t start = clock();
    MCTS* mcts = new MCTS();
    mcts->run(mapStat, start, 5.9);

    /*int max_count = -1;
    int key_x = 0, key_y = 0;
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            int Tn = mcts->getTn(i, j);
            if (max_count < Tn) {
                max_count = Tn;
                key_x = i;
                key_y = j;
            }

        }
    }*/
    // cout << max_count << endl;
    mcts->getBestChild(step);

    return step;
}

int main()
{
    cin.tie(0);
    cout.tie(0);
    cin.sync_with_stdio(0);

    int id_package;
    int mapStat[12][12];
    int gameStat[12][12];

    while (true)
    {
        if (GetBoard(id_package, mapStat, gameStat))
            break;

        std::vector<int> step = GetStep(mapStat, gameStat);
        SendStep(id_package, step);
    }
}
