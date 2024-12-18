#include "libc.h"
#include "types.h"

#define SECOND 1000 //! CHANGE THIS TO A SECOND ON YOUR MACHINE IF THE GAME IS TOO SLOW/FAST

#define nBridge 6
#define bridge_Width 5

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
SByte player_update(player_t* player, char input, Word* screen);
int enemy_update(int time, level_t* level, Word* screen, SByte px, SByte py, SByte current_move);
void wait(int ticks);
int update_delay(int time, short* remaining_delay, int* prev, unsigned short delay);
void clearScreen(screen_params* screen) {
  /*for (int i = 0; i < WIDTH * HEIGHT - 1; ++i) {
    screen->buf[i] = 0;
  }*/

  // AIGUA
  for (int i = 1; i < HEIGHT/2; ++i) {
    for (int j = 0; j < WIDTH; ++j) {
      screen->buf[i*WIDTH + j] = (BG_BLUE << 4 | FG_BLACK) << 8 | 0x00;
    }
  }

  // CARRETERA  
  for (int i = HEIGHT/2 + 1; i < REALHEIGHT; ++i) {
    for (int j = 0; j < WIDTH; ++j) {
      screen->buf[i*WIDTH + j] = (BG_BLACK << 4 | FG_BLACK) << 8 | 0x00;
    }
  }

  // Voreres
  for (int i = 0; i < WIDTH; ++i) screen->buf[HEIGHT/2*WIDTH + i] = (BG_MAGENTA << 4 | FG_BLACK) << 8 | 0x00;
  for (int i = 0; i < WIDTH; ++i) screen->buf[REALHEIGHT*WIDTH + i] = (BG_MAGENTA << 4 | FG_BLACK) << 8 | 0x00;

  // Meta
  for (int i = 0; i < WIDTH; ++i) screen->buf[i] = ((i&0x01 ? BG_LIGHT_GRAY : BG_RED) << 4 | FG_BLACK) << 8 | 0x00;

  // Ponts
  int bridge_spacing = WIDTH/(nBridge + 1);
  for (int b = 1; b <= nBridge; ++b) {
    for (int r = 1; r < HEIGHT/2; ++r) {
      for (int c = 0; c < bridge_Width; ++c) {
        screen->buf[b*bridge_spacing + r*WIDTH + c] = (BG_BROWN << 4 | FG_BLACK) << 8 | 0x00;
      }
    }
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

    SByte cMove = 0;

    if (
      update_delay(time, &player.remaining_delay, &player.prev, player.delay) &&
      keyboard.input
    ) {
      cMove += player_update(&player, keyboard.input, screen.buf);
      keyboard.input = 0;
      // Reached end
      if (player.y == 0) {
        level_transition(&level_id, &player, &keyboard, &screen);
        level = levels[level_id];
        continue;
      } else if (player.bg == BG_BLUE) { // estem a l'aigua
        goto player_death;
      }
    }

    // If player collides with enemy
    if (enemy_update(time, &level, screen.buf, player.x, player.y, cMove)) {
      goto player_death;
      /*clearScreen(&screen);
      wait(1000);
      player.x = WIDTH/2;
      player.y = REALHEIGHT;
      level = levels[level_id];
      player_update(&player, 0, screen.buf);
      continue;*/
    }

    write_xy(player.sprite, player.x, player.y, player.fg, player.bg,screen.buf); // al final dibuixem l'sprite del jugador (si no de vegades no es veia)
    
    semSignal(screen.start);
    semWait(screen.end);

    continue;
    player_death:
      clearScreen(&screen);
      wait(1000);
      player.x = WIDTH/2;
      player.y = REALHEIGHT;
      player.bg = BG_MAGENTA;
      level = levels[level_id];
      player_update(&player, 0, screen.buf);
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
    .fg = FG_LIGHT_GREEN,
    .bg = BG_MAGENTA,
    .delay = STANDARD_DELAY,
    .remaining_delay = STANDARD_DELAY,
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
  player->bg = BG_MAGENTA;
  clearScreen(screen);
  
  if (*level_id == num_levels) {
    print_centered("You've reached the game's end", 3);
    print_centered("Congratulations!", 4);
    wait(SECOND * 5);
    start_screen(player, keyboard, screen);
    *level_id = 0;
  }
}

// Return the horizontal movement of the player
SByte player_update(player_t* player, char input, Word* screen) {
    write_xy(player->sprite, player->x, player->y, player->bg, player->bg, screen);
    SByte res = 0;
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
        else res = LEFT;
        break;
      }
      case 'd': {
        player->x += RIGHT;
        if (player->x > REALWIDTH) player->x = 0;
        else res = RIGHT;
        break;
      }
    }

    //drawSprite(player->sprite, player->x, player->y, player->fg, 0xF0, Word *screen)
    player->bg = screen[player->x + player->y*WIDTH]>>12;

    //write_xy(player->sprite, player->x, player->y, player->fg, player->bg, screen);
    return res;
}

