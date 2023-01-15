#define SFML_STATIC

#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <bitset>
#include <string>
#include <sstream>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <queue>
#include <math.h>

#define ROWS 6
#define COLS 7
#define WIDTH 1000
#define HEIGHT 1200
// #define H 2
//#define MAXDEPTH 8
#define PEGSIZE 100
#define BRANCHING 7
#define TREEWINDOWWIDTH 1000
#define TREEWINDOWHEIGHT 800
#define CHARSIZE 20
#define NODERADIUS 10.f

enum PEG
{
    NONE,
    RED,
    YELLOW
};

int H = 2;
int MAXDEPTH = 8;
int RADIUS = PEGSIZE / 2 - 12;
class State;

int divide(uint64_t bitboard);
std::pair<State *, int> maximize(State *s, int alpha, int beta, bool pruning, int player);
std::pair<State *, int> minimize(State *s, int alpha, int beta, bool pruning, int player);
std::unordered_map<uint64_t, int> H1_costs;
std::unordered_map<uint64_t, int> H2_costs;
std::unordered_map<uint64_t, int> H1_costs2;
std::unordered_map<uint64_t, int> H2_costs2;

std::unordered_map<State*,std::vector<State*>> parentTree;
std::unordered_map<State*,State*> childTree;
std::unordered_map<State*,int> score;

const int scores[6][7] = {
    {3, 4, 5, 7, 5, 4, 3},
    {4, 6, 8, 10, 8, 6, 4},
    {5, 8, 11, 13, 11, 8, 5},
    {5, 8, 11, 13, 11, 8, 5},
    {4, 6, 8, 10, 8, 6, 4},
    {3, 4, 5, 7, 5, 4, 3}};

class State
{
private:
    uint64_t bitBoards[2];
    int *height;
    int count;
    int depth;
    inline static std::unordered_map<uint64_t, int> fours;
    inline static std::unordered_map<uint64_t, int> threes;
    inline static std::unordered_map<uint64_t, int> twos;
    State* prev;
    static int count_fours(uint64_t bitboard)
    {
        // if(State::fours.find(bitboard)!=State::fours.end()) return State::fours[bitboard];
        int count = 0;
        count += divide(bitboard & (bitboard >> 7) & (bitboard >> 14) & (bitboard >> 21));
        count += divide(bitboard & (bitboard >> 8) & (bitboard >> 16) & (bitboard >> 24));
        count += divide(bitboard & (bitboard >> 6) & (bitboard >> 12) & (bitboard >> 18));
        count += divide(bitboard & (bitboard >> 1) & (bitboard >> 2) & (bitboard >> 3));

        // State::fours[bitboard] = count;
        return count;
    }
    static int count_threes(uint64_t bitboard)
    {
        // if(State::threes.find(bitboard)!=State::threes.end()) return State::threes[bitboard];
        int count = 0;
        count += divide(bitboard & (bitboard >> 7) & (bitboard >> 14));
        count += divide(bitboard & (bitboard >> 8) & (bitboard >> 16));
        count += divide(bitboard & (bitboard >> 6) & (bitboard >> 12));
        count += divide(bitboard & (bitboard >> 1) & (bitboard >> 2));

        // State::threes[bitboard] = count;
        return count;
    }
    static int count_twos(uint64_t bitboard)
    {
        // if(State::twos.find(bitboard)!=State::twos.end()) return State::twos[bitboard];
        int count = 0;
        count += divide(bitboard & (bitboard >> 7));
        count += divide(bitboard & (bitboard >> 8));
        count += divide(bitboard & (bitboard >> 6));
        count += divide(bitboard & (bitboard >> 1));

        // State::twos[bitboard] = count;
        return count;
    }

public:
    State()
    {
        this->height = (int *)malloc(COLS * sizeof(int));
        for (int i = 0; i < COLS; i++)
            this->height[i] = i * 7;
        this->count = 0;
        this->depth = 0;
        this->bitBoards[0] = 0;
        this->bitBoards[1] = 0;
    }
    State(uint64_t b1, uint64_t b2) : State()
    {
        this->bitBoards[0] = b1;
        this->bitBoards[1] = b2;
    }
    State(State &cpy)
    {
        this->height = (int *)malloc(COLS * sizeof(int));
        std::copy(cpy.height, cpy.height + COLS, this->height);
        this->count = cpy.count;
        this->depth = cpy.depth;
        this->bitBoards[0] = cpy.bitBoards[0];
        this->bitBoards[1] = cpy.bitBoards[1];
    }
    uint64_t getBoard(char choice)
    {
        if (choice == 0 || choice == 1)
            return this->bitBoards[choice];

        return 0;
    }
    int getCount()
    {
        return count;
    }
    void setCount(int c)
    {
        this->count = c;
    }
    int incrementCount()
    {
        return ++this->count;
    }
    int *getHeight()
    {
        return this->height;
    }
    void setHeight(int *h)
    {
        this->height = h;
    }
    int getDepth()
    {
        return this->depth;
    }
    int incrementDepth()
    {
        return ++this->depth;
    }
    void move(int col)
    {
        uint64_t move = (uint64_t)1 << this->height[col];
        this->height[col]++;

        // count & 1 returns 1 if it's the odd players turn and 0 if it's the even player's turn
        // xor with move to save the new move
        this->bitBoards[this->count & 1] |= move;
        this->count++;
    }
    int countFours(int mode)
    {
        if (mode == 0 || mode == 1)
        {
            return State::count_fours(this->bitBoards[mode]);
        }
        else
        {
            return State::count_fours(this->bitBoards[0]) - State::count_fours(this->bitBoards[1]);
        }
    }
    int countThrees(int mode)
    {
        if (mode == 0 || mode == 1)
        {
            return State::count_threes(this->bitBoards[mode]);
        }
        else
        {
            return State::count_threes(this->bitBoards[0]) - State::count_threes(this->bitBoards[1]);
        }
    }
    int countTwos(int mode)
    {
        if (mode == 0 || mode == 1)
        {
            return State::count_twos(this->bitBoards[mode]);
        }
        else
        {
            return State::count_twos(this->bitBoards[0]) - State::count_twos(this->bitBoards[1]);
        }
    }
    bool isFull()
    {
        return this->count == ROWS * COLS;
    }
    std::vector<int> get_valid_moves()
    {
        std::vector<int> moves;
        uint64_t top_row = 0b1000000100000010000001000000100000010000001000000;
        for (int col = 0; col < COLS; col++)
        {
            if ((top_row & ((uint64_t)1 << this->height[col])) == 0)
            {
                moves.push_back(col);
            }
        }
        return moves;
    }
    std::vector<State *> get_neighbors()
    {
        std::vector<State *> neighbors;
        std::vector<int> moves = this->get_valid_moves();
        for (auto col : moves)
        {
            State *temp = new State(*this);
            temp->move(col);
            temp->incrementDepth();
            // int *temp_height = (int*)malloc(COLS*sizeof(int));
            // std::copy(height, height+COLS, temp_height);
            // uint64_t temp = move(col,bitboard,temp_height);
            neighbors.push_back(temp);
        }
        return neighbors;
    }
    void printBits()
    {
        printf("\n\n");
        for (int i = ROWS - 1; i >= 0; i--)
        {
            for (int j = 0; j < COLS; j++)
            {
                char p;
                if (((this->bitBoards[0] >> i) >> (ROWS + 1) * j) & 1)
                    p = 1;
                else if (((this->bitBoards[1] >> i) >> (ROWS + 1) * j) & 1)
                    p = 2;
                else
                    p = 0;
                printf("%d ", p);
            }
            printf("\n");
        }
        printf("\n\n");
    }

