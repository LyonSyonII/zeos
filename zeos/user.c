#include "libc.h"
#include "types.h"

#define SECOND 1000 //! CHANGE THIS TO A SECOND ON YOUR MACHINE IF THE GAME IS TOO SLOW/FAST

#define WIDTH 80
#define REALWIDTH 79
#define HEIGHT 25
#define REALHEIGHT 24
#define LEFT -1
#define RIGHT 1
#define UP -1
#define DOWN 1
#define STANDARD_DELAY (SECOND/20)

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define TASK(task) (void(*)(void*))(task)
#define print_centered(msg, offset_y) \
  gotoXY(40-sizeof(msg)/2, HEIGHT/2+offset_y); \
  changeColor(FG_LIGHT_GREEN, BG_BLINKING_BLACK); \
  println(msg, FD_SCREEN)

typedef struct {
  volatile char input;
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
  unsigned short delay;
  short remaining_delay;
  int prev;
} player_t;

typedef struct {
  char sprite;
  SByte x;
  SByte y;
  SByte direction;
  colors_fg fg;
  colors_bg bg;
  unsigned short delay;
  short remaining_delay;
  int prev;
} enemy_t;

typedef struct {
  Byte num_enemies;
  enemy_t* enemies;
} level_t;

static level_t levels[];
static Byte num_levels;

void task_screen(screen_params* params);
void task_keyboard(keyboard_params* params);
void start_screen(player_t* player, keyboard_params* keyboard, screen_params* screen);
void level_transition(Byte* level_id, player_t* player, keyboard_params* keyboard, screen_params* screen);
void player_update(player_t* player, char input, Word* screen);
int enemy_update(int time, level_t* level, Word* screen, SByte px, SByte py);
void wait(int ticks);
int update_delay(int time, short* remaining_delay, int* prev, unsigned short delay);
void clearScreen(screen_params* screen) {
  for (int i = 0; i < WIDTH * HEIGHT - 1; ++i) {
    screen->buf[i] = 0;
  }
  semSignal(screen->start);
  semWait(screen->end);
}

