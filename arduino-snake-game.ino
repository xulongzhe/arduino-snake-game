#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <MsTimer2.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define STATUS_BAR_HEIGHT 10
#define GRID_SIZE 3
#define GRID_WIDTH SCREEN_WIDTH / GRID_SIZE
#define GRID_HEIGHT (SCREEN_HEIGHT - STATUS_BAR_HEIGHT) / GRID_SIZE

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SPEED 1
#define DIRECTION_RIGHT 1
#define DIRECTION_LEFT 2
#define DIRECTION_UP 3
#define DIRECTION_DOWN 4

class Node
{
private:
public:
  int x;
  int y;
  Node *next = NULL;

  Node(int x, int y)
  {
    this->x = x;
    this->y = y;
  }

  Node()
  {
  }

  ~Node()
  {
    Serial.println("delete node");
  }

  void show()
  {
    display.drawRect(x * GRID_SIZE, y * GRID_SIZE + (STATUS_BAR_HEIGHT / GRID_SIZE * GRID_SIZE), GRID_SIZE, GRID_SIZE, SSD1306_WHITE);
  }

  // 设置节点位置，范围不会超过
  void setLocation(int x, int y)
  {
    if (x > GRID_WIDTH)
    {
      this->x = 0;
    }
    else if (x < 0)
    {
      this->x = GRID_WIDTH;
    }
    else
    {
      this->x = x;
    }

    if (y > GRID_HEIGHT)
    {
      this->y = 0;
    }
    else if (y < 0)
    {
      this->y = GRID_HEIGHT;
    }
    else
    {
      this->y = y;
    }
  }
};

class Food : public Node
{
public:
  Food(int x, int y) : Node(x, y) {}
};

class Snake
{
private:
public:
  Node *head;
  int length = 1;
  int direction = DIRECTION_DOWN;
  Snake(int x, int y)
  {
    head = new Node(x, y);
  }

  ~Snake()
  {
    Serial.println("delete snake");
    Node *current = head;
    while (current->next != NULL)
    {
      Node *next = current->next;
      delete current;
      current = next;
    }
  }

  // 尝试吃食物（食物与蛇头的碰撞检测）
  bool tryEat(Food *food)
  {
    if ((abs(head->x - food->x) == 2 && head->y == food->y) || (abs(head->y - food->y) == 2 && head->x == food->x))
    {
      Serial.println("eat");
      eat(food);
      return true;
    }
    else
    {
      return false;
    }
  }

  // 吃掉食物后长大（将食物所在节点作为新的头部节点）
  void eat(Food *food)
  {
    Node *newHead = new Node(food->x, food->y);
    delete food;
    newHead->next = head;
    head = newHead;
    length++;
  }

  // 将蛇向某个点移动
  void moveTo(int x, int y)
  {
    // 将切掉的尾部节点作为新的头部节点
    Node *newHead = cutTail();
    newHead->setLocation(x, y);
    if (newHead != head)
    {
      newHead->next = head;
      head = newHead;
    }
  }

  // 切掉蛇的最后一个节点，并将此节点返回（如果只有一个节点，则返回头部节点）
  Node *cutTail()
  {
    Serial.println("cut tail");
    // 用于存放倒数第二个节点
    Node *pre = NULL;
    // 当前节点设为头部节点
    Node *current = head;
    // 遍历获取最后一个节点和倒数第二个节点
    while (current->next)
    {
      pre = current;
      current = current->next;
    }

    // 如果有倒数第二个节点，则倒数第二个节点变成最后一个节点
    if (pre)
    {
      pre->next = NULL;
    }

    return current;
  }

  void left()
  {
    direction = DIRECTION_LEFT;
    Serial.println("left");
  }

  void right()
  {
    direction = DIRECTION_RIGHT;
    Serial.println("right");
  }

  void up()
  {
    direction = DIRECTION_UP;
    Serial.println("up");
  }

  void down()
  {
    direction = DIRECTION_DOWN;
    Serial.println("down");
  }

  void move()
  {
    switch (direction)
    {
    case DIRECTION_LEFT:
      Serial.println("move left");
      moveTo(head->x - SPEED, head->y);
      break;
    case DIRECTION_RIGHT:
      Serial.println("move right");
      moveTo(head->x + SPEED, head->y);
      break;
    case DIRECTION_UP:
      Serial.println("move up");
      moveTo(head->x, head->y - SPEED);
      break;
    case DIRECTION_DOWN:
      Serial.println("move down");
      moveTo(head->x, head->y + SPEED);
      break;
    default:
      Serial.println("unknown direction");
      break;
    }
  }

  // 绘制整个蛇
  void show()
  {
    Node *current = head;
    current->show();
    while (current->next)
    {
      current = current->next;
      current->show();
    }
  }
};

class Game
{
public:
  Snake *snake;
  Food *food;
  int score = 0;

  void init()
  {
    createFood();
    createSnake();
  }

  void createSnake()
  {
    snake = new Snake(random(GRID_WIDTH), random(GRID_HEIGHT));
  }

  // 随机创建一个食物
  void createFood()
  {
    Serial.println("creat new food");
    food = new Food(random(GRID_WIDTH), random(GRID_HEIGHT));
  }

  void refresh()
  {
    display.clearDisplay();
    snake->show();
    food->show();
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(F("SCORE:"));
    display.print(score);
    display.drawFastHLine(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SSD1306_WHITE);
    display.display();
    if (snake->tryEat(food))
    {
      score++;
      createFood();
    }
    snake->move();
  }

  void acceptInput()
  {
    int c = Serial.read();
    if (c != -1)
    {
      switch (c)
      {
      case DIRECTION_LEFT:
        snake->left();
        break;
      case DIRECTION_RIGHT:
        snake->right();
        break;
      case DIRECTION_UP:
        snake->up();
        break;
      case DIRECTION_DOWN:
        snake->down();
        break;
      default:
        Serial.println("unknown command");
        break;
      }
    }
  }
};

// 是否到达刷新时间
static bool timeUp = false;
static Game *game = new Game();

// 定时器触发时将标志位设为真，这里不能直接将逻辑写在定时器中，因为业务逻辑中包含串口IO，耗时过长。
void refreshTimer()
{
  timeUp = true;
}

void setup()
{
  Serial.begin(38400);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  // 清除屏幕
  display.clearDisplay();
  display.display();

  // 启动定时器，每隔一段时间刷新游戏界面
  MsTimer2::set(200, refreshTimer);
  MsTimer2::start();

  game->init();
}

void loop()
{
  if (timeUp)
  {
    game->refresh();
    timeUp = false;
  }
  game->acceptInput();
}
