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

class SnakeNode
{
private:
public:
  int x;
  int y;
  SnakeNode *next = NULL;

  SnakeNode(int x, int y)
  {
    this->x = x;
    this->y = y;
  }

  SnakeNode()
  {
  }

  ~SnakeNode()
  {
    Serial.println("delete node");
  }

  void show()
  {
    display.drawPixel(x, y, SSD1306_WHITE);
  }

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
  SnakeNode *head;
  int length = 1;
  int direction = DIRECTION_DOWN;
  Snake()
  {
    head = new SnakeNode(10, 10);
  }

  ~Snake()
  {
    Serial.println("delete snake");
    SnakeNode *current = head;
    while (current->next != NULL)
    {
      SnakeNode *next = current->next;
      delete current;
      current = next;
    }
  }

  void addHead(int x, int y)
  {
    Serial.println("add head");
    SnakeNode *newHead = cutTail();
    newHead->setLocation(x, y);
    if (newHead != head)
    {
      Serial.println("not equel");
      newHead->next = head;
      head = newHead;
    }
  }

  SnakeNode *cutTail()
  {
    Serial.println("cut tail");
    SnakeNode *current = head;
    while (current->next)
    {
      current = current->next;
      char s[100];
      sprintf(s, "in %x", &current);
      Serial.println(s);
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
    showSnake();
    switch (direction)
    {
    case DIRECTION_LEFT:
      Serial.println("move left");
      addHead(head->x - SPEED, head->y);
      break;
    case DIRECTION_RIGHT:
      Serial.println("move right");
      addHead(head->x + SPEED, head->y);
      break;
    case DIRECTION_UP:
      Serial.println("move up");
      addHead(head->x, head->y - SPEED);
      break;
    case DIRECTION_DOWN:
      Serial.println("move down");
      addHead(head->x, head->y + SPEED);
      break;
    default:
      Serial.println("unknown direction");
      break;
    }
  }

  void showSnake()
  {
    display.clearDisplay();
    SnakeNode *current = head;
    do
    {
      current->show();
      current = current->next;
    } while (current->next);
    display.display();
  }

  String print()
  {
    Serial.println("======================");
    SnakeNode *current = this->head;
    while (current->next != NULL)
    {
      current = current->next;
    }
    Serial.println("======================");
  }
};

Snake *snake = new Snake();

static bool timeUp = false;

void myTimer()
{
  timeUp = true;
  Serial.println(timeUp);
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
  display.clearDisplay();
  display.display();
  Serial.println("timmer start");
  MsTimer2::set(500, myTimer);
  MsTimer2::start();
  Serial.println("timmer started");
}

void loop()
{
  if (timeUp)
  {
    Serial.println("move");
    snake->move();
    timeUp = false;
  }
  int c = Serial.read();
  if (c != -1)
  {
    Serial.println(c);
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
      Serial.println("unknown " + c);
      break;
    }
  }
}
