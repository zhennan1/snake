/*Snake Game
2023.12
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <conio.h>
#include <windows.h>
#include <string>
#include <memory>
#include <filesystem>
#include <vector>
#include <algorithm>

using namespace std;

// 方向
enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// 坐标
struct Point
{
    int x;
    int y;
};

// 食物
struct Food
{
    int x;
    int y;
    int value;
};

// 配置
struct Config
{
    // 难度，1-10，蛇移动速度为每 1 / gameDifficulty 秒移动一格
    int gameDifficulty;
    // 随机种子
    int randomSeed = -1;
    // 食物数量，1-5
    int numOfFood;
    // 食物概率，0-1，分别为 1、2、3 分食物的概率
    double foodProb[3] = {0.1, 0.3, 0.6};
    // 配置文件路径
    string configPath;
};

// 地图
struct Map
{
    // 地图宽度
    int width;
    // 地图高度
    int height;
    // 地图边界属性，0 为虚边界，1 为实边界
    int real[4];
    // 障碍物数量
    int numOfObstacle;
    // 障碍物坐标
    vector<Point> obstacle;
    // 地图文件路径
    string mapPath;
};

struct LeaderboardEntry
{
    string name;
    int score;
    string date;
    string time;
    string configPath;
    string mapPath;
};

// 贪吃蛇游戏类
class SnakeGame
{
private:
    // 当前方向
    Direction currentDirection;
    // 蛇头即将移动到的位置
    Point snakeHead;
    // 蛇
    vector<Point> snake;
    // 食物
    vector<Food> food;
    // 配置
    Config config;
    // 地图
    Map map;

    // 蛇长度，初始为 4，每吃一个食物加 1，已用 vector<Point> snake.size() 代替
    int snakeLength;
    // 当前分数
    int score;
    // 分数记录
    vector<int> scoreRecord;
    // 游戏是否结束
    bool gameOver;
    // 游戏是否暂停
    bool gamePause;
    // 是否回放
    bool replay;

    // 地图计数，用于回放
    int screenCount = 0;
    // 当前游戏画面
    vector<vector<char>> screen;
    // 游戏画面记录，用于回放
    vector<vector<vector<char>>> screenRecord;

    // 拓展功能：排行榜
    vector<LeaderboardEntry> leaderboard;

public:
    // 构造函数
    SnakeGame();
    // 析构函数
    ~SnakeGame();

    // 初始化
    void Init();
    // 运行游戏
    void Run();
    // 绘制地图
    void DrawMap();
    // 生成食物，用于初始化，生成 config.numOfFood 个食物
    void GenerateFood();
    // 生成食物，用于吃掉一个食物后生成一个新的食物
    void GenerateFood(int i);
    // 移动蛇
    void MoveSnake();
    // 处理输入
    void HandleInput();
    // 暂停游戏
    void PauseGame();
    // 结束游戏
    void EndGame();

    // 保存记录
    void SaveRecord();
    // 回放
    void Replay();

    // 创建配置文件
    void CreateConfig();
    // 加载配置文件
    void LoadConfig();
    // 加载上次使用的配置文件
    void LoadLastConfig();

    // 创建地图文件
    void CreateMap();
    // 加载地图文件
    void LoadMap();
    // 加载上次使用的地图文件
    void LoadLastMap();

    // 拓展功能：排行榜
    // 更新排行榜
    string playerName;
    void UpdateLeaderboard();
    // 显示排行榜
    void DisplayLeaderboard();
};

SnakeGame::SnakeGame() {}

SnakeGame::~SnakeGame() {}

void SnakeGame::Init()
{
    // 加载 map 和 config 文件
    LoadLastMap();
    LoadLastConfig();

    // 初始化 screen、snake、food、score、gameOver、replay 变量
    screen.clear();
    screen.resize(map.height + 2, vector<char>(map.width + 2, '0'));
    screenRecord.clear();
    screenCount = 0;
    snake.clear();
    snake.resize(4);
    food.clear();
    food.resize(config.numOfFood);
    score = 0;
    scoreRecord.clear();
    gameOver = false;
    replay = false;

    // 根据地图大小初始化蛇的坐标
    snake[0].x = map.width / 2 + 1;
    snake[0].y = map.height / 2 + 1;

    for (int i = 0; i < map.height + 2; ++i)
        for (int j = 0; j < map.width + 2; ++j)
            screen[i][j] = '0';

    screen[snake[0].y][snake[0].x] = '#';

    for (int i = 1; i < 4; ++i)
    {
        snake[i].x = map.width / 2 - i + 1;
        snake[i].y = map.height / 2 + 1;
        screen[snake[i].y][snake[i].x] = '*';
    }

    // 设置障碍物
    if (map.numOfObstacle > 0)
    {
        for (int i = 0; i < map.numOfObstacle; ++i)
        {
            screen[map.obstacle[i].y + 1][map.obstacle[i].x + 1] = 'O';
        }
    }

    // 设置边界
    if (map.real[LEFT] == 1)
    {
        for (int i = 0; i < map.height + 2; ++i)
        {
            screen[i][0] = '|';
        }
    }
    if (map.real[RIGHT] == 1)
    {
        for (int i = 0; i < map.height + 2; ++i)
        {
            screen[i][map.width + 1] = '|';
        }
    }
    if (map.real[UP] == 1)
    {
        for (int i = 0; i < map.width + 2; ++i)
        {
            screen[0][i] = '-';
        }
    }
    if (map.real[DOWN] == 1)
    {
        for (int i = 0; i < map.width + 2; ++i)
        {
            screen[map.height + 1][i] = '-';
        }
    }

    // 生成食物
    GenerateFood();
    // 设置初始方向为向右
    currentDirection = RIGHT;
}

void SnakeGame::Run()
{
    Init();
    while (!gameOver)
    {
        // 保存当前游戏画面和分数，用于回放
        screenRecord.push_back(screen);
        ++screenCount;
        scoreRecord.push_back(score);
        DrawMap();
        HandleInput();
        MoveSnake();
    }
    EndGame();
}

void SnakeGame::DrawMap()
{
    // 清屏
    system("cls");

    // 绘制画面，根据不同的字符，输出不同的字符和颜色
    // 0 为空格，1 为 1 分食物，2 为 2 分食物，3 为 3 分食物，# 为蛇头，* 为蛇身，O 为障碍物
    for (int i = 0; i <= map.height + 1; ++i)
    {
        for (int j = 0; j <= map.width + 1; ++j)
        {
            if (screen[i][j] == '0')
            {
                cout << ' ';
            }
            else if (screen[i][j] == 'O' || screen[i][j] == '|' || screen[i][j] == '-')
            {
                cout << screen[i][j];
            }
            else if (screen[i][j] == '1')
            {
                // 1 分食物为蓝色
                cout << "\033[44m@\033[0m";
            }
            else if (screen[i][j] == '2')
            {
                // 2 分食物为紫色
                cout << "\033[45m@\033[0m";
            }
            else if (screen[i][j] == '3')
            {
                // 3 分食物为黄色
                cout << "\033[43m@\033[0m";
            }
            else if (screen[i][j] == '#')
            {
                if (gameOver)
                {
                    // 游戏结束时蛇头为红色
                    cout << "\033[41m#\033[0m";
                }
                else
                {
                    cout << "\033[42m#\033[0m";
                }
            }
            else if (screen[i][j] == '*')
            {
                if (gameOver)
                {
                    // 游戏结束时蛇身为红色
                    cout << "\033[41m*\033[0m";
                }
                else
                {
                    cout << "\033[42m*\033[0m";
                }
            }
        }
        cout << endl;
    }

    // 输出分数
    if (!gameOver)
    {
        cout << "Current score: " << score << endl;
    }
    else
    {
        cout << "Game over! Your score is " << score << endl;
    }

    // 输出配置文件路径和地图文件路径
    cout << "Config: " << config.configPath << endl;
    cout << "Map: " << map.mapPath << endl;

    // 根据游戏状态输出提示信息
    if (!replay)
    {
        if (!gameOver)
        {
            if (!gamePause)
            {
                cout << "Enter space to pause, w/a/s/d to move." << endl;
            }
            else
            {
                cout << "Enter space to continue, q to quit." << endl;
            }
        }
        else
        {
            cout << "Enter b to save record, l to update leaderboard, or any key to go back to main menu." << endl;
        }
    }
    else
    {
        if (!gameOver)
        {
            cout << "Enter q to quit." << endl;
        }
        else
        {
            cout << "Replay finished. Enter any key to go back to main menu." << endl;
        }
    }
}

void SnakeGame::GenerateFood()
{
    // 根据随机种子初始化随机数生成器
    srand(config.randomSeed == -1 ? time(NULL) : config.randomSeed);

    // 生成食物
    for (int i = 0; i < config.numOfFood; ++i)
    {
        // 食物不能生成在蛇身上
        do
        {
            food[i].x = rand() % map.width + 1;
            food[i].y = rand() % map.height + 1;
        } while (screen[food[i].y][food[i].x] != '0');

        // 根据概率生成不同分数的食物
        float randValue = static_cast<float>(rand()) / RAND_MAX;
        if (randValue < config.foodProb[0])
        {
            food[i].value = 1;
            screen[food[i].y][food[i].x] = '1';
        }
        else if (randValue < config.foodProb[0] + config.foodProb[1])
        {
            food[i].value = 2;
            screen[food[i].y][food[i].x] = '2';
        }
        else
        {
            food[i].value = 3;
            screen[food[i].y][food[i].x] = '3';
        }
    }
}

void SnakeGame::GenerateFood(int i)
{
    // 根据随机种子初始化随机数生成器
    srand(config.randomSeed == -1 ? time(NULL) : config.randomSeed);

    // 食物不能生成在蛇身上
    do
    {
        food[i].x = rand() % map.width + 1;
        food[i].y = rand() % map.height + 1;
    } while (screen[food[i].y][food[i].x] != '0');

    // 根据概率生成不同分数的食物
    float randValue = static_cast<float>(rand()) / RAND_MAX;
    if (randValue < config.foodProb[0])
    {
        food[i].value = 1;
        screen[food[i].y][food[i].x] = '1';
    }
    else if (randValue < config.foodProb[0] + config.foodProb[1])
    {
        food[i].value = 2;
        screen[food[i].y][food[i].x] = '2';
    }
    else
    {
        food[i].value = 3;
        screen[food[i].y][food[i].x] = '3';
    }
}

void SnakeGame::MoveSnake()
{
    // 蛇头即将移动到的位置
    snakeHead.x = snake[0].x;
    snakeHead.y = snake[0].y;

    // 根据方向移动蛇头
    switch (currentDirection)
    {
    case UP:
        snakeHead.y--;
        break;
    case DOWN:
        snakeHead.y++;
        break;
    case LEFT:
        snakeHead.x--;
        break;
    case RIGHT:
        snakeHead.x++;
        break;
    }

    // 判断蛇头是否撞到边界，如果是则游戏结束
    if (snakeHead.y == 0)
    {
        if (map.real[UP] == 1)
        {
            gameOver = true;
            return;
        }
        else
        {
            snakeHead.y = map.height;
        }
    }
    if (snakeHead.y == map.height + 1)
    {
        if (map.real[DOWN] == 1)
        {
            gameOver = true;
            return;
        }
        else
        {
            snakeHead.y = 1;
        }
    }
    if (snakeHead.x == 0)
    {
        if (map.real[LEFT] == 1)
        {
            gameOver = true;
            return;
        }
        else
        {
            snakeHead.x = map.width;
        }
    }
    if (snakeHead.x == map.width + 1)
    {
        if (map.real[RIGHT] == 1)
        {
            gameOver = true;
            return;
        }
        else
        {
            snakeHead.x = 1;
        }
    }

    // 判断蛇头是否撞到蛇身，如果是则游戏结束
    for (int i = 1; i < snake.size() - 1; ++i)
    {
        if (snakeHead.x == snake[i].x && snakeHead.y == snake[i].y)
        {
            gameOver = true;
            return;
        }
    }

    // 判断蛇头是否撞到障碍物，如果是则游戏结束
    if (map.numOfObstacle > 0)
    {
        for (int i = 0; i < map.numOfObstacle; ++i)
        {
            if (snakeHead.x == map.obstacle[i].x + 1 && snakeHead.y == map.obstacle[i].y + 1)
            {
                gameOver = true;
                return;
            }
        }
    }

    // 移动蛇
    screen[snake[snake.size() - 1].y][snake[snake.size() - 1].x] = '0';

    int tempX = snake[snake.size() - 1].x;
    int tempY = snake[snake.size() - 1].y;

    for (int i = snake.size() - 1; i > 0; --i)
    {
        snake[i].x = snake[i - 1].x;
        snake[i].y = snake[i - 1].y;
    }

    snake[0].x = snakeHead.x;
    snake[0].y = snakeHead.y;

    screen[snake[0].y][snake[0].x] = '#';
    screen[snake[1].y][snake[1].x] = '*';

    // 判断蛇头是否吃到食物，如果是则加分并生成新的食物
    for (int i = 0; i < config.numOfFood; ++i)
    {
        if (snake[0].x == food[i].x && snake[0].y == food[i].y)
        {
            score += food[i].value;
            snakeLength++;
            // 还原蛇尾
            snake.push_back({tempX, tempY});
            screen[tempY][tempX] = '*';
            GenerateFood(i);
        }
    }
}

void SnakeGame::HandleInput()
{
    // 只处理1000 / gameDifficulty毫秒内的最后一个输入
    DWORD endTime = GetTickCount64() + 1000 / config.gameDifficulty;
    char key = 0;

    while (GetTickCount64() < endTime)
    {
        if (_kbhit())
        {
            key = _getch();
        }
    }

    // 处理输入，如果输入为q则退出游戏，如果输入为其他方向键则改变方向，如果输入为空格则暂停游戏
    if (key != 0)
    {
        if (key == 'w' && currentDirection != DOWN)
        {
            currentDirection = UP;
        }
        else if (key == 'a' && currentDirection != RIGHT)
        {
            currentDirection = LEFT;
        }
        else if (key == 's' && currentDirection != UP)
        {
            currentDirection = DOWN;
        }
        else if (key == 'd' && currentDirection != LEFT)
        {
            currentDirection = RIGHT;
        }
        else if (key == ' ')
        {
            PauseGame();
        }
    }
}

void SnakeGame::PauseGame()
{
    // 暂停游戏，按空格键继续游戏，按q键退出游戏
    gamePause = true;
    DrawMap();
    while (true)
    {
        if (_kbhit())
        {
            char key = _getch();
            if (key == ' ')
            {
                break;
            }
            else if (key == 'q')
            {
                gameOver = true;
                break;
            }
        }
    }
    gamePause = false;
}

#include <iostream>
#include <conio.h>  // 对于 _getch() 和 _kbhit()

void SnakeGame::EndGame()
{
    // 保存最后一帧游戏画面和分数，用于回放
    screenRecord.push_back(screen);
    ++screenCount;
    scoreRecord.push_back(score);

    // 绘制最后一帧游戏画面
    DrawMap();

    bool bPressed = false;
    bool lPressed = false;
    char key;

    while (true)
    {
        // 用户输入
        key = _getch();

        // 清空缓冲区
        while (_kbhit()) _getch();

        // 检查是否需要保存记录，并确保只执行一次
        if (key == 'b')
        {
            if (!bPressed)
            {
                SaveRecord();
                bPressed = true;
            }
            else
            {
                cout << "Error: Record already saved. Press 'l' to update leaderboard, or any other key to return to main menu." << endl;
            }
        }

        // 检查是否需要更新排行榜，并确保只执行一次
        else if (key == 'l')
        {
            if (!lPressed)
            {
                UpdateLeaderboard();
                lPressed = true;
            }
            else
            {
                cout << "Error: Leaderboard already updated. Press 'b' to save record, or any other key to return to main menu." << endl;
            }
        }
        // 如果用户按下其他键，退出循环
        else
        {
            break;
        }
    }
    // 如果用户选择既不保存记录也不更新排行榜，将直接退出
    cout << "Returning to main menu." << endl;
}



void SnakeGame::SaveRecord()
{
    filesystem::path dir = "record";
    if (!filesystem::exists(dir))
    {
        filesystem::create_directories(dir);
    }

    // 保存记录，输入记录文件名，如果文件名已存在则提示错误，如果文件名为q则取消保存
    string recordName;
    cout << "Enter the record file name: " << endl;
    cin >> recordName;
    if (recordName == "q")
    {
        return;
    }

    // 保存记录文件
    string recordPath = "record/" + recordName + ".rec";
    ifstream recordFile(recordPath);
    if (recordFile)
    {
        cout << "Record file already exists." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    ofstream newRecordFile(recordPath);
    if (!newRecordFile)
    {
        cout << "Failed to create record file." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    // 保存配置文件路径、地图文件路径、难度、地图大小、地图计数、游戏画面和分数
    newRecordFile << config.configPath << endl;
    newRecordFile << map.mapPath << endl;
    newRecordFile << config.gameDifficulty << endl;
    newRecordFile << map.height << " " << map.width << endl;
    newRecordFile << screenCount << endl;

    for (int i = 0; i < screenCount; ++i)
    {
        for (int j = 0; j <= map.height + 1; ++j)
        {
            for (int k = 0; k <= map.width + 1; ++k)
            {
                newRecordFile << screenRecord[i][j][k];
            }
            newRecordFile << endl;
        }
        newRecordFile << scoreRecord[i] << endl;
    }
    newRecordFile.close();

    cout << "Record saved." << endl;
}

void SnakeGame::Replay()
{
    // 回放，输入记录文件名，如果文件不存在则提示错误，如果文件名为q则取消回放
    string recordName;
    cout << "Enter the record file name: " << endl;
    cin >> recordName;
    if (recordName == "q")
    {
        return;
    }

    // 读取记录文件
    string recordPath = "record/" + recordName + ".rec";
    ifstream recordFile(recordPath);
    if (!recordFile)
    {
        cout << "Record file does not exist." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    // 读取配置文件路径、地图文件路径、难度、地图大小、地图计数、游戏画面和分数
    recordFile >> config.configPath;
    recordFile >> map.mapPath;
    recordFile >> config.gameDifficulty;
    recordFile >> map.height >> map.width;
    recordFile >> screenCount;

    screen.clear();
    screenRecord.resize(screenCount, vector<vector<char>>(map.height + 2, vector<char>(map.width + 2, '0')));
    scoreRecord.clear();
    scoreRecord.resize(screenCount);

    for (int i = 0; i < screenCount; ++i)
    {
        for (int j = 0; j <= map.height + 1; ++j)
        {
            for (int k = 0; k <= map.width + 1; ++k)
            {
                recordFile >> screenRecord[i][j][k];
            }
        }
        recordFile >> scoreRecord[i];
    }

    recordFile.close();

    replay = true;
    gameOver = false;

    // 回放游戏
    for (int i = 0; i < screenCount; ++i)
    {
        if (i == screenCount - 1)
        {
            gameOver = true;
        }
        score = scoreRecord[i];
        screen = screenRecord[i];
        DrawMap();

        DWORD endTime = GetTickCount64() + 1000 / config.gameDifficulty;
        char key = 0;

        while (GetTickCount64() < endTime)
        {
            if (_kbhit())
            {
                key = _getch();
            }
            // 按q退出回放
            if (key == 'q')
            {
                return;
            }
        }
    }
    char key = _getch();

    replay = false;
    gameOver = false;
}

void SnakeGame::CreateConfig()
{
    // 创建配置文件，输入配置文件名，如果文件名已存在则提示错误，如果文件名为q则取消创建
    string configName;
    cout << "Enter the configuration file name: ";
    cin >> configName;
    if (configName == "q")
    {
        return;
    }

    // 打开配置文件，如果文件已存在则提示错误，如果文件不存在则创建文件
    string configPath = "config/" + configName + ".config";
    ifstream configFile(configPath);
    if (configFile)
    {
        cout << "Configuration file already exists." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    ofstream newConfigFile(configPath);
    if (!newConfigFile)
    {
        cout << "Failed to create configuration file." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    // 输入难度、随机种子、食物数量、食物概率
    cout << "Enter the game difficulty (1-10): ";
    cin >> config.gameDifficulty;
    while (config.gameDifficulty < 1 || config.gameDifficulty > 10)
    {
        cout << "Invalid game difficulty. Please enter a number between 1 and 10: ";
        cin >> config.gameDifficulty;
    }

    cout << "Enter the random seed (-1 for current time): ";
    cin >> config.randomSeed;

    cout << "Enter the number of food items (1-5): ";
    cin >> config.numOfFood;
    while (config.numOfFood < 1 || config.numOfFood > 5)
    {
        cout << "Invalid number of food items. Please enter a number between 1 and 5: ";
        cin >> config.numOfFood;
    }

    cout << "Enter the probability of generating 1-point food (0-1): ";
    cin >> config.foodProb[0];
    while (config.foodProb[0] < 0 || config.foodProb[0] > 1)
    {
        cout << "Invalid probability. Please enter a number between 0 and 1: ";
        cin >> config.foodProb[0];
    }

    cout << "Enter the probability of generating 2-point food (0-1): ";
    cin >> config.foodProb[1];
    while (config.foodProb[1] < 0 || config.foodProb[0] + config.foodProb[1] > 1)
    {
        cout << "Invalid probability. Please enter a number between 0 and 1: ";
        cin >> config.foodProb[1];
    }

    cout << "Enter the probability of generating 3-point food (0-1): ";
    cin >> config.foodProb[2];
    while (config.foodProb[2] < 0 || config.foodProb[0] + config.foodProb[1] + config.foodProb[2] != 1.0)
    {
        cout << "Invalid probability. Please enter a number between 0 and 1: ";
        cin >> config.foodProb[2];
    }

    // 保存配置文件
    newConfigFile << config.gameDifficulty << endl;
    newConfigFile << config.randomSeed << endl;
    newConfigFile << config.numOfFood << endl;
    newConfigFile << config.foodProb[0] << " " << config.foodProb[1] << " " << config.foodProb[2] << endl;

    newConfigFile.close();

    cout << "Configuration created." << endl;
    cout << "Enter any key to go back to main menu." << endl;
    char key = _getch();
}

void SnakeGame::LoadConfig()
{
    // 加载上次使用的配置文件
    ifstream configPathFile("config/last.config");
    if (!configPathFile)
    {
        config.configPath = "config/default.config";
    }
    else
    {
        getline(configPathFile, config.configPath);
    }

    cout << "The current configuration file name is " << config.configPath << endl;
    cout << "Enter a configuration file name to load, or enter q to cancel: ";

    // 输入配置文件名，如果文件名为q则取消加载
    string configName;
    cin >> configName;
    if (configName == "q")
    {
        return;
    }

    // 打开配置文件，如果文件不存在则提示错误，如果文件存在则加载配置文件
    config.configPath = "config/" + configName + ".config";

    ifstream configFile(config.configPath);
    if (!configFile)
    {
        cout << "Failed to load configuration file." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    // 读取配置文件
    configFile >> config.gameDifficulty;
    configFile >> config.randomSeed;
    configFile >> config.numOfFood;
    configFile >> config.foodProb[0] >> config.foodProb[1] >> config.foodProb[2];

    configFile.close();

    // 保存配置文件路径
    ofstream lastConfigFile("config/last.config");
    if (!lastConfigFile)
    {
        cout << "Failed to save configuration file." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    lastConfigFile << config.configPath << endl;
    lastConfigFile.close();

    cout << "Configuration loaded." << endl;
    cout << "Enter any key to go back to main menu." << endl;
    char key = _getch();
}

void SnakeGame::LoadLastConfig()
{
    // 打开config文件夹，如果不存在则创建文件夹
    filesystem::path dir = "config";
    if (!filesystem::exists(dir))
    {
        filesystem::create_directories(dir);
    }

    // 检查default.config文件，如果不存在则创建文件
    filesystem::path defaultConfigPath = "config/default.config";
    if (!filesystem::exists(defaultConfigPath))
    {
        ofstream defaultConfigFile(defaultConfigPath);
        defaultConfigFile << "1\n-1\n1\n0.6 0.3 0.1\n";
    }

    // 检查last.config文件，如果不存在则创建文件，如果存在则读取配置文件路径
    ifstream configFile("config/last.config");
    if (!configFile)
    {
        config.configPath = "config/default.config";
        ofstream lastConfigFile("config/last.config");
        lastConfigFile << config.configPath << endl;
    }
    else
    {
        getline(configFile, config.configPath);
        configFile.close();
    }

    // 打开配置文件，如果文件不存在则提示错误，如果文件存在则加载配置文件
    ifstream lastConfigFile(config.configPath);
    if (!lastConfigFile)
    {
        config.configPath = "config/default.config";
        ofstream updateLastConfig("config/last.config");
        updateLastConfig << config.configPath << endl;
        lastConfigFile.open("config/default.config");
    }

    // 读取配置文件
    lastConfigFile >> config.gameDifficulty;
    lastConfigFile >> config.randomSeed;
    lastConfigFile >> config.numOfFood;
    lastConfigFile >> config.foodProb[0] >> config.foodProb[1] >> config.foodProb[2];

    lastConfigFile.close();
}

void SnakeGame::CreateMap()
{
    // 创建地图文件，输入地图文件名，如果文件名已存在则提示错误，如果文件名为q则取消创建
    string mapName;
    cout << "Enter the map file name: ";
    cin >> mapName;
    if (mapName == "q")
    {
        return;
    }

    // 打开地图文件，如果文件已存在则提示错误，如果文件不存在则创建文件
    string mapPath = "map/" + mapName + ".map";
    ifstream mapFile(mapPath);
    if (mapFile)
    {
        cout << "Map file already exists." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    ofstream newMapFile(mapPath);
    if (!newMapFile)
    {
        cout << "Failed to create map file." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    // 设置新地图的大小
    Map newMap;
    cout << "Enter the map map.width (8-20): ";
    cin >> newMap.width;
    while (newMap.width < 8 || newMap.width > 20)
    {
        cout << "Invalid map map.width. Please enter a number between 8 and 20: ";
        cin >> newMap.width;
    }

    cout << "Enter the map map.height (8-20): ";
    cin >> newMap.height;
    while (newMap.height < 8 || newMap.height > 20)
    {
        cout << "Invalid map map.height. Please enter a number between 8 and 20: ";
        cin >> newMap.height;
    }

    // 初始化新地图
    newMap.numOfObstacle = 0;
    newMap.obstacle.clear();
    newMap.real[UP] = 1;
    newMap.real[DOWN] = 1;
    newMap.real[LEFT] = 1;
    newMap.real[RIGHT] = 1;

    // 创建新地图
    char choice;
    int x, y;
    bool found = false;
    bool finished = false;
    screen.clear();
    screen.resize(newMap.height + 2, vector<char>(newMap.width + 2, '0'));

    // 设置障碍物和边界属性
    while (!finished)
    {
        system("cls");
        // 绘制画面，根据不同的字符，输出不同的字符和颜色
        for (int i = 0; i <= newMap.height + 1; ++i)
        {
            for (int j = 0; j <= newMap.width + 1; ++j)
            {
                if (screen[i][j] == 'O')
                {
                    cout << "\033[44mO\033[0m";
                }
                else if (i == 0)
                {
                    if (newMap.real[UP] == 1)
                    {
                        cout << "\033[41m-\033[0m";
                    }
                    else
                    {
                        cout << "\033[42m-\033[0m";
                    }
                }
                else if (i == newMap.height + 1)
                {
                    if (newMap.real[DOWN] == 1)
                    {
                        cout << "\033[41m-\033[0m";
                    }
                    else
                    {
                        cout << "\033[42m-\033[0m";
                    }
                }
                else if (j == 0)
                {
                    if (newMap.real[LEFT] == 1)
                    {
                        cout << "\033[41m|\033[0m";
                    }
                    else
                    {
                        cout << "\033[42m|\033[0m";
                    }
                }
                else if (j == newMap.width + 1)
                {
                    if (newMap.real[RIGHT] == 1)
                    {
                        cout << "\033[41m|\033[0m";
                    }
                    else
                    {
                        cout << "\033[42m|\033[0m";
                    }
                }
                else
                {
                    cout << " ";
                }
            }
            cout << endl;
        }

        // 设置障碍物和边界属性，o添加障碍物，p删除障碍物，w/a/s/d设置边界属性，f完成创建，q取消创建
        cout << "Enter o x y to add an obstacle at (x, y)." << endl;
        cout << "Enter p x y to remove an obstacle at (x, y)." << endl;
        cout << "Enter w 0/1 to set the up boundary to false/true." << endl;
        cout << "Enter s 0/1 to set the down boundary to false/true." << endl;
        cout << "Enter a 0/1 to set the left boundary to false/true." << endl;
        cout << "Enter d 0/1 to set the right boundary to false/true." << endl;
        cout << "Enter f to finish." << endl;
        cout << "Enter q to cancel." << endl;
        cout << "Enter your command: " << endl;
        cin >> choice;
        switch (choice)
        {
        case 'o':
            newMap.numOfObstacle++;
            cin >> x >> y;
            while (x < 0 || x >= newMap.width || y < 0 || y >= newMap.height)
            {
                cout << "Invalid coordinates. Please enter a number between 0 and " << newMap.width - 1 << " for x, and 0 and " << map.height - 1 << " for y: ";
                cin >> x >> y;
            }
            newMap.obstacle.push_back({x, y});
            screen[y + 1][x + 1] = 'O';
            break;
        case 'p':
            cin >> x >> y;
            while (x < 0 || x >= newMap.width || y < 0 || y >= newMap.height)
            {
                cout << "Invalid coordinates. Please enter a number between 0 and " << newMap.width - 1 << " for x, and 0 and " << newMap.height - 1 << " for y: ";
                cin >> x >> y;
            }
            for (int i = 0; i < newMap.numOfObstacle; ++i)
            {
                if (newMap.obstacle[i].x == x && newMap.obstacle[i].y == y)
                {
                    newMap.obstacle.erase(newMap.obstacle.begin() + i);
                    newMap.numOfObstacle--;
                    screen[y + 1][x + 1] = '0';
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                cout << "No obstacle at (" << x << ", " << y << ")." << endl;
            }
            break;
        case 'w':
            int value;
            cin >> value;
            while (value != 0 && value != 1)
            {
                cout << "Invalid value. Please enter 0 or 1: ";
                cin >> value;
            }
            newMap.real[UP] = value;
            break;
        case 's':
            cin >> value;
            while (value != 0 && value != 1)
            {
                cout << "Invalid value. Please enter 0 or 1: ";
                cin >> value;
            }
            newMap.real[DOWN] = value;
            break;
        case 'a':
            cin >> value;
            while (value != 0 && value != 1)
            {
                cout << "Invalid value. Please enter 0 or 1: ";
                cin >> value;
            }
            newMap.real[LEFT] = value;
            break;
        case 'd':
            cin >> value;
            while (value != 0 && value != 1)
            {
                cout << "Invalid value. Please enter 0 or 1: ";
                cin >> value;
            }
            newMap.real[RIGHT] = value;
            break;
        case 'f':
            finished = true;
            break;
        case 'q':
            return;
        default:
            cout << "Invalid choice. Please try again." << endl;
            break;
        }
    }

    // 保存地图文件
    newMapFile << newMap.width << " " << newMap.height << endl;
    newMapFile << newMap.real[UP] << " " << newMap.real[DOWN] << " " << newMap.real[LEFT] << " " << newMap.real[RIGHT] << endl;
    newMapFile << newMap.numOfObstacle << endl;
    if (newMap.numOfObstacle > 0)
    {
        for (int i = 0; i < newMap.numOfObstacle; ++i)
        {
            newMapFile << newMap.obstacle[i].x << " " << newMap.obstacle[i].y << endl;
        }
    }

    newMapFile.close();

    cout << "Map created." << endl;
    cout << "Enter any key to go back to main menu." << endl;
    char key = _getch();
}

void SnakeGame::LoadMap()
{
    // 加载上次使用的地图文件
    ifstream mapPathFile("map/last.map");
    if (!mapPathFile)
    {
        map.mapPath = "map/default.map";
    }
    else
    {
        getline(mapPathFile, map.mapPath);
    }

    cout << "The current map file name is " << map.mapPath << "." << endl;
    cout << "Enter a map file name to load, or enter q to cancel: ";

    // 输入地图文件名，如果文件名为q则取消加载
    string mapName;
    cin >> mapName;
    if (mapName == "q")
    {
        return;
    }

    map.mapPath = "map/" + mapName + ".map";

    // 打开地图文件，如果文件不存在则提示错误，如果文件存在则加载地图文件
    ifstream mapFile(map.mapPath);
    if (!mapFile)
    {
        cout << "Failed to load map file." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    map.numOfObstacle = 0;
    map.obstacle.clear();

    // 读取地图文件
    mapFile >> map.width >> map.height;
    mapFile >> map.real[UP] >> map.real[DOWN] >> map.real[LEFT] >> map.real[RIGHT];
    mapFile >> map.numOfObstacle;
    if (map.numOfObstacle > 0)
    {
        for (int i = 0; i < map.numOfObstacle; ++i)
        {
            int x, y;
            mapFile >> x >> y;
            map.obstacle.push_back({x, y});
        }
    }

    mapFile.close();

    // 保存地图文件路径
    ofstream lastMapFile("map/last.map");
    if (!lastMapFile)
    {
        cout << "Failed to save map file." << endl;
        cout << "Enter any key to go back to main menu." << endl;
        char key = _getch();
        return;
    }

    lastMapFile << map.mapPath << endl;
    lastMapFile.close();

    cout << "Map loaded." << endl;
    cout << "Enter any key to go back to main menu." << endl;
    char key = _getch();
}

void SnakeGame::LoadLastMap()
{
    // 打开map文件夹，如果不存在则创建文件夹
    filesystem::path dir = "map";
    if (!filesystem::exists(dir))
    {
        filesystem::create_directories(dir);
    }

    // 检查default.map文件，如果不存在则创建文件
    filesystem::path defaultMapPath = "map/default.map";
    if (!filesystem::exists(defaultMapPath))
    {
        ofstream defaultMapFile(defaultMapPath);
        defaultMapFile << "15 15\n1 1 1 1\n0\n";
    }

    // 检查last.map文件，如果不存在则创建文件，如果存在则读取地图文件路径
    ifstream mapFile("map/last.map");
    if (!mapFile)
    {
        map.mapPath = "map/default.map";
        ofstream lastMapFile("map/last.map");
        lastMapFile << map.mapPath << endl;
    }
    else
    {
        getline(mapFile, map.mapPath);
        mapFile.close();
    }

    // 打开地图文件，如果文件不存在则提示错误，如果文件存在则加载地图文件
    ifstream lastMapFile(map.mapPath);
    if (!lastMapFile)
    {
        map.mapPath = "map/default.map";
        ofstream updateLastMap("map/last.map");
        updateLastMap << map.mapPath << endl;
        lastMapFile.open("map/default.map");
    }

    map.numOfObstacle = 0;
    map.obstacle.clear();

    // 读取地图文件
    lastMapFile >> map.width >> map.height;
    lastMapFile >> map.real[UP] >> map.real[DOWN] >> map.real[LEFT] >> map.real[RIGHT];
    lastMapFile >> map.numOfObstacle;

    if (map.numOfObstacle > 0)
    {
        for (int i = 0; i < map.numOfObstacle; ++i)
        {
            int x, y;
            lastMapFile >> x >> y;
            map.obstacle.push_back({x, y});
        }
    }

    lastMapFile.close();
}

void SnakeGame::UpdateLeaderboard()
{
    // 打开leaderboard文件夹，如果不存在则创建文件夹
    filesystem::path dir = "leaderboard";
    if (!filesystem::exists(dir))
    {
        filesystem::create_directories(dir);
    }

    // 检查leaderboard文件，如果不存在则创建文件
    filesystem::path leaderboardPath = "leaderboard/leaderboard.txt";
    if (!filesystem::exists(leaderboardPath))
    {
        ofstream leaderboardFile(leaderboardPath);
        leaderboardFile << "Name Score Date Time Configuration Map\n";
    }

    // 读取leaderboard文件
    ifstream leaderboardFile(leaderboardPath);
    string line;
    leaderboard.clear();
    while (getline(leaderboardFile, line))
    {
        if (line == "Name Score Date Time Configuration Map")
        {
            continue;
        }
        std::stringstream ss(line);
        LeaderboardEntry entry;
        ss >> entry.name >> entry.score >> entry.date >> entry.time >> entry.configPath >> entry.mapPath;
        leaderboard.push_back(entry);
    }
    leaderboardFile.close();
    
    // 获取玩家姓名
    string name;
    cout << "Enter your name: ";
    cin >> name;
    playerName = name;

    // 获取当前时间
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string date = to_string(1900 + ltm->tm_year) + "/" + to_string(1 + ltm->tm_mon) + "/" + to_string(ltm->tm_mday);
    string time = to_string(ltm->tm_hour) + ":" + to_string(ltm->tm_min) + ":" + to_string(ltm->tm_sec);

    // 添加新记录
    LeaderboardEntry newEntry;
    newEntry.name = playerName;
    newEntry.score = score;
    newEntry.date = date;
    newEntry.time = time;
    newEntry.configPath = config.configPath;
    newEntry.mapPath = map.mapPath;
    leaderboard.push_back(newEntry);

    // 排序
    sort(leaderboard.begin(), leaderboard.end(), [](const LeaderboardEntry &a, const LeaderboardEntry &b)
         { return a.score > b.score; });

    // 保存leaderboard文件
    ofstream newLeaderboardFile(leaderboardPath);
    newLeaderboardFile << "Name Score Date Time Configuration Map\n";
    for (int i = 0; i < leaderboard.size(); ++i)
    {
        newLeaderboardFile << leaderboard[i].name << " " << leaderboard[i].score << " " << leaderboard[i].date << " " << leaderboard[i].time << " " << leaderboard[i].configPath << " " << leaderboard[i].mapPath << endl;
    }
    newLeaderboardFile.close();

    cout << "Leaderboard updated." << endl;
}

void SnakeGame::DisplayLeaderboard()
{
    // 打开leaderboard文件夹，如果不存在则创建文件夹
    filesystem::path dir = "leaderboard";
    if (!filesystem::exists(dir))
    {
        filesystem::create_directories(dir);
    }

    // 检查leaderboard文件，如果不存在则创建文件
    filesystem::path leaderboardPath = "leaderboard/leaderboard.txt";
    if (!filesystem::exists(leaderboardPath))
    {
        ofstream leaderboardFile(leaderboardPath);
        leaderboardFile << "Name Score Date Time Configuration Map\n";
    }

    // 读取leaderboard文件
    ifstream leaderboardFile(leaderboardPath);
    string line;
    leaderboard.clear();
    while (getline(leaderboardFile, line))
    {
        if (line == "Name Score Date Time Configuration Map")
        {
            continue;
        }
        std::stringstream ss(line);
        LeaderboardEntry entry;
        ss >> entry.name >> entry.score >> entry.date >> entry.time >> entry.configPath >> entry.mapPath;
        leaderboard.push_back(entry);
    }
    leaderboardFile.close();

    // 排序
    sort(leaderboard.begin(), leaderboard.end(), [](const LeaderboardEntry &a, const LeaderboardEntry &b)
         { return a.score > b.score; });

    // 输出leaderboard
    system("cls");

    cout << left << setw(5) << "Rank"
         << setw(20) << "Name"
         << setw(10) << "Score"
         << setw(15) << "Date"
         << setw(10) << "Time"
         << setw(30) << "Configuration"
         << setw(30) << "Map" << endl;

    for (int i = 0; i < leaderboard.size(); ++i)
    {
        cout << left << setw(5) << i + 1
             << setw(20) << leaderboard[i].name
             << setw(10) << leaderboard[i].score
             << setw(15) << leaderboard[i].date
             << setw(10) << leaderboard[i].time
             << setw(30) << leaderboard[i].configPath
             << setw(30) << leaderboard[i].mapPath << endl;
    }
    cout << "Enter any key to go back to main menu." << endl;
    char key = _getch();
}

// 主函数
int main()
{
    SnakeGame snakeGame;
    snakeGame.Init();

    char choice;
    do
    {
        system("cls");
        cout << "Snake - Fundamentals of Programming" << endl;
        cout << "-----------------------------------" << endl;
        cout << "g: Start Game" << endl;
        cout << "q: Quit Game" << endl;
        cout << "i: Create Configuration" << endl;
        cout << "u: Load Configuration" << endl;
        cout << "n: Create Map" << endl;
        cout << "m: Load Map" << endl;
        cout << "r: Replay" << endl;
        cout << "l: display leaderboard" << endl;

        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case 'g':
            snakeGame.Run();
            break;
        case 'q':
            cout << "Goodbye!" << endl;
            break;
        case 'i':
            snakeGame.CreateConfig();
            break;
        case 'u':
            snakeGame.LoadConfig();
            break;
        case 'n':
            snakeGame.CreateMap();
            break;
        case 'm':
            snakeGame.LoadMap();
            break;
        case 'r':
            snakeGame.Replay();
            break;
        case 'l':
            snakeGame.DisplayLeaderboard();
            break;
        default:
            cout << "Invalid choice. Please try again." << endl;
            break;
        }
    } while (choice != 'q');
    return 0;
}