int __attribute__ ((__section__(".text.main"))) main(void) {
  keyboard_params keyboard = {
    .input = 0,
  };
  screen_params screen = {
    .buf = (Word*)memRegGet(1),
    .start = semCreate(0),
    .end = semCreate(0)
  };
  player_t player;
  Byte level_id = 0;
  level_t level = levels[level_id];
  
  threadCreateWithStack(TASK(task_screen), 1, &screen);
  threadCreateWithStack(TASK(task_keyboard), 1, &keyboard);
  
  start_screen(&player, &keyboard, &screen);
  

  // GAME LOOP
  while (1) {
    const int time = gettime();

    if (
      update_delay(time, &player.remaining_delay, &player.prev, player.delay) &&
      keyboard.input
    ) {
      player_update(&player, keyboard.input, screen.buf);
      keyboard.input = 0;
      // Reached end
      if (player.y == 0) {
        level_transition(&level_id, &player, &keyboard, &screen);
        level = levels[level_id];
        continue;
      }
    }
  
    // If player collides with enemy
    if (enemy_update(time, &level, screen.buf, player.x, player.y)) {
      clearScreen(&screen);
      wait(1000);
      player.x = WIDTH/2;
      player.y = REALHEIGHT;
      level = levels[level_id];
      player_update(&player, 0, screen.buf);
      continue;
    }
    
    semSignal(screen.start);
    semWait(screen.end);
    // wait(delay);
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
  while (1) {
    int ret = getKey((char*)&params->input, 1 << 30);
    if (ret < 0) {
      printf("[keybrd] Error receiving character %d\n", &ret);
    }
  }
}

void start_screen(player_t* player, keyboard_params* keyboard, screen_params* screen) {
  *player = (player_t){
    .sprite = 2,
    .x = WIDTH/2,
    .y = REALHEIGHT,
    .fg = FG_LIGHT_BLUE,
    .bg = BG_BLACK,
    .delay = STANDARD_DELAY,
    .remaining_delay = STANDARD_DELAY
  };

  clrscr(NULL);
  print_centered("Press any key to start!", 0);
  while (!keyboard->input) continue;
  clearScreen(screen);
}

void level_transition(Byte* level_id, player_t* player, keyboard_params* keyboard, screen_params* screen) {
  clrscr(NULL);
  print_centered("Level completed!", 0);
  wait(SECOND * 2);
  *level_id += 1;
  player->x = WIDTH/2;
  player->y = REALHEIGHT;
  player->remaining_delay = player->delay;
  clearScreen(screen);
  
  if (*level_id == num_levels) {
    print_centered("You've reached the game's end", 0);
    print_centered("Congratulations!", 1);
    wait(SECOND * 5);
    start_screen(player, keyboard, screen);
    *level_id = 0;
  }
}

void player_update(player_t* player, char input, Word* screen) {
    write_xy(player->sprite, player->x, player->y, FG_BLACK, BG_BLACK, screen);
    switch (input) {
      case 'w': {
        player->y = max(player->y+UP, 0);
        break;
      }
      case 's': {
        player->y = min(player->y+DOWN, REALHEIGHT);
        break;
      }
      case 'a': {
        player->x += LEFT;
        if (player->x < 0) player->x = REALWIDTH;
        break;
      }
      case 'd': {
        player->x += RIGHT;
        if (player->x > REALWIDTH) player->x = 0;
        break;
      }
    }
    write_xy(player->sprite, player->x, player->y, player->fg, player->bg, screen);
}

// Returns 1 if the player collided with an enemy, 0 otherwise.
int enemy_update(int time, level_t* level, Word* screen, SByte px, SByte py) {
  for (int i = 0; i < level->num_enemies; i++) {
    enemy_t* enemy = &level->enemies[i];
    if (!update_delay(time, &enemy->remaining_delay, &enemy->prev, enemy->delay)) continue;
    
    write_xy(enemy->sprite, enemy->x, enemy->y, FG_BLACK, BG_BLACK, screen);
    enemy->x += enemy->direction;
    if (enemy->x > REALWIDTH) { 
      enemy->x = REALWIDTH;
      enemy->direction *= -1;
    }
    else if (enemy->x < 0) { 
      enemy->x = 0;
      enemy->direction *= -1;
    }
    if (px == enemy->x && py == enemy->y) return 1;
    write_xy(enemy->sprite, enemy->x, enemy->y, enemy->fg, enemy->bg, screen);
  }
  return 0;
}

void wait(int ticks) {
  int prev = 0;
  while (ticks > 0) {
    int time = gettime();
    if (prev == time) continue;
    prev = time;
    ticks -= 1;
  }
}

// Returns 1 if the delay finished, 0 otherwise.
int update_delay(int time, short* remaining_delay, int* prev, unsigned short delay) {
  if (*prev == time) return 0;

  *prev = time;
  *remaining_delay -= 1;
  if (*remaining_delay < 0) {
    *remaining_delay = delay;
    return 1;
  }
  return 0;
}

/*
typedef struct {
  char sprite;
  SByte x;
  SByte y;
  SByte direction;
  colors_fg fg;
  colors_bg bg;
  const unsigned short delay;
  unsigned short remaining_delay;
} enemy_t;
*/
#define LEVEL(_enemies) (level_t){ .enemies = _enemies, .num_enemies = sizeof(_enemies)/sizeof(enemy_t) }
#define enemy(_sprite, _x, _y, _direction, _fg, _bg, _delay) (enemy_t) { .sprite = _sprite, .x = _x, .y = _y, .direction = _direction, .fg = _fg, .bg = _bg, .delay = _delay, .remaining_delay = _delay }
#define card(x, y, direction) enemy(8, x, y, direction, FG_RED, BG_BLACK, STANDARD_DELAY/2)
#define card2(x, y, direction) enemy(8, x, y, direction, FG_RED, BG_BLACK, SECOND/36)

static enemy_t level1[] = {
  card(WIDTH/2, 3, LEFT),
  card(WIDTH/2, 6, RIGHT),
  card(WIDTH/2, 9, LEFT),
  card(WIDTH/2, 12, RIGHT),
  card(WIDTH/2, 15, LEFT),
  card(WIDTH/2, 18, RIGHT),
  card(WIDTH/2, 21, LEFT),
};
static enemy_t level2[] = {
  card2(0, 3, LEFT),
  card2(WIDTH/2, 3, LEFT),
  card2(REALWIDTH, 3, LEFT),
  
  card2(0, 6, RIGHT),
  card2(WIDTH/2, 6, RIGHT),
  card2(REALWIDTH, 6, RIGHT),
  
  card2(0, 9, LEFT),
  card2(WIDTH/2, 9, LEFT),
  card2(REALWIDTH, 9, LEFT),
  
  card2(0, 12, RIGHT),
  card2(WIDTH/2, 12, RIGHT),
  card2(REALWIDTH, 12, RIGHT),
  
  card2(0, 15, LEFT),
  card2(WIDTH/2, 15, LEFT),
  card2(REALWIDTH, 15, LEFT),
  
  card2(0, 18, RIGHT),
  card2(WIDTH/2, 18, RIGHT),
  card2(REALWIDTH, 18, RIGHT),

  card2(0, 21, LEFT),
  card2(WIDTH/2, 21, LEFT),
  card2(REALWIDTH, 21, LEFT),
};

static level_t levels[] = {
  LEVEL(level1),
  LEVEL(level2),
};

static Byte num_levels = sizeof(levels)/sizeof(level_t);