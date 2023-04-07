#include "STcpClient_1.h"
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

int checkMoveValidation(int state[12][12], Step move, default_random_engine& engine)
{
    // move = [x, y, move # of step, move direction]
    if (state[move.x][move.y] != 0)
        return -1;
    if (move.dir < 1 || move.dir > 6)
        return -1;

    vector<int> result(2);
    result[0] = move.x;
    result[1] = move.y;

    int max_distance = 1;
    uniform_int_distribution<int> uniform(1, 3);
    int max_step = uniform(engine);

    for (int i = 0; i < max_step - 1; i++)
    {
        result = Next_Node(result[0], result[1], move.dir);
        if (result[0] < 0 || result[0] > 11 || result[1] < 0 || result[1] > 11 || state[result[0]][result[1]] != 0)
            break;
        else
            max_distance++;
    }
    //cout << move.x << " " << move.y << " " << move.numOfStep << " " << move.dir << "\n";
    return max_distance;
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
        Step parent_move;                 // (x, y)
        vector<vector<Node*>> child2board; // keep action according to the board position
        vector<pair<int, int>> legal;                 // keep remain empty coordinate(x, y)
        vector<Node*> children;

        Node(int s[12][12], std::default_random_engine& engine) : Tn(0), x(0), raveTn(0), raveX(0)
        {
            memcpy(state, s, 12 * 12 * sizeof(int));
            child2board.resize(12, vector<Node*>(12, nullptr));
            vector<int> legal_dir = { 1, 2, 3, 4, 5, 6 };

            for (int i = 0; i < 12; i++)
            {
                for (int j = 0; j < 12; j++)
                {
                    if (state[i][j] == 0)
                        legal.push_back(pair<int, int>(i, j));
                    /*shuffle(legal_dir.begin(), legal_dir.end(), engine);
                    for (int k = 0; k < 6; k++)
                    {
                        for(int n = 1; n < 4; n++)
                        {
                            Step tmp = Step(i, j, n, legal_dir[k]);
                            int max_dis = checkMoveValidation(state, tmp);
                            if(max_dis == n){
                                legal.push_back(tmp);
                                cout << tmp.x << " " << tmp.y << " " << tmp.numOfStep << " " << tmp.dir << "\n";
                                break;
                            }
                        }
                        Step tmp = Step(i, j, 3, legal_dir[k]);
                        int max_dis = checkMoveValidation(state, tmp);

                        if (max_dis != -1) {
                            std::uniform_int_distribution<int> uniform(1, max_dis);
                            int dis = uniform(engine);
                            tmp.numOfStep = dis;
                            legal.push_back(tmp);

                            cout << tmp.x << " " << tmp.y << " " << tmp.numOfStep << " " << tmp.dir << "\n";
                            break;
                        }
                    }*/
                }
            }

            std::shuffle(legal.begin(), legal.end(), engine);
        }
    };

    MCTS()
    {
        engine.seed(1234);
        visited.resize(12, vector<int>(12, 0));
    }

    int getTn(int x, int y) {
        if (root->child2board[x][y]) {
            cout << root->child2board[x][y]->parent_move.x << " " << root->child2board[x][y]->parent_move.y << endl;
            return root->child2board[x][y]->Tn;
        }
        else return 0;
    }

    Step* getBestChild(int x, int y) {
        cout << x << " " << y << endl;
        cout << root->child2board[x][y]->parent_move.x << " " << root->child2board[x][y]->parent_move.y << endl;
        return &(root->child2board[x][y]->parent_move);
    }

    vector<int> getBestStep() {
        int maxTn = -1;
        Step* bestStep = nullptr;
        vector<int> res(4);

        for (auto child : root->children) {
            if (child->Tn > maxTn) {
                maxTn = child->Tn;
                bestStep = &child->parent_move;
            }
        }

        res[0] = bestStep->x;
        res[1] = bestStep->y;
        res[2] = bestStep->numOfStep;
        res[3] = bestStep->dir;
        cout << "return\n";
        for (int i = 0; i < 4; i++) {
            cout << res[i] << " ";
        }
        cout << "\n";
        return res;
    }

    void print_check() {
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 12; j++) {
                if (root->child2board[i][j]) {
                    cout << "x: " << i << "y: " << j << endl;
                    cout << "x: " << root->child2board[i][j]->parent_move.x << "y: " << root->child2board[i][j]->parent_move.y << "double check x: " << root->child2board[i][j]->parent_move.x << "numOfStep: " << root->child2board[i][j]->parent_move.numOfStep << "dir: " << root->child2board[i][j]->parent_move.dir << endl;
                }
            }
        }
    }

    void run(int state[12][12], clock_t start, int time_limit)
    {
        root = new Node(state, engine);
        nodePool.push_back(*root);
        clock_t end;

        while (1)
        {
            traverse(root);
            end = clock();
            cout << "1\n";
            if (((double)(end - start)) / CLOCKS_PER_SEC >= time_limit)
                break;
        }
    }

    int traverse(Node* node, bool isOpponent = false)
    {
        while (!node->legal.empty() && node->state[node->legal.back().first][node->legal.back().second] != 0) node->legal.pop_back();
        if (!node->legal.empty())
        {
            //cout << "before next expand:\n";
            //print_check();
            Node* leaf = expand(node);
            //cout << "after next expand:\n";
            //print_check();
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

        if (max_UCT == -1) {
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
        int tmp[12][12];
        memcpy(tmp, node->state, 12 * 12 * sizeof(int));
        vector<int> result(2);
        result[0] = it.first;
        result[1] = it.second;
        // let player = 3 be MCTS simualtion player
        tmp[result[0]][result[1]] = 3;
        // randomly pick max distance
        uniform_int_distribution<int> uniform(1, 3);
        int max_step = uniform(engine);
        // randomly pick direction
        vector<int> legal_dir = { 1, 2, 3, 4, 5, 6 };
        shuffle(legal_dir.begin(), legal_dir.end(), engine);
        int keep = 1;

        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < max_step - 1; j++)
            {
                if (result[0] < 0 || result[0] > 11 || result[1] < 0 || result[1] > 11 || tmp[result[0]][result[1]] == 0) {
                    result = Next_Node(result[0], result[1], legal_dir[i]);
                    tmp[result[0]][result[1]] = 3;
                    keep++;
                }
            }
            if (keep != 1 || i == 5) {
                Node* newNode = new Node(tmp, engine);
                Step newStep = Step(it.first, it.second, keep, legal_dir[i]);
                newNode->parent_move = newStep;
                node->children.push_back(newNode);
                node->child2board[it.first][it.second] = newNode;
                break;
            }
        }


        /*nodePool.push_back(Node(tmp, engine));
        nodePool.back().parent_move = it;
        node->children.push_back(&nodePool.back());
        node->child2board[it.x][it.y] = &nodePool.back();*/
        //if (root == node) cout << "It's root!\n";
        //cout << "x: " << it.x << "y: " << it.y << "double check x: " << node->child2board[it.x][it.y]->parent_move.x << "numOfStep: " << node->child2board[it.x][it.y]->parent_move.numOfStep << "dir: " << node->child2board[it.x][it.y]->parent_move.dir << endl;
        //cout << "check:\n";
        //print_check();
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
        //if (n == 0) {
        //    cout << "empty exit\n";
        //    exit(0);
        //}

        while (1)
        {
            int i = 0;
            //int tmp[12][12];
            //memcpy(tmp, after, 12 * 12 * sizeof(int));
            Step* nextMove = nullptr;

            while (i < n)
            {
                // randomly pick up an empty space
                std::uniform_int_distribution<int> uniform(i, n - 1);
                int index = uniform(engine);
                auto it = empty[index];

                if (after[it.first][it.second] != 0) {
                    // maybe be occupied by 2 or 3 step
                    swap(empty[index], empty[i]);
                    i++;
                }
                else {
                    vector<int> result(2);
                    result[0] = it.first;
                    result[1] = it.second;
                    // let player = 3 be MCTS simualtion player
                    after[result[0]][result[1]] = 3;
                    // randomly pick max distance
                    uniform_int_distribution<int> uniform(1, 3);
                    int max_step = uniform(engine);
                    // randomly pick direction
                    vector<int> legal_dir = { 1, 2, 3, 4, 5, 6 };
                    shuffle(legal_dir.begin(), legal_dir.end(), engine);
                    int keep = 1;

                    for (int i = 0; i < 6; i++) {
                        for (int j = 0; j < max_step - 1; j++)
                        {
                            if (result[0] < 0 || result[0] > 11 || result[1] < 0 || result[1] > 11 || after[result[0]][result[1]] == 0) {
                                result = Next_Node(result[0], result[1], legal_dir[i]);
                                after[result[0]][result[1]] = 3;
                                keep++;
                            }
                        }
                        if (keep != 1 || i == 5) break;
                    }
                }


                /*std::uniform_int_distribution<int> uniform(i, n - 1);
                int index = uniform(engine);
                auto it = empty[index];

                vector<int> legal_dir = { 1, 2, 3, 4, 5, 6 };
                shuffle(legal_dir.begin(), legal_dir.end(), engine);
                Step* legal_move = nullptr;

                for (int i = 0; i < 6; i++) {
                    Step move = Step(it.first, it.second, 3, legal_dir[i]);
                    int max_dis = checkMoveValidation(tmp, move);
                    if (max_dis != -1) {
                        std::uniform_int_distribution<int> uniform1(1, max_dis);
                        int dis = uniform1(engine);
                        move.numOfStep = dis;
                        legal_move = &move;
                        break;
                    }
                }

                if (!legal_move) {
                    swap(empty[index], empty[i]);
                    i++;
                }
                else {
                    swap(empty[index], empty[n - 1]);
                    nextMove = legal_move;
                    break;
                }*/
            }

            if (n == 0 || i == n - 1)
                return isOpponent;

            isOpponent = !isOpponent;
            n--;
        }
    }

    void backpropagate(Node* node, int result)
    {
        node->Tn++;
        node->x += result;
        for (int i = 0; i < (int)visited.size(); i++) {
            for (int j = 0; j < (int)visited[0].size(); j++) {
                if (visited[i][j] == 1 && node->child2board[i][j] != nullptr) {
                    node->child2board[i][j]->raveTn++;
                    node->child2board[i][j]->raveX += result;
                }
            }
        }
    }

    void calculate_UCT(Node& node, int N, bool isOpponent, double& UCT_val)
    {
        if (node.Tn == 0)
            return;
        double beta = (double)node.raveTn / ((double)node.Tn + (double)node.raveTn + 4 * (double)node.Tn * (double)node.raveTn * 0.025 * 0.025);
        double winRate = (double)node.x / (double)(node.Tn + 1);
        double raveWinRate = (double)node.raveX / (double)(node.raveTn + 1);
        double exploit = (isOpponent) ? (1 - beta) * (1 - winRate) + beta * (1 - raveWinRate) : (1 - beta) * winRate + beta * raveWinRate;
        double explore = sqrt(log(N) / (double)(node.Tn + 1));
        UCT_val = exploit + c * explore;
    }

private:
    Node* root;
    float c = 0.5;
    vector<Node> nodePool;
    vector<vector<int>> visited;
    default_random_engine engine;
    uniform_int_distribution<int> uniform;
    int boardSize = 12 * 12;
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
    cout << "109550005_MCTS\n";
    clock_t start = clock();
    MCTS* mcts = new MCTS();
    mcts->run(mapStat, start, 4);

    int max_count = -1;
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
    }
    cout << max_count << endl;
    Step* tmp = mcts->getBestChild(key_x, key_y);
    step[0] = tmp->x;
    step[1] = tmp->y;
    step[2] = tmp->numOfStep;
    step[3] = tmp->dir;
    //cout << "wefwef\n";
    //step = mcts->getBestStep();

    return step;
}

int main()
{
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
