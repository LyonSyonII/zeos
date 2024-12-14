#include "libc.h"
#include "queue.h"
#include "types.h"

#define WIDTH 80
#define HEIGHT 25
#define LEFT -1
#define RIGHT 1
#define UP -1
#define DOWN 1

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define TASK(task) (void(*)(void*))(task)

typedef struct {
  int delay;
  int prev;
} delay_t;

typedef struct {
  keyboard_buffer buf;
} keyboard_params;

typedef struct {
  struct sem_t* start;
  struct sem_t* end;
  Word* buf;
} screen_params;

typedef struct {
  char sprite;
  SByte x;
  SByte y;
  colors_fg fg;
  colors_bg bg;
} player_t;

typedef struct {
  char sprite;
  SByte x;
  SByte y;
  SByte direction;
  colors_fg fg;
  colors_bg bg;
} enemy_t;

typedef struct {
  uint delay;
  Byte num_enemies;
  enemy_t enemies[];
} level_t;

extern enemy_t* levels[];
extern Byte level_num_enemies[];

void task_screen(screen_params* params);
void task_keyboard(keyboard_params* params);
void enemy_update(enemy_t enemies[], Byte num_enemies);
void player_update(player_t* player, char input);

void delay_tick(uint* delay) {
  static uint prev = 0;
  uint time = gettime();
  if (prev == time) return;
  prev = time;
  *delay -= 1;
}

void wait(int ticks) {
  int prev = -1;
  while (ticks > 0) {
    int time = gettime();
    if (prev == time) continue;
    prev = time;
    ticks -= 1;
  }
}

// Delay for game loop
const int delay = 150;
// Player
const char P = 2;
// Blank
const char B = ' ';

int __attribute__ ((__section__(".text.main"))) main(void) {
  keyboard_params keyboard = {
    .buf = KBUF_NEW(2),
  };
  screen_params screen = {
    .buf = (Word*)memRegGet(1),
    .start = semCreate(0),
    .end = semCreate(0)
  };
  player_t player = {
    .sprite = P,
    .x = WIDTH/2,
    .y = HEIGHT-1,
    .fg = FG_LIGHT_BLUE,
    .bg = BG_BLACK
  };
  Byte level = 0;
  enemy_t* enemies = levels[level];
  Byte num_enemies = level_num_enemies[level];

  char input = 0;

  
  clrscr(NULL);
  threadCreateWithStack(TASK(task_screen), 1, &screen);
  threadCreateWithStack(TASK(task_keyboard), 1, &keyboard);
  
  // START SCREEN
  gotoXY(28, HEIGHT/2);
  changeColor(FG_LIGHT_GREEN, BG_BLINKING_BLACK);
  println("Press any key to start!", FD_SCREEN);
  while (!kbuf_pop(&keyboard.buf, &input)) continue;
  clrscr(NULL);
  kbuf_push(&keyboard.buf, input);
  
  // GAME LOOP
  while (1) {
    if (kbuf_pop(&keyboard.buf, &input)) {
      write_xy(player.sprite, player.x, player.y, FG_BLACK, BG_BLACK, screen.buf);
      player_update(&player, input);
      write_xy(player.sprite, player.x, player.y, player.fg, player.bg, screen.buf);
    }
    
    for (int i = 0; i < num_enemies; i++)
      write_xy(enemies[i].sprite, enemies[i].x, enemies[i].y, FG_BLACK, BG_BLACK, screen.buf);
    enemy_update(enemies, num_enemies);
    for (int i = 0; i < num_enemies; i++)
      write_xy(enemies[i].sprite, enemies[i].x, enemies[i].y, enemies[i].fg, enemies[i].bg, screen.buf);
    
    semSignal(screen.start);
    semWait(screen.end);
    wait(delay);
  }
}

void task_screen(screen_params* params) {
  println("[screen] Spawned Screen Thread", FD_BOCHS);
  while (1) {
    semWait(params->start);
    clrscr((char*)params->buf);
    semSignal(params->end);
  }
}

void task_keyboard(keyboard_params* params) {
  println("[keybrd] Spawned Keyboard", FD_BOCHS);
  char c;
  while (1) {
    int ret = getKey(&c, 1 << 30);
    if (ret >= 0) {
      if (kbuf_push(&params->buf, c) == 0) {
          KBUF_ITER_CONSUME(params->buf);
          kbuf_push(&params->buf, c);
          // println("[keybrd] Buffer full, emptying", FD_BOCHS);
      }
    } else {
      printf("[keybrd] Error receiving character %d\n", &ret);
    }
  }
}

void player_update(player_t* player, char input) {
    switch (input) {
      case 'w': {
        player->y = max(player->y+UP, 0);
        break;
      }
      case 's': {
        player->y = min(player->y+DOWN, HEIGHT-1);
        break;
      }
      case 'a': {
        player->x += LEFT;
        if (player->x < 0) player->x = WIDTH-1;
        break;
      }
      case 'd': {
        player->x += RIGHT;
        if (player->x > WIDTH-1) player->x = 0;
        break;
      }
    }
}

void enemy_update(enemy_t enemies[], Byte num_enemies) {

}

#define block_e(x, y, direction) (enemy_t) { .sprite = 8, x, y, direction }

enemy_t level1[] = {
block_e(WIDTH/2, HEIGHT-5, 1)
};
enemy_t* levels[] = {
  level1
};
Byte level_num_enemies[] = {
  sizeof(level1)
};