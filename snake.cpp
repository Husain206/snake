#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iostream>
#include <termios.h>
#include <unistd.h>

using namespace std;

#define HEIGHT 20
#define WIDTH 30

enum direction {
  STOP,
  LEFT,
  RIGHT,
  UP,
  DOWN,
};
direction dir = UP;

struct pos {
  int x, y;
};

deque<pos> snake = {{WIDTH / 2, HEIGHT / 2}};
pos food;

bool posEq(const pos &a, const pos &b) { return a.x == b.x && a.y == b.y; }

struct Theme {
  string head, body, food, bg, reset;
};

// Define multiple themes
Theme classic = {.head = "O",
                 .body = "o",
                 .food = "@",
                 .bg = "\033[48;5;235m", // Dark gray background
                 .reset = "\033[0m"};

Theme square = {.head = "\033[38;5;46m█",
                .body = "\033[38;5;34m▓",
                .food = "\033[38;5;196m■",
                .bg = "\033[48;5;235m",
                .reset = "\033[0m"};

Theme circle = {.head = "\033[38;5;220m◉",
                .body = "\033[38;5;228m●",
                .food = "\033[38;5;196m✿",
                .bg = "\033[48;5;235m",
                .reset = "\033[0m"};
// food: Or ▣, ✚, ◆, ⬤ etc. rememeber you put this to copy the emojies xDDDDD

Theme theme = circle;

struct termios org_ter;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &org_ter);
  cout << "\e[?25h";
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &org_ter);
  atexit(disableRawMode);
  struct termios raw = org_ter;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void moveCursor(int x, int y) { cout << "\033[" << y << ";" << x << "H"; }

void hideCursor() { cout << "\e[?25l"; }

char readInput() {
  char ch = 0;
  ssize_t n = read(STDIN_FILENO, &ch, 1);
  return (n == 1) ? ch : 0;
}

void placeFood() {
  while (true) {
    food.x = rand() % WIDTH;
    food.y = rand() % HEIGHT;
    bool onSnake = false;
    for (auto &s : snake) {
      if (posEq(s, food)) {
        onSnake = true;
        break;
      }
    }
    if (!onSnake)
      break;
  }
}

void draw() {
  char grid[HEIGHT][WIDTH];

  for (int y = 0; y < HEIGHT; y++)
    for (int x = 0; x < WIDTH; x++)
      grid[y][x] = ' ';

  grid[snake.front().y][snake.front().x] = 'H';

  for (size_t i = 1; i < snake.size(); ++i)
    grid[snake[i].y][snake[i].x] = 'B';

  grid[food.y][food.x] = 'F';

  moveCursor(1, 1);
  cout << theme.bg;

  for (int x = 0; x <= WIDTH + 1; x++)
    cout << '#';
  cout << '\n';

  for (int y = 0; y < HEIGHT; y++) {
    cout << '#';
    for (int x = 0; x < WIDTH; x++) {
      switch (grid[y][x]) {
      case 'H':
        cout << theme.head;
        break;
      case 'B':
        cout << theme.body;
        break;
      case 'F':
        cout << theme.food;
        break;
      default:
        cout << ' ';
      }
    }
    cout << "#\n";
  }

  for (int x = 0; x <= WIDTH + 1; x++)
    cout << '#';
  cout << theme.reset;
  cout << "\nScore: " << snake.size() - 1 << "\n";
}

void update() {
  pos head = snake.front();

  switch (dir) {
  case UP:
    head.y--;
    break;
  case DOWN:
    head.y++;
    break;
  case LEFT:
    head.x--;
    break;
  case RIGHT:
    head.x++;
    break;
  default:
    break;
  }

  if (head.x < 0 || head.x >= WIDTH || head.y < 0 || head.y >= HEIGHT ||
      any_of(
          snake.begin() + 1, snake.end(),
          [&](const pos &p) { return posEq(p, head); })) {
    moveCursor(1, HEIGHT + 4);
    cout << theme.reset << "Game Over! Final Score: " << snake.size() - 1
         << "\n";
    exit(0);
  }

  snake.push_front(head);

  if (posEq(head, food)) {
    placeFood();
  } else {
    snake.pop_back();
  }
}

direction getDirection(char c) {
  if (c == 27) {
    char seq[2];
    if (read(STDIN_FILENO, &seq[0], 1) && read(STDIN_FILENO, &seq[1], 1)) {
      if (seq[0] == '[') {
        switch (seq[1]) {
        case 'A':
          return UP;
        case 'B':
          return DOWN;
        case 'C':
          return RIGHT;
        case 'D':
          return LEFT;
        }
      }
    }
  } else if (c == 'q') {
    moveCursor(1, HEIGHT + 4);
    cout << theme.reset << "Quit.\n";
    exit(0);
  }
  return dir;
}

int main() {
  srand(time(0));
  enableRawMode();
  hideCursor();
  placeFood();

  while (true) {
    char c = readInput();
    direction next = getDirection(c);
    if ((dir == UP && next != DOWN) || (dir == DOWN && next != UP) ||
        (dir == LEFT && next != RIGHT) || (dir == RIGHT && next != LEFT)) {
      dir = next;
    }

    update();
    draw();
    usleep(100000);
  }

  return 0;
}