    std::string stringBits(){
        std::string board = "";
        for(int i=ROWS-1;i>=0;i--){
            for(int j=0;j<COLS;j++){
                if(((this->bitBoards[0] >> i) >> (ROWS+1)*j) & 1) board+='1';
                else if(((this->bitBoards[1] >> i) >> (ROWS+1)*j) & 1) board+='2';
                else board+='0';
            }
            board+="\n";
        }
        return board;
    }

    std::vector<std::vector<char>> bitsto2D()
    {
        std::vector<std::vector<char>> x;
        for (int i = ROWS - 1; i >= 0; i--)
        {
            x.push_back(std::vector<char>());
            for (int j = 0; j < COLS; j++)
            {
                char p;
                if (((this->bitBoards[0] >> i) >> (ROWS + 1) * j) & 1)
                    p = 1;
                else if (((this->bitBoards[1] >> i) >> (ROWS + 1) * j) & 1)
                    p = 2;
                else
                    p = 0;
                x[ROWS - 1 - i].push_back(p);
            }
        }
        return x;
    }
    int getCol(State *s)
    {
        int *otherHeight = s->getHeight();
        for (int i = 0; i < COLS; i++)
        {
            if (this->height[i] != otherHeight[i])
                return i;
        }
        return -1;
    }
    void printWinner()
    {
        int count1 = this->countFours(0);
        int count2 = this->countFours(1);
        if (count1 > count2)
        {
            printf("Player 1 wins!\n");
        }
        else if (count1 < count2)
        {
            printf("Player 2 wins!\n");
        }
        else
        {
            printf("Draw!\n");
        }

        printf("Final score:\n");
        printf("Player 1: %d\n", count1);
        printf("Player 2: %d\n", count2);
    }
};