// Returns 1 if the player collided with an enemy, 0 otherwise.
int enemy_update(int time, level_t* level, Word* screen, SByte px, SByte py, SByte current_move) {
  for (int i = 0; i < level->num_enemies; i++) {
    enemy_t* enemy = &level->enemies[i];
    if (!update_delay(time, &enemy->remaining_delay, &enemy->prev, enemy->delay)) continue;

    write_xy(enemy->sprite, enemy->x, enemy->y, enemy->bg, enemy->bg, screen);
    enemy->x += enemy->direction;
    int temp_direction = enemy->direction;
    if (enemy->x > REALWIDTH) { 
      enemy->x = REALWIDTH;
      enemy->direction *= -1;
    }
    else if (enemy->x < 0) { 
      enemy->x = 0;
      enemy->direction *= -1;
    }

    enemy->bg = screen[enemy->x + enemy->y*WIDTH] >> 12; // update bg

    if (py == enemy->y) {
      if (px == enemy->x) return 1; // check normal
      /*
      if (current_move != 0 && current_move != temp_direction) {
        if (current_move > 0) {
          if (px - current_move == enemy->x) return 1; // Per algun motiu no funciona el mateix if per a les dues direccions (no li agradava que SByte fos un char en comptes d'un signed char crec)
        } else if (px == enemy->x - temp_direction) return 1;
      }*/
      if (current_move + temp_direction == 0) {
        if (px == enemy->x - temp_direction) return 1;
      }
    }
    /*if (px == enemy->x && py == enemy->y) return 1;*/
    
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
#define card(x, y, direction) enemy(8, x, y, direction, FG_RED, y >= HEIGHT>>1 ? BG_BLACK : BG_BLUE, STANDARD_DELAY/2)
#define card2(x, y, direction) enemy(8, x, y, direction, FG_RED, y >= HEIGHT>>1 ? BG_BLACK : BG_BLUE, SECOND/36)

static enemy_t level1[] = {
  card(WIDTH/2, 3, LEFT),
  card(WIDTH/2, 6, RIGHT),
  card(WIDTH/2, 9, LEFT),
  card(WIDTH/2, 13, RIGHT),
  card(WIDTH/2, 15, LEFT),
  card(WIDTH/2, 18, RIGHT),
  card(WIDTH/2, 21, LEFT),
};
static enemy_t level2[] = {
  card2(WIDTH/2, 4, LEFT),
  card(WIDTH/2, 7, LEFT),
  card2(10, 10, RIGHT),

  card2(WIDTH/2, 13, LEFT),
  card(WIDTH/2, 16, LEFT),
  card2(10, 19, RIGHT),
  card2(WIDTH/2, 4, LEFT),
  card(WIDTH/2, 7, LEFT),
  card2(10, 22, RIGHT),

};
static enemy_t level3[] = {
  card2(10, 4, RIGHT),
  card(WIDTH - 9, 7, LEFT),
  card2(REALWIDTH, 10, LEFT),

  card2(WIDTH/2, 13, LEFT),
  card(WIDTH/2, 16, LEFT),
  card2(10, 19, RIGHT),
};
static enemy_t level4[] = {
  card2(WIDTH/2 - 5, 4, RIGHT),
  card2(WIDTH/2 - 4, 7, LEFT),
  card2(WIDTH/2 - 7, 10, LEFT),

  card2(WIDTH/2 + 5, 4, RIGHT),
  card2(WIDTH/2 + 7, 7, LEFT),
  card2(WIDTH/2 + 6, 10, RIGHT),

  card2(WIDTH/2 - 15, 13, RIGHT),
  card2(WIDTH/2 - 35, 16, RIGHT),
  card2(WIDTH/2 + 15, 19, LEFT),
  card2(WIDTH/2 + 35, 19, LEFT),
  
};
static enemy_t level5[] = {
  card(0, 1, LEFT),
  card(10, 1, LEFT),
  card(20, 1, LEFT),
  card(30, 1, LEFT),
  card(40, 1, LEFT),
  card(50, 1, LEFT),
  card(60, 1, LEFT),
  card(70, 1, LEFT),

  card(70, 6, LEFT),
  card(70, 7, LEFT),

  card(20, 10, RIGHT),
  card(20, 11, RIGHT),

  card2(32, 13, RIGHT),
  card2(13, 13, RIGHT),
  card2(0, 13, RIGHT),

  card(WIDTH/2 + 5, 15, LEFT),
  card(WIDTH/2 + 20, 15, LEFT),
  card(REALWIDTH - 10, 15, LEFT),
  
  card(WIDTH/2 + 10, 18, LEFT),
  card(WIDTH/2 + 25, 18, LEFT),
  card(REALWIDTH, 18, LEFT),

  card2(30, 21, RIGHT),
  card2(15, 21, RIGHT),
  card2(0, 21, RIGHT),
};

static level_t levels[] = {
  LEVEL(level1),
  LEVEL(level2),
  LEVEL(level3),
  LEVEL(level4), 
  LEVEL(level5),
};

static Byte num_levels = sizeof(levels)/sizeof(level_t);
