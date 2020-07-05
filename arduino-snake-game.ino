#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <MsTimer2.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

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
    display.drawPixel(x, y, SSD1306_WHITE);
  }

  // 设置节点位置，范围不会超过
  void setLocation(int x, int y)
  {
    if (x > SCREEN_WIDTH)
    {
      this->x = 0;
    }
    else if (x < 0)
    {
      this->x = SCREEN_WIDTH;
    }
    else
    {
      this->x = x;
    }

    if (y > SCREEN_HEIGHT)
    {
      this->y = 0;
    }
    else if (y < 0)
    {
      this->y = SCREEN_HEIGHT;
    }
    else
    {
      this->y = y;
    }
  }

  char *toString()
  {
    char *str = new char[20];
    sprintf(str, "[%d,%d]", x, y);
    return str;
  }

  void print()
  {
    Serial.println(toString());
  }
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
  bool tryEat(Node *food)
  {
    if ((abs(head->x - food->x) == 1 && head->y == food->y) || (abs(head->y - food->y) == 1 && head->x == food->x))
    {
      Serial.println("eat");
      growUp(food);
      return true;
    }
    else
    {
      return false;
    }
  }

  // 吃掉食物后长大（将食物所在节点作为新的头部节点）
  void growUp(Node *node)
  {
    node->next = head;
    head = node;
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
    do
    {
      current->show();
      current = current->next;
    } while (current->next);
  }

  String print()
  {
    Serial.println("======================");
    Node *current = this->head;
    while (current->next != NULL)
    {
      current = current->next;
    }
    Serial.println("======================");
  }
};

Snake *snake = new Snake(random(SCREEN_WIDTH), random(SCREEN_HEIGHT));
Node *food;

// 是否到达刷新时间
static bool timeUp = false;

void refreshTimer()
{
  timeUp = true;
}

// 随机创建一个食物
void creatFood()
{
  Serial.println("creat new food");
  food = new Node(random(SCREEN_WIDTH), random(SCREEN_HEIGHT));
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

  // 创建一个食物
  creatFood();
}

void loop()
{
  display.clearDisplay();
  snake->show();
  food->show();
  display.display();
  if (timeUp)
  {
    if (snake->tryEat(food))
    {
      creatFood();
    }
    snake->move();
    timeUp = false;
  }
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