class Button {
public:
    Button(){

    }
    Button(std::string words,sf::Vector2f location) {
        this->font.loadFromFile("arial.ttf");
        string.setFont(font);
        string.setString(words);
        string.setCharacterSize(CHARSIZE);
        string.setFillColor(sf::Color::White);
        string.setPosition(location.x-(string.getGlobalBounds().width/2), location.y-(string.getGlobalBounds().height/2));
        x[0] = string.getPosition().x;
        x[1] = (string.getPosition().x + string.getGlobalBounds().width);
        y[0] = string.getPosition().y;
        y[1] = (string.getPosition().y + string.getGlobalBounds().height);
    }
    bool checkClick (sf::Vector2i mousePos) {
        if (mousePos.x>x[0] && mousePos.x<x[1]) {
            if(mousePos.y>y[0] && mousePos.y<y[1]) {
                return true;
            }
        }
        return false;
    }
    void setColor(const sf::Color &c){
        string.setFillColor(c);
    }
    void setText(std::string words) {
        string.setString(words);
    }
    sf::Text& getText() {
        return string;
    }
private:
    float x[2];
    float y[2];
    sf::Font font;
    sf::Text string;
    bool current;
};

class Tree{
    public:
        std::unordered_map<State*, std::vector<State*>> m;
        std::unordered_map<State*,int> vals;
        std::vector<std::pair<Button*,State*>> nodes;
        void iterate_map(State* r){
            std::vector<int> x(4,0);
            std::queue<State*> q;
            q.push(r);
            State* temp;
            std::vector<State*> t;
            int score;
            int depth;
            while(!q.empty()){
                t.clear();
                temp = q.front();
                q.pop();
                t = this->m[temp];
                score = this->vals[temp];
                depth = temp->getDepth();
                float x_pos = (TREEWINDOWWIDTH/pow(BRANCHING,depth))*x[depth] + TREEWINDOWWIDTH/(pow(BRANCHING,depth)*2) - NODERADIUS;
                float y_pos = depth*(TREEWINDOWHEIGHT/(MAXDEPTH+1)) + TREEWINDOWHEIGHT/((MAXDEPTH+1)*2) - NODERADIUS;
                Button* node = new Button(std::to_string(score), sf::Vector2f(x_pos, y_pos));
                nodes.push_back(std::pair<Button*,State*>(node, temp));
                // printf("%d\n", nodes.size());
                printf("Level: %d\n", depth);
                printf("Parent: (%.2F, %.2F)\n", x_pos, y_pos);
                // printf("Children: ");
                for(auto c : t){
                    // printf("%c ", c->name);
                    q.push(c);
                }
                x[depth]++;
                printf("\n\n");
            }
        }

        void iterate_level(State* r){
            nodes.clear();
            std::vector<int> x(2,0);
            std::vector<State*> t = m[r];
            int score;
            int depth;
            int root_level = r->getDepth();
            int player = m[nullptr][0]->getCount();
            t.insert(t.begin(), r);
            for(auto st : t){
                score = this->vals[st];
                depth = st->getDepth();
                float x_pos = (TREEWINDOWWIDTH/pow(BRANCHING,(depth-root_level)))*x[depth-root_level] + TREEWINDOWWIDTH/(pow(BRANCHING,depth-root_level)*2) - NODERADIUS;
                float y_pos = (depth-root_level)*(TREEWINDOWHEIGHT/2) + TREEWINDOWHEIGHT/(2*2) - NODERADIUS;
                std::string text = "    " + std::to_string(score) + "\n" + st->stringBits();
                Button* node = new Button(text, sf::Vector2f(x_pos,y_pos));
                if((st->getCount()&1)==(player&1)) node->setColor(sf::Color::Green);
                else node->setColor(sf::Color::Red);
                nodes.push_back(std::pair<Button*,State*>(node, st));
                x[depth-root_level]++;
            }
        }
};

uint64_t bitBoards[] = {0, 0};

int height[] = {0, 7, 14, 21, 28, 35, 42};

int count;

int nodes;

//-------------------------------------------------- Blackbox ---------------------------------------------------------#
int BitCount(unsigned int u)
{
    unsigned int uCount;

    uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
    return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

int divide(uint64_t bitboard)
{
    return BitCount(bitboard & 0xFFFFFFFF) + BitCount(bitboard >> 32);
}
//----------------------------------------- end blackbox --------------------------------------------------------------#

void printMatrix(std::vector<std::vector<char>> vectors)
{
    printf("\n\n");
    for (auto vector : vectors)
    {
        for (auto b : vector)
        {
            printf("%d ", b);
        }
        printf("\n");
    }
    printf("\n\n");
}

int H1(State *s, int player)
{
    int cost[2] = {-2147483645, -2147483645};

    if (H1_costs.find(s->getBoard(player)) != H1_costs.end())
        cost[player] = H1_costs[s->getBoard(player)];
    if (H1_costs2.find(s->getBoard(!player)) != H1_costs2.end())
        cost[!player] = H1_costs2[s->getBoard(!player)];

    if (cost[0] == -2147483645 || cost[1] == -2147483645)
    {
        cost[0] = 0;
        cost[1] = 0;
        int temp_player = player + 1;
        int temp_not_player = !player + 1;
        std::vector<std::vector<char>> board = s->bitsto2D();
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++)
            {
                if (board[i][j] == temp_player)
                {
                    cost[player] += scores[i][j];
                }
                else if (board[i][j] == temp_not_player)
                {
                    cost[!player] += scores[i][j];
                }
            }
        }
        H1_costs[s->getBoard(player)] = cost[player];
        H1_costs2[s->getBoard(!player)] = cost[!player];
    }
    return cost[player] - cost[!player];
}

int H2(State *s, int player)
{
    int cost[2] = {-2147483645, -2147483645};
    if (H2_costs.find(s->getBoard(player)) != H2_costs.end())
        cost[player] = H2_costs[s->getBoard(player)];
    if (H2_costs2.find(s->getBoard(!player)) != H2_costs2.end())
        cost[!player] = H2_costs2[s->getBoard(!player)];

    if (cost[0] == -2147483645 || cost[1] == -2147483645)
    {
        int twos[2] = {s->countTwos(0), s->countTwos(1)};
        int threes[2] = {s->countThrees(0), s->countThrees(1)};
        int fours[2] = {s->countFours(0), s->countFours(1)};
        cost[player] = (twos[player]) * 30;    // twos
        cost[player] += (threes[player]) * 75; // threes
        cost[player] += (fours[player]) * 500; // fours

        // cost[!player]  = (twos[!player])*5; // opponent twos
        // cost[!player] += (threes[!player])*55; // opponent threes
        // cost[!player] += (fours[!player])*150; // opponent fours

        cost[!player] = (twos[!player]) * 30;    // opponent twos
        cost[!player] += (threes[!player]) * 75; // opponent threes
        cost[!player] += (fours[!player]) * 500; // opponent fours

        H2_costs[s->getBoard(player)] = cost[player];
        H2_costs2[s->getBoard(!player)] = cost[!player];
    }

    return cost[player] - cost[!player];
 }
int evaluate(State *s, int player)
{
    if (s->getCount() < 10)
        return H1(s, player);

    switch (H)
    {
    case 1:
        return H1(s, player);
    case 2:
        return H2(s, player);
    default:
        return H2(s, player);
    }
}

std::pair<State *, int> minimize(State *s, int alpha, int beta, bool pruning, int player)
{
    nodes++;
    if(s->isFull()){
        score[s] = s->countFours(player) - s->countFours(!player);
        return std::pair<State*, int>(s, score[s]);
    }   
    if(s->getDepth() >= MAXDEPTH){
        score[s] = evaluate(s, player);
        return std::pair<State*, int>(s, score[s]);
    }

    std::pair<State *, int> min_found(0, 2147483000);
    std::pair<State *, int> temp_state;

    parentTree[s] = std::vector<State *>();

    for (auto x : s->get_neighbors())
    {
        parentTree[s].push_back(x);
        childTree[x] = s;
        temp_state = maximize(x, alpha, beta, pruning, player);
        if (temp_state.second < min_found.second)
        {
            min_found.second = temp_state.second;
            min_found.first = x;
        }
        if (min_found.second <= alpha && pruning)
        {
            break;
        }
        if (min_found.second < beta && pruning)
        {
            beta = min_found.second;
        }
    }
    score[s] = min_found.second;
    return min_found;
}

std::pair<State *, int> maximize(State *s, int alpha, int beta, bool pruning, int player)
{
    nodes++;
    if(s->isFull()){
        score[s] = s->countFours(player) - s->countFours(!player);
        return std::pair<State*, int>(s, score[s]);
    }    
    if(s->getDepth() >= MAXDEPTH){
        score[s] = evaluate(s, player);
        return std::pair<State*, int>(s, score[s]);
    }

    std::pair<State *, int> max_found(0, -2147483000);
    std::pair<State *, int> temp_state;

    parentTree[s] = std::vector<State *>();

    for (auto x : s->get_neighbors())
    {
        parentTree[s].push_back(x);
        childTree[x] = s;
        temp_state = minimize(x, alpha, beta, pruning, player);
        if (temp_state.second > max_found.second)
        {
            max_found.second = temp_state.second;
            max_found.first = x;
        }
        if (max_found.second >= beta && pruning)
        {
            break;
        }
        if (max_found.second > alpha && pruning)
        {
            alpha = max_found.second;
        }
    }
    score[s] = max_found.second;
    return max_found;
}

int minimax(State *s, int player, bool pruning)
{
    nodes = 0;
    parentTree.clear();
    score.clear();
    parentTree[nullptr] = std::vector<State *>(1, s);
    // printf("in minimax %d\n", parentTree[nullptr].size());
    childTree.clear();
    H1_costs.clear();
    H2_costs.clear();
    H1_costs2.clear();
    H2_costs2.clear();
    return s->getCol(maximize(s, INT32_MIN, INT32_MAX, pruning, player).first);
}

int main()
{
    Tree t;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    int mouseX;
    sf::Image icon;
    sf::Texture board;
    sf::Texture red;
    sf::Texture yellow;
    sf::Texture button;
    sf::Texture tributton;
    sf::Texture triup;
    sf::Font font;
    sf::Font font2;
    if (!icon.loadFromFile("icon.png"))
    {
        printf("Error loading image\n");
        exit(EXIT_FAILURE);
    }
    if (!board.loadFromFile("testboard.png"))
    {
        printf("Error loading image\n");
        exit(EXIT_FAILURE);
    }
    if (!red.loadFromFile("redpeg.png"))
    {
        printf("Error loading image\n");
        exit(EXIT_FAILURE);
    }
    if (!yellow.loadFromFile("yellowpeg.png"))
    {
        printf("Error loading image\n");
        exit(EXIT_FAILURE);
    }
    if (!font.loadFromFile("goldleaf.ttf"))
    {
        std::cout << "Failed to load resources.\n\n";
        return 0;
    }
    if (!font2.loadFromFile("Aquire.otf"))
    {
        std::cout << "Failed to load resources.\n\n";
        return 0;
    }
    if (!button.loadFromFile("button.png"))
    {
        printf("Error loading image\n");
        exit(EXIT_FAILURE);
    }
    if (!tributton.loadFromFile("tributton.png"))
    {
        printf("Error loading image\n");
        exit(EXIT_FAILURE);
    }
    if (!triup.loadFromFile("triup.png"))
    {
        printf("Error loading image\n");
        exit(EXIT_FAILURE);
    }

    sf::Vector2u boardSize = board.getSize();
    sf::RectangleShape background(sf::Vector2f(boardSize.x + PEGSIZE, boardSize.y + PEGSIZE * 3));
    background.setFillColor(sf::Color::White);

    sf::RenderWindow window(sf::VideoMode(boardSize.x + PEGSIZE, boardSize.y + PEGSIZE * 3), "Connect 4", sf::Style::Close, settings);
    window.setFramerateLimit(60);
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    sf::Sprite boardSprite;
    sf::Sprite redSprite;
    sf::Sprite yellowSprite;
    sf::Sprite minimaxButton;
    sf::Sprite pruningButton;
    sf::Sprite startButton;
    sf::Sprite easyButton;
    sf::Sprite hardButton;
    sf::Sprite redStarts;
    sf::Sprite yellowStarts;
    sf::Sprite visualizeTree;
    sf::Sprite restart;
    sf::Sprite triangle;
    sf::Sprite triangle2;
    sf::CircleShape redChip(RADIUS);

    boardSprite.setTexture(board);
    boardSprite.setPosition(sf::Vector2f(PEGSIZE / 2, PEGSIZE));
    redSprite.setTexture(red);
    yellowSprite.setTexture(yellow);
    minimaxButton.setTexture(button);
    pruningButton.setTexture(button);
    startButton.setTexture(button);
    easyButton.setTexture(button);
    hardButton.setTexture(button);
    redStarts.setTexture(button);
    yellowStarts.setTexture(button);
    visualizeTree.setTexture(button);
    restart.setTexture(button);
    triangle.setTexture(tributton);
    triangle2.setTexture(triup);
    redChip.setFillColor(sf::Color::Red);

    State *s = new State();
    State *prev;
    int col;
    PEG grid[49];
    for (int i = 0; i < 49; i++)
        grid[i] = NONE;

    sf::Text miniText;
    minimaxButton.setPosition(sf::Vector2f(290, 780));
    miniText.setFont(font);
    miniText.setString("Minimax");
    miniText.setCharacterSize(40);
    miniText.setPosition(310, 783);

    sf::Text pruningText;
    pruningButton.setPosition(sf::Vector2f(290, 840));
    pruningText.setFont(font);
    pruningText.setString("Pruning");
    pruningText.setCharacterSize(40);
    pruningText.setPosition(320, 843);

    sf::Text startText;
    startButton.setPosition(sf::Vector2f(90, 810));
    startText.setFont(font);
    startText.setString("START");
    startText.setCharacterSize(40);
    startText.setPosition(120, 813);

    sf::Text restartText;
    restart.setPosition(sf::Vector2f(90, 920));
    restartText.setFont(font);
    restartText.setString("RESTART");
    restartText.setCharacterSize(40);
    restartText.setPosition(110, 923);

    sf::Text depth;
    triangle.setPosition(sf::Vector2f(870, 500));
    triangle2.setPosition(sf::Vector2f(870, 300));
    depth.setFont(font2);
    depth.setString(std::to_string(MAXDEPTH));
    depth.setCharacterSize(35);
    depth.setFillColor(sf::Color::Blue);
    depth.setPosition(882, 395);

    sf::Text easyText;
    easyButton.setPosition(sf::Vector2f(490, 780));
    easyText.setFont(font);
    easyText.setString("Easy");
    easyText.setCharacterSize(42);
    easyText.setPosition(534, 780);

    sf::Text hardText;
    hardButton.setPosition(sf::Vector2f(490, 840));
    hardText.setFont(font);
    hardText.setString("Hard");
    hardText.setCharacterSize(42);
    hardText.setPosition(534, 841);

    sf::Text treeText;
    visualizeTree.setPosition(sf::Vector2f(690, 810));
    treeText.setFont(font);
    treeText.setString("Visualize");
    treeText.setCharacterSize(40);
    treeText.setPosition(713, 812);

    sf::Text redturn;
    redStarts.setPosition(sf::Vector2f(290, 920));
    redturn.setFont(font);
    redturn.setString("You Start");
    redturn.setCharacterSize(35);
    redturn.setPosition(315, 923);

    sf::Text yellowturn;
    yellowStarts.setPosition(sf::Vector2f(490, 920));
    yellowturn.setFont(font);
    yellowturn.setString("AI Starts");
    yellowturn.setCharacterSize(35);
    yellowturn.setPosition(515, 923);

    sf::Text connectFour;
    connectFour.setFont(font2);
    connectFour.setString("CONNECT FOUR");
    connectFour.setCharacterSize(70);
    connectFour.setPosition(185, 25);
    connectFour.setFillColor(sf::Color::Black);

    sf::Text playerScore;
    playerScore.setFont(font2);
    // playerScore.setString("Your Score > ");
    playerScore.setCharacterSize(25);
    playerScore.setPosition(678, 915);
    playerScore.setFillColor(sf::Color::Red);
    sf::Text aiScore;
    aiScore.setFont(font2);
    // aiScore.setString("Agent Score > ");
    aiScore.setCharacterSize(25);
    aiScore.setPosition(678, 945);
    aiScore.setFillColor(sf::Color(253, 190, 0));

    sf::Cursor cursor;
    bool isDropped = false;
    bool pruning = true;
    bool start = false;
    bool hover = false;
    bool redFirst = false;

    std::vector<int> boundaries = {170, 270, 370, 470, 570, 670, 770};
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseMoved)
            {
                if (startButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
                {
                    startText.setFillColor(sf::Color::Green);
                    hover = true;
                }
                else if (restart.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
                {
                    restartText.setFillColor(sf::Color::Green);
                    hover = true;
                }
                else if (triangle.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
                {
                    hover = true;
                }
                else if (triangle2.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
                {
                    hover = true;
                }
                else if (visualizeTree.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
                {
                    treeText.setFillColor(sf::Color::Cyan);
                    hover = true;
                }

                else if (redStarts.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y) && !start)
                {
                    redturn.setFillColor(sf::Color::Red);
                    hover = true;
                }
                else if (yellowStarts.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y) && !start)
                {
                    yellowturn.setFillColor(sf::Color::Yellow);
                    hover = true;
                }
                else if (minimaxButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
                {
                    miniText.setFillColor(sf::Color::Cyan);
                    hover = true;
                }
                else if (pruningButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
                {
                    pruningText.setFillColor(sf::Color::Cyan);
                    hover = true;
                }
                else if (easyButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y) && !start)
                {
                    easyText.setFillColor(sf::Color::Yellow);
                    hover = true;
                }
                else if (hardButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y) && !start)
                {
                    hardText.setFillColor(sf::Color::Red);
                    hover = true;
                }
                else
                {
                    hover = false;
                }

                // if (minimaxButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y) || pruningButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y) || startButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y) || hardButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y) || easyButton.getGlobalBounds().contains(event.mouseMove.x, event.mouseMove.y))
                if (hover)
                {
                    if (cursor.loadFromSystem(sf::Cursor::Hand))
                        window.setMouseCursor(cursor);
                }
                else
                {
                    if (cursor.loadFromSystem(sf::Cursor::Arrow))
                        window.setMouseCursor(cursor);
                    startText.setFillColor(sf::Color::White);
                    restartText.setFillColor(sf::Color::White);
                    miniText.setFillColor(sf::Color::White);
                    pruningText.setFillColor(sf::Color::White);
                    easyText.setFillColor(sf::Color::White);
                    hardText.setFillColor(sf::Color::White);
                    treeText.setFillColor(sf::Color::White);
                    redturn.setFillColor(sf::Color::White);
                    yellowturn.setFillColor(sf::Color::White);

                    if (start)
                    {
                        easyText.setFillColor(sf::Color(128, 128, 128));
                        hardText.setFillColor(sf::Color(128, 128, 128));
                        redturn.setFillColor(sf::Color(128, 128, 128));
                        yellowturn.setFillColor(sf::Color(128, 128, 128));
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (minimaxButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    pruning = false;
                }
                else if (pruningButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    pruning = true;
                }
                else if (triangle2.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    MAXDEPTH++;
                    if(MAXDEPTH>13) MAXDEPTH = 13;
                    depth.setString(std::to_string(MAXDEPTH));
                }
                else if (triangle.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    MAXDEPTH--;
                    if(MAXDEPTH<1) MAXDEPTH = 1;
                    depth.setString(std::to_string(MAXDEPTH));
                }
                else if (easyButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    H = 1;
                }
                else if (hardButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    H = 2;
                }
                else if (startButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    playerScore.setString("Your Score > 0");
                    aiScore.setString("Agent Score > 0");
                    start = true;
                }
                else if (restart.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    for (int i = 0; i < 49; i++)
                        grid[i] = NONE;
                    playerScore.setString("Your Score > 0");
                    aiScore.setString("Agent Score > 0");
                    s = new State();
                    start = true;
                }
                else if (visualizeTree.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    sf::RenderWindow treeWindow(sf::VideoMode(TREEWINDOWWIDTH, TREEWINDOWHEIGHT), "Tree");
                    treeWindow.setFramerateLimit(60);
                    t.m = parentTree;
                    t.vals = score;
                    t.iterate_level(prev);
                    while (treeWindow.isOpen())
                    {
                        sf::Event treeEvent;
                        while (treeWindow.pollEvent(treeEvent))
                        {
                            if (treeEvent.type == sf::Event::Closed)
                                treeWindow.close();

                            if(treeEvent.type == treeEvent.MouseButtonReleased && treeEvent.mouseButton.button == sf::Mouse::Button::Left){
                                for(int i=0;i<t.nodes.size();i++){
                                    if(t.nodes[i].first->checkClick(sf::Mouse::getPosition(treeWindow))){
                                        if(i==0){
                                            if(childTree.find(t.nodes[i].second)!=childTree.end()) t.iterate_level(childTree[t.nodes[i].second]);
                                        }else{
                                            t.iterate_level(t.nodes[i].second);
                                        }
                                    }
                                }
                            }
                        }

                        treeWindow.clear();
                        for(auto n : t.nodes){
                            treeWindow.draw(n.first->getText());
                        }
                        treeWindow.display();
                    }
                }
                else if (redStarts.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    redFirst = true;
                }
                else if (yellowStarts.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    redFirst = false;
                }
                else if (start && event.mouseButton.x >= PEGSIZE / 2 + RADIUS && event.mouseButton.x <= boardSize.x - RADIUS && !s->isFull())
                {
                    std::cout << pruning << "\n";
                    isDropped = true;
                    col = (event.mouseButton.x / 100) - 1;
                    std::vector<int> valid_cols = s->get_valid_moves();
                    std::vector<int>::iterator it;
                    it = std::find(valid_cols.begin(), valid_cols.end(), col);
                    if (it == valid_cols.end())
                    {
                        col = -1;
                        isDropped = false;
                    }

                    // std::cout << col << "\n";
                }
            }
        }
        mouseX = sf::Mouse::getPosition(window).x;
        for (int i = 0; i < boundaries.size(); i++)
        {
            if (mouseX > boundaries[i] && mouseX < boundaries[i + 1])
            {
                if (mouseX - boundaries[i] < boundaries[i + 1] - mouseX)
                {
                    mouseX = boundaries[i];
                }
                else
                    mouseX = boundaries[i + 1];
            }
        }
        if (mouseX < boundaries[0])
            mouseX = boundaries[0];
        else if (mouseX > boundaries[6])
            mouseX = boundaries[6];
        if (start)
            redSprite.setPosition(sf::Vector2f(mouseX - RADIUS, 20));

        int s0, s1;
        if (redFirst)
        {
            if (!(s->getCount() & 1) && isDropped)
            {
                isDropped = false;
                s->move(col);
                int *height = s->getHeight();
                grid[height[col]] = RED;
                s0 = s->countFours(0);
                playerScore.setString("Your Score > " + std::to_string(s0));
            }
            else if (s->getCount() & 1 && start && !s->isFull())
            {
                const clock_t begin_time = clock();
                col = minimax(s, 1, pruning);
                printf("Time: %.2f\n", float(clock() - begin_time) / CLOCKS_PER_SEC);
                printf("Nodes: %d\n", nodes);

                // std::cout << col << "\n";
                prev = new State(*s);
                score[prev] = score[s];
                parentTree[nullptr][0] = prev;
                parentTree[prev] = parentTree[s];
                for (auto child : parentTree[prev])
                {
                    childTree[child] = prev;
                }

                s->move(col);
                int *height = s->getHeight();
                grid[height[col]] = YELLOW;
                s1 = s->countFours(1);
                aiScore.setString("Agent Score > " + std::to_string(s1));
            }
            else if (s->isFull())
            {
                start = false;
                s0 = s->countFours(0);
                s1 = s->countFours(1);
                playerScore.setString("Your Score > " + std::to_string(s0));
                aiScore.setString("Agent Score > " + std::to_string(s1));
            }
        }
        else
        {
            if ((s->getCount() & 1) && isDropped)
            {
                isDropped = false;
                s->move(col);
                int *height = s->getHeight();
                grid[height[col]] = RED;
                s1 = s->countFours(1);
                playerScore.setString("Your Score > " + std::to_string(s1));
            }
            else if (!(s->getCount() & 1) && start && !s->isFull())
            {

                const clock_t begin_time = clock();
                col = minimax(s, 0, pruning);
                printf("Time: %.2f\n", float(clock() - begin_time) / CLOCKS_PER_SEC);
                printf("Nodes: %d\n", nodes);

                // std::cout << col << "\n";
                prev = new State(*s);
                score[prev] = score[s];
                parentTree[nullptr][0] = prev;
                parentTree[prev] = parentTree[s];
                for (auto child : parentTree[prev])
                {
                    childTree[child] = prev;
                }
                s->move(col);
                int *height = s->getHeight();
                grid[height[col]] = YELLOW;
                s0 = s->countFours(0);
                aiScore.setString("Agent Score > " + std::to_string(s0));
            }
            else if (s->isFull())
            {
                start = false;
                s0 = s->countFours(0);
                s1 = s->countFours(1);
                playerScore.setString("Your Score > " + std::to_string(s1));
                aiScore.setString("Agent Score > " + std::to_string(s0));
            }
        }

        window.clear();
        window.draw(background);
        window.draw(boardSprite);
        window.draw(minimaxButton);
        window.draw(miniText);
        window.draw(pruningButton);
        window.draw(pruningText);
        window.draw(startButton);
        window.draw(startText);
        window.draw(easyButton);
        window.draw(easyText);
        window.draw(hardButton);
        window.draw(hardText);
        window.draw(redStarts);
        window.draw(yellowStarts);
        window.draw(visualizeTree);
        window.draw(redturn);
        window.draw(yellowturn);
        window.draw(treeText);
        window.draw(restart);
        window.draw(triangle2);
        window.draw(triangle);
        window.draw(depth);
        window.draw(restartText);
        if (start)
            window.draw(redSprite);
        else
            window.draw(connectFour);
        window.draw(playerScore);
        window.draw(aiScore);

        for (int i = 0; i < 49; i++)
        {
            // circle.setPosition(PEGSIZE + RADIUS-4 +(2*RADIUS+23)*(i/7), COLS*PEGSIZE+33 - (PEGSIZE-1.1)*(i%COLS));
            if (grid[i] == RED && grid[i] % 7 != 6)
            {
                // circle.setFillColor(sf::Color::Red);
                redSprite.setPosition(PEGSIZE + RADIUS - 4 + (2 * RADIUS + 23) * (i / 7), COLS * PEGSIZE + 33 - (PEGSIZE - 1.1) * (i % COLS));
            }
            else if (grid[i] == YELLOW && grid[i] % 7 != 6)
            {
                // circle.setFillColor(sf::Color::Yellow);
                yellowSprite.setPosition(PEGSIZE + RADIUS - 4 + (2 * RADIUS + 23) * (i / 7), COLS * PEGSIZE + 33 - (PEGSIZE - 1.1) * (i % COLS));
            }
            else
                continue;

            // circle.setFillColor(sf::Color(80, 80, 90));
            // window.draw(circle);
            window.draw(redSprite);
            window.draw(yellowSprite);
        }
        window.display();
    }
    // int col;
    /* while (!s->isFull())
     {
         if (s->getCount() & 1)
         {
             // printf("Enter col:\n");
             // scanf("%d", &col);
             col = minimax(s, 1, true);
             printf("Visited: %d\n", nodes);
             printf("Keys: %d\n", parentTree.size());
         }
         else
         {
             col = minimax(s, 0, true);
             printf("Visited: %d\n", nodes);
             printf("Keys: %d\n", parentTree.size());
         }
         s->move(col);
         s->printBits();
     }
     s->printWinner();*/

    return 0;
}
