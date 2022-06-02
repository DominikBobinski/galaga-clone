#include "primlib.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define INITIAL_BULLET_DISTANCE_FROM_SHIP 110
#define MAX_BULLETS 3
#define MIN_DISTANCE_FOR_HIT 27
#define EXPLOSION_FRAMES 10
#define BULLET_VELOCITY 20
#define ENEMY_VELOCITY 5
#define AVERAGE_ENEMY_HEIGHT 80
#define SCOREBOARD_COUNTER_MAX_DIGITS 3
#define MAX_ENEMIES 15
#define MAX_ENEMY_WAIT_TIME 4 // Sets time in seconds until enemy appears.
#define SHIP_SPEED 18
#define STAR_AMOUNT 60
#define SHIP_RELATIVE_Y (gfx_screenHeight() - 50)
#define MAX_LIVES 5
#define STARTING_LIVES 2
#define ENEMY_BULLET_VELOCITY 7
#define HITS_TO_GAIN_LIFE 10
#define ENEMY_SHOOT_CHANCE 2000 // Higher value lowers the chance of shooting.
#define MAX_LEVEL 5
#define LEVEL_ZERO_ENEMIES 3
#define LEVEL_TRANSITION_TIME 2 // Time spent in between levels, in seconds.

struct Bullet {
  int x;
  int y;
  double fire_position;
  double distance;
  bool visible;
};

struct Enemy {
  float x;
  float y;
  float multiplier;
  bool visible;
  int time_to_appear;
  int direction;
};

struct Explosion {
  float x;
  float y;
  int frames_left;
  int scale;
};

struct Star {
  int x;
  float y;
  int velocity;
};

struct Enemy_bullet {
  int y;
  int x;
  bool should_shoot;
  int velocity;
  bool is_visible;
};

struct Stats {
  int bullet_counter;
  int enemies_hit_counter;
  int current_level;
  int lives_left;
};

struct Level {
  int max_enemies;
  int current_enemies;
  float enemy_characteristic;
};

// Sets the initial bullet-ship distance and the bullet position.
void shoot(struct Bullet *bullet, int ship_position) {
  bullet->distance = INITIAL_BULLET_DISTANCE_FROM_SHIP;
  bullet->fire_position = ship_position;
}

void draw_enemy(float x_enemy, float y_enemy, float scale) {
  gfx_circle(x_enemy, y_enemy, 15 * scale, GREEN);
  gfx_filledCircle(x_enemy, y_enemy, 12 * scale, GREEN);

  gfx_filledTriangle(x_enemy + 6 * scale, y_enemy - 6 * scale,
                     x_enemy + 10 * scale, y_enemy - 1 * scale,
                     x_enemy + 15 * scale, y_enemy - 10 * scale, GREEN);
  gfx_filledTriangle(x_enemy - 6 * scale, y_enemy - 6 * scale,
                     x_enemy - 10 * scale, y_enemy - 1 * scale,
                     x_enemy - 15 * scale, y_enemy - 10 * scale, GREEN);
  gfx_filledTriangle(x_enemy + 2 * scale, y_enemy - 4 * scale,
                     x_enemy + 6 * scale, y_enemy + 2 * scale,
                     x_enemy + 6 * scale, y_enemy - 16 * scale, GREEN);
  gfx_filledTriangle(x_enemy - 2 * scale, y_enemy - 4 * scale,
                     x_enemy - 6 * scale, y_enemy + 2 * scale,
                     x_enemy - 6 * scale, y_enemy - 16 * scale, GREEN);

  gfx_filledCircle(x_enemy, y_enemy - 2 * scale, 4 * scale, WHITE);
  gfx_filledCircle(x_enemy, y_enemy - 1 * scale, 2 * scale, BLACK);

  gfx_line(x_enemy - 5 * scale, y_enemy - 7 * scale, x_enemy - 1 * scale,
           y_enemy - 6 * scale, BLACK);
  gfx_line(x_enemy + 5 * scale, y_enemy - 7 * scale, x_enemy + 1 * scale,
           y_enemy - 6 * scale, BLACK);

  gfx_filledTriangle(x_enemy - 12 * scale, y_enemy, x_enemy - 1 * scale,
                     y_enemy + 13 * scale, x_enemy - 6 * scale,
                     y_enemy + 18 * scale, GREEN);
  gfx_filledTriangle(x_enemy + 12 * scale, y_enemy, x_enemy + 1 * scale,
                     y_enemy + 13 * scale, x_enemy + 6 * scale,
                     y_enemy + 18 * scale, GREEN);

  gfx_filledTriangle(x_enemy - 3 * scale, y_enemy + 12 * scale,
                     x_enemy + 3 * scale, y_enemy + 12 * scale, x_enemy,
                     y_enemy + 15 * scale, WHITE);
  gfx_filledTriangle(x_enemy - 6 * scale, y_enemy + 16 * scale,
                     x_enemy - 4 * scale, y_enemy + 16 * scale,
                     x_enemy - 2 * scale, y_enemy + 18 * scale, WHITE);
  gfx_filledTriangle(x_enemy + 6 * scale, y_enemy + 16 * scale,
                     x_enemy + 4 * scale, y_enemy + 16 * scale,
                     x_enemy + 2 * scale, y_enemy + 18 * scale, WHITE);

  gfx_filledTriangle(x_enemy - 6 * scale, y_enemy + 11 * scale,
                     x_enemy - 1 * scale, y_enemy + 13 * scale,
                     x_enemy - 6 * scale, y_enemy + 20 * scale, RED);
  gfx_filledTriangle(x_enemy + 6 * scale, y_enemy + 11 * scale,
                     x_enemy + 1 * scale, y_enemy + 13 * scale,
                     x_enemy + 6 * scale, y_enemy + 20 * scale, RED);
}

// Move enemy in a sinusoid path.
void move_enemy(float *x_enemy, float *y_enemy, float enemy_multiplier,
                float enemy_characteristic, int direction) {
  const int y_amplitude = 20;
  const double vertical_displacement = y_amplitude * sin(*x_enemy * 0.02);
  *y_enemy = (AVERAGE_ENEMY_HEIGHT + vertical_displacement) * enemy_multiplier;
  *x_enemy += direction *
              (ENEMY_VELOCITY + enemy_multiplier * 0.1 * enemy_characteristic);
}

// Removes the bullet that hit a enemy.
void destroy_bullet(bool *bullet) { *bullet = false; }

// Resets the hit enemy's position.
void destroy_enemy(float *x_enemy, int *direction) {
  if (rand() % 2 == 0) {
    *x_enemy = 0;
    *direction = 1;
  } else {
    *x_enemy = gfx_screenWidth();
    *direction = -1;
  }
}

// Detects if a bullet came in contact with a enemy, returns true or false.
bool is_hit(float x_enemy, float y_enemy, int bullet_x, int bullet_y) {
  double bullet_to_enemy_distance =
      hypot((x_enemy - bullet_x), (y_enemy - bullet_y));

  return bullet_to_enemy_distance <= MIN_DISTANCE_FOR_HIT;
}

void draw_explosion(float x_enemy, float y_enemy, int scale) {
  gfx_circle(x_enemy, y_enemy, 5 * scale / 2, RED);
  gfx_circle(x_enemy, y_enemy, 3 * scale / 4, YELLOW);
}

void draw_background() {
  gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
}

void draw_ship(int ship_position, int lives_left) {
  /* Additional relative coordinate system is needed because
  the ship starts it's movement from the center of the screen. */
  int relative_x = ship_position;
  int relative_y = SHIP_RELATIVE_Y;

  gfx_filledCircle(relative_x - 22, relative_y + 20, 10, YELLOW);
  gfx_circle(relative_x - 22, relative_y + 20, 12, RED);
  gfx_circle(relative_x - 22, relative_y + 20, 14, RED);
  gfx_filledCircle(relative_x + 22, relative_y + 20, 10, YELLOW);
  gfx_circle(relative_x + 22, relative_y + 20, 12, RED);
  gfx_circle(relative_x + 22, relative_y + 20, 14, RED);

  gfx_filledTriangle(relative_x - 20, relative_y - 30, relative_x + 20,
                     relative_y - 30, relative_x + 40, relative_y, BLUE);
  gfx_filledTriangle(relative_x - 40, relative_y, relative_x + 40, relative_y,
                     relative_x - 20, relative_y - 30, BLUE);

  gfx_filledTriangle(relative_x - 50, relative_y - 20, relative_x + 50,
                     relative_y - 20, relative_x + 80, relative_y + 20, BLUE);
  gfx_filledTriangle(relative_x - 80, relative_y + 20, relative_x + 80,
                     relative_y + 20, relative_x - 50, relative_y - 20, BLUE);

  gfx_filledTriangle(relative_x - 40, relative_y, relative_x + 40, relative_y,
                     relative_x, relative_y + 40, BLUE);

  gfx_filledRect(relative_x - 50, relative_y, relative_x - 45, relative_y + 25,
                 WHITE);
  gfx_filledTriangle(relative_x - 50, relative_y + 25, relative_x - 45,
                     relative_y + 25, relative_x - 47, relative_y + 30, WHITE);

  gfx_filledRect(relative_x - 30, relative_y - 10, relative_x - 25,
                 relative_y + 15, WHITE);
  gfx_filledTriangle(relative_x - 30, relative_y + 15, relative_x - 25,
                     relative_y + 15, relative_x - 27, relative_y + 20, WHITE);

  gfx_filledRect(relative_x + 50, relative_y, relative_x + 45, relative_y + 25,
                 WHITE);
  gfx_filledTriangle(relative_x + 50, relative_y + 25, relative_x + 45,
                     relative_y + 25, relative_x + 47, relative_y + 30, WHITE);

  gfx_filledRect(relative_x + 30, relative_y - 10, relative_x + 25,
                 relative_y + 15, WHITE);
  gfx_filledTriangle(relative_x + 30, relative_y + 15, relative_x + 25,
                     relative_y + 15, relative_x + 27, relative_y + 20, WHITE);

  gfx_line(relative_x - 50, relative_y - 15, relative_x - 73, relative_y + 15,
           BLACK);
  gfx_line(relative_x + 50, relative_y - 15, relative_x + 73, relative_y + 15,
           BLACK);

  gfx_line(relative_x - 44, relative_y - 18, relative_x - 44, relative_y - 10,
           WHITE);
  gfx_line(relative_x - 41, relative_y - 18, relative_x - 41, relative_y - 10,
           WHITE);
  gfx_line(relative_x - 38, relative_y - 18, relative_x - 38, relative_y - 10,
           WHITE);
  gfx_line(relative_x - 35, relative_y - 18, relative_x - 35, relative_y - 10,
           WHITE);
  gfx_line(relative_x - 46, relative_y - 18, relative_x - 33, relative_y - 10,
           WHITE);

  gfx_line(relative_x + 35, relative_y - 18, relative_x + 35, relative_y - 10,
           WHITE);
  gfx_line(relative_x + 38, relative_y - 18, relative_x + 38, relative_y - 10,
           WHITE);

  gfx_filledCircle(relative_x, relative_y, 12, WHITE);
  gfx_filledCircle(relative_x - 5, relative_y - 2, 3, BLUE);
  gfx_filledCircle(relative_x + 5, relative_y - 2, 3, BLUE);
  gfx_filledTriangle(relative_x - 3, relative_y + 2, relative_x + 3,
                     relative_y + 2, relative_x, relative_y + 5, BLUE);

  for (int i = 1; i <= lives_left; ++i) {
    gfx_filledCircle(relative_x - 24 + i * 8, relative_y - 20, 3, RED);
  }

  if (lives_left == MAX_LIVES) {
    gfx_filledCircle(relative_x - 5, relative_y - 2, 3, RED);
    gfx_filledCircle(relative_x + 5, relative_y - 2, 3, RED);
    gfx_circle(relative_x - 5, relative_y - 2, 1, MAGENTA);
    gfx_circle(relative_x + 5, relative_y - 2, 1, MAGENTA);
  }
}

void draw_bullet(int x_bullet, int y_bullet) {
  gfx_filledTriangle(x_bullet - 12, y_bullet + 16, x_bullet + 12, y_bullet + 16,
                     x_bullet, y_bullet, WHITE);

  gfx_filledCircle(x_bullet, y_bullet + 14, 10, CYAN);
  gfx_filledCircle(x_bullet, y_bullet + 14, 7, MAGENTA);
  gfx_filledCircle(x_bullet, y_bullet + 14, 4, YELLOW);
  gfx_circle(x_bullet, y_bullet + 14, 4, BLACK);

  gfx_circle(x_bullet, y_bullet + 25, 6, MAGENTA);
  gfx_circle(x_bullet, y_bullet + 27, 3, MAGENTA);
}

// Checks if the bullet left the screen, returns true or false.
bool is_bullet_out_of_bounds(int bullet_x, int bullet_y) {
  return bullet_y < 0 || bullet_x < 0 || bullet_x > gfx_screenWidth();
}

// Draws the score table.
void draw_stats(struct Stats stats) {
  const char bullets_shot_text[] = "Bullets shot:";
  const char enemies_hit_text[] = "Enemies hit:";
  char *bullet_count_buffer =
      malloc(sizeof(char) * (SCOREBOARD_COUNTER_MAX_DIGITS + 1));
  if (bullet_count_buffer == NULL) {
    free(bullet_count_buffer);
  }
  char *enemies_hit_buffer =
      malloc(sizeof(char) * (SCOREBOARD_COUNTER_MAX_DIGITS + 1));
  if (enemies_hit_buffer == NULL) {
    free(enemies_hit_buffer);
  }

  gfx_rect(gfx_screenWidth() - 146, gfx_screenHeight() - 53, gfx_screenWidth(),
           gfx_screenHeight(), WHITE);
  gfx_textout(gfx_screenWidth() - 140, gfx_screenHeight() - 46,
              bullets_shot_text, WHITE);
  gfx_textout(gfx_screenWidth() - 140, gfx_screenHeight() - 30,
              enemies_hit_text, WHITE);
  gfx_textout(gfx_screenWidth() - 30, gfx_screenHeight() - 46,
              SDL_itoa(stats.bullet_counter, bullet_count_buffer, 10), WHITE);
  gfx_textout(gfx_screenWidth() - 30, gfx_screenHeight() - 30,
              SDL_itoa(stats.enemies_hit_counter, enemies_hit_buffer, 10),
              WHITE);

  const char level[] = "Level:";
  char *current_level_buffer =
      malloc(sizeof(char) * (SCOREBOARD_COUNTER_MAX_DIGITS + 1));
  if (current_level_buffer == NULL) {
    free(current_level_buffer);
  }
  gfx_textout(gfx_screenWidth() - 140, gfx_screenHeight() - 14, level, WHITE);
  gfx_textout(gfx_screenWidth() - 30, gfx_screenHeight() - 14,
              SDL_itoa(stats.current_level + 1, current_level_buffer, 10),
              WHITE);

  free(bullet_count_buffer);
  free(enemies_hit_buffer);
  free(current_level_buffer);
}

// Limits the ships movement to the screen width.
void set_ship_boundary(int *ship_position) {
  if (*ship_position <= 0) {
    *ship_position = 0;
  }
  if (*ship_position >= gfx_screenWidth()) {
    *ship_position = gfx_screenWidth();
  }
}

// Chooses random coordinates for a star to be placed in later on.
void generate_star_pattern(int *star_x, float *star_y) {
  *star_x = rand() % gfx_screenWidth();
  *star_y = rand() % gfx_screenHeight();
}

// Draws a rectangular star at given coordinates.
void draw_star(int x, float y) {
  gfx_filledTriangle(x, y + 1, x - 1, y, x + 1, y, WHITE);
  gfx_filledTriangle(x, y - 1, x - 1, y, x + 1, y, WHITE);
}

// Moves the created stars along the Y direction.
void move_star(float *star_y, int *star_velocity) { *star_y += *star_velocity; }

void draw_enemy_bullet(int x, int y) {
  gfx_filledCircle(x, y + 2, 5, CYAN);
  gfx_filledTriangle(x - 5, y, x + 5, y, x, y - 15, CYAN);
  gfx_filledCircle(x, y + 2, 3, MAGENTA);
  gfx_filledTriangle(x - 3, y, x + 3, y, x, y - 13, MAGENTA);
}

bool player_is_hit(int bullet_x, int bullet_y, int ship_position) {
  return (
      hypot((ship_position - bullet_x), (SHIP_RELATIVE_Y - bullet_y)) <= 40 ||
      hypot((ship_position - bullet_x - 50), (SHIP_RELATIVE_Y - bullet_y)) <=
          25 ||
      hypot((ship_position - bullet_x + 50), (SHIP_RELATIVE_Y - bullet_y)) <=
          25 ||
      hypot((ship_position - bullet_x - 70),
            (SHIP_RELATIVE_Y - bullet_y + 15)) <= 10 ||
      hypot((ship_position - bullet_x + 70),
            (SHIP_RELATIVE_Y - bullet_y + 15)) <= 10 ||
      hypot((ship_position - bullet_x - 80),
            (SHIP_RELATIVE_Y - bullet_y + 20)) <= 3 ||
      hypot((ship_position - bullet_x + 80),
            (SHIP_RELATIVE_Y - bullet_y + 20)) <= 3);
}

void destroy_enemy_bullet(bool *bullet) { *bullet = false; }

// Shows the "game over" screen and allows to either exit or play again.
void game_over(struct Stats stats, bool won) {
  int frame_color = 0;

  const char game_over[] = "GAME OVER";
  const char you_win[] = "YOU WIN!";
  const char press_esc[] = "PRESS ESC TO QUIT";
  const char press_space[] = "PRESS SPACE TO PLAY AGAIN";
  const char final_score[] = "Final Score:";

  const char bullets_shot_text[] = "Bullets shot:";
  const char enemies_hit_text[] = "Enemies hit:";
  char *bullet_count_buffer =
      malloc(sizeof(char) * (SCOREBOARD_COUNTER_MAX_DIGITS + 1));
  if (bullet_count_buffer == NULL) {
    free(bullet_count_buffer);
  }
  char *enemies_hit_buffer =
      malloc(sizeof(char) * (SCOREBOARD_COUNTER_MAX_DIGITS + 1));
  if (enemies_hit_buffer == NULL) {
    free(enemies_hit_buffer);
  }
  const char level[] = "Level:";
  char *current_level_buffer =
      malloc(sizeof(char) * (SCOREBOARD_COUNTER_MAX_DIGITS + 1));
  if (current_level_buffer == NULL) {
    free(current_level_buffer);
  }

  while (1) {
    int key_pressed = gfx_pollkey();

    draw_background();

    if (won == true) {
      frame_color = GREEN;
      gfx_textout(gfx_screenWidth() / 2 - 32, gfx_screenHeight() / 2 - 100,
                  you_win, frame_color);

    } else {
      frame_color = RED;
      gfx_textout(gfx_screenWidth() / 2 - 35, gfx_screenHeight() / 2 - 100,
                  game_over, frame_color);
    }

    gfx_rect(gfx_screenWidth() / 2 - 500, gfx_screenHeight() / 2 - 200,
             gfx_screenWidth() / 2 + 500, gfx_screenHeight() / 2 + 200,
             frame_color);
    gfx_rect(gfx_screenWidth() / 2 - 495, gfx_screenHeight() / 2 - 195,
             gfx_screenWidth() / 2 + 495, gfx_screenHeight() / 2 + 195,
             frame_color);

    gfx_line(gfx_screenWidth() / 2 - 45, gfx_screenHeight() / 2 - 90,
             gfx_screenWidth() / 2 + 45, gfx_screenHeight() / 2 - 90, WHITE);

    gfx_textout(gfx_screenWidth() / 2 - 45, gfx_screenHeight() / 2 - 20,
                final_score, WHITE);
    gfx_rect(gfx_screenWidth() / 2 - 80, gfx_screenHeight() / 2 - 30,
             gfx_screenWidth() / 2 + 80, gfx_screenHeight() / 2 + 65, WHITE);

    gfx_textout(gfx_screenWidth() / 2 - 100, gfx_screenHeight() / 2 + 80,
                press_space, WHITE);

    gfx_textout(gfx_screenWidth() / 2 - 68, gfx_screenHeight() / 2 + 150,
                press_esc, WHITE);

    gfx_textout(gfx_screenWidth() / 2 - 63, gfx_screenHeight() / 2,
                bullets_shot_text, WHITE);
    gfx_textout(gfx_screenWidth() / 2 - 63, gfx_screenHeight() / 2 + 20,
                enemies_hit_text, WHITE);
    gfx_textout(gfx_screenWidth() / 2 + 45, gfx_screenHeight() / 2,
                SDL_itoa(stats.bullet_counter, bullet_count_buffer, 10), WHITE);
    gfx_textout(gfx_screenWidth() / 2 + 45, gfx_screenHeight() / 2 + 20,
                SDL_itoa(stats.enemies_hit_counter, enemies_hit_buffer, 10),
                WHITE);

    gfx_textout(gfx_screenWidth() / 2 - 63, gfx_screenHeight() / 2 + 40, level,
                WHITE);
    gfx_textout(gfx_screenWidth() / 2 + 45, gfx_screenHeight() / 2 + 40,
                SDL_itoa(stats.current_level + 1, current_level_buffer, 10),
                WHITE);

    if (key_pressed == SDLK_ESCAPE) {
      exit(3);
    }

    if (key_pressed == SDLK_SPACE) {
      break;
    }
    gfx_updateScreen();
    SDL_Delay(10);
  }
  free(bullet_count_buffer);
  free(enemies_hit_buffer);
  free(current_level_buffer);
}

// Calculates the maximum number avaible given the amount of digits.
int max_num_from_digits(int digits) {
  int max_num = 0;
  for (int i = 0; i < digits; ++i) {
    max_num = max_num * 10 + 9;
  }
  return max_num;
}

// Sets the bullet or enemies hit counters to 0 if they exceed max digit
// limits.
void control_digit_amount_in_scoreboard(struct Stats *stats) {
  if (stats->bullet_counter >=
      max_num_from_digits(SCOREBOARD_COUNTER_MAX_DIGITS)) {
    stats->bullet_counter = 0;
  }

  if (stats->enemies_hit_counter >=
      max_num_from_digits(SCOREBOARD_COUNTER_MAX_DIGITS)) {
    stats->enemies_hit_counter = 0;
  }

  if (stats->current_level >=
      max_num_from_digits(SCOREBOARD_COUNTER_MAX_DIGITS)) {
    stats->current_level = 0;
  }
}

void level_transition(struct Star *stars, struct Stats stats, int ship_position,
                      int frame_time) {
  for (int i = 0; i < STAR_AMOUNT; ++i) {
    stars[i].velocity += 15;
  }

  time_t start = time(NULL);

  while (1) {
    time_t now = time(NULL);

    draw_background();

    // Star animation loop.
    for (int i = 0; i < STAR_AMOUNT; ++i) {
      draw_star(stars[i].x, stars[i].y);
      move_star(&stars[i].y, &stars[i].velocity);
      // Set star y to 0 if it exits the screen
      if (stars[i].y >= gfx_screenHeight()) {
        stars[i].y = 0;
      }
    }

    draw_stats(stats);

    draw_ship(ship_position, stats.lives_left);

    if (start + LEVEL_TRANSITION_TIME - now != 0) {
      gfx_textout(gfx_screenWidth() / 2 - 120, 200,
                  "TRAVELLING TO THE NEXT LOCATION", GREEN);
    } else {
      break;
    }

    SDL_Delay(16.6666 - frame_time);
    gfx_updateScreen();
  }

  for (int i = 0; i < STAR_AMOUNT; ++i) {
    stars[i].velocity -= 15;
  }
}

int main() {
  if (gfx_init())
    exit(3);

  struct Bullet bullets[MAX_BULLETS];
  struct Star stars[STAR_AMOUNT];
  struct Enemy enemies[MAX_ENEMIES];
  struct Explosion explosions[MAX_ENEMIES];
  struct Enemy_bullet enemy_bullets[MAX_ENEMIES];
  struct Explosion enemy_bullets_explosions[MAX_ENEMIES];
  struct Level levels[MAX_LEVEL];

  time_t frame_time = 0;

START:;

  srand(time(0));

  // Fetches the time at which the game starts.
  time_t game_start_time = time(NULL);

  /* Initializes the amount of enemies per level and their characteristic
   modifier */
  for (int l = 0; l < MAX_LEVEL; ++l) {
    levels[l].max_enemies = LEVEL_ZERO_ENEMIES + l;
    // The amount of enemies per level is still limited by MAX_ENEMIES.
    if (levels[l].max_enemies > MAX_ENEMIES) {
      levels[l].max_enemies = MAX_ENEMIES;
    }
    levels[l].current_enemies = levels[l].max_enemies;
    levels[l].enemy_characteristic = l + 1;
  }

  struct Stats stats = {.bullet_counter = 0,
                        .enemies_hit_counter = 0,
                        .current_level = 0,
                        .lives_left = STARTING_LIVES};

  for (int i = 0; i < MAX_BULLETS; ++i) {
    bullets[i].x = 0;
    bullets[i].y = 0;
    bullets[i].fire_position = gfx_screenWidth() / 2;
    bullets[i].visible = false;
    bullets[i].distance = 0;
  }

  /* Generates random coordinates for the given STAR_AMOUNT and gives each star
  a random velocity from the range <0, 4> */
  for (int i = 0; i < STAR_AMOUNT; ++i) {
    generate_star_pattern(&stars[i].x, &stars[i].y);
    stars[i].velocity = rand() % 5;
  }

  // Initializes some variables of enemies and explosions.
  for (int j = 0; j < MAX_ENEMIES; ++j) {
    enemies[j].x = 0;
    enemies[j].y = AVERAGE_ENEMY_HEIGHT;
    enemies[j].multiplier = rand() % 5 + 1 + j * 0.1 * pow(-1, j);
    enemies[j].time_to_appear = rand() % MAX_ENEMY_WAIT_TIME;
    enemies[j].visible = false;

    if (rand() % 2 == 0) {
      enemies[j].x = 0;
      enemies[j].direction = 1;
    } else {
      enemies[j].x = gfx_screenWidth();
      enemies[j].direction = -1;
    }

    enemy_bullets[j].y = 0;
    enemy_bullets[j].x = 0;
    enemy_bullets[j].should_shoot = false;
    enemy_bullets[j].velocity = ENEMY_BULLET_VELOCITY;
    enemy_bullets[j].is_visible = false;

    explosions[j].x = 0;
    explosions[j].y = 0;
    explosions[j].frames_left = 0;
    explosions[j].scale = 0;

    enemy_bullets_explosions[j].x = 0;
    enemy_bullets_explosions[j].y = 0;
    enemy_bullets_explosions[j].frames_left = 0;
    enemy_bullets_explosions[j].scale = 0;
  }

  /* The default position is in the center of the screen */
  int ship_position = gfx_screenWidth() / 2;

  bool should_shoot = false;

  /* Scales that can be used for enemy sizes. Have to be careful as they aren't
     connectd in any way to the MIN_DISTANCE_FOR_HIT. */
  float enemy_scales[4] = {1, 1.5, 2, 2.5};

  int counter_control = 0;

  while (1) {
    time_t frame_start_time = time(NULL);

    int pressed_key = gfx_pollkey();

    if (pressed_key == SDLK_SPACE)
      should_shoot = true;

    if (pressed_key == SDLK_ESCAPE)
      exit(3);

    if (pressed_key == SDLK_r)
      goto START;

    control_digit_amount_in_scoreboard(&stats);

    // Increase level when all enemies get shot down.
    if (levels[stats.current_level].current_enemies == 0) {
      if (stats.current_level == MAX_LEVEL - 1) {
        game_over(stats, true);
        goto START;
      }

      for (int j = 0; j < levels[stats.current_level].max_enemies; ++j) {
        enemies[j].visible = false;
        explosions[j].frames_left = 0;
        destroy_enemy_bullet(&enemy_bullets[j].is_visible);
      }

      for (int i = 0; i < MAX_BULLETS; ++i) {
        destroy_bullet(&bullets[i].visible);
      }

      stats.current_level += 1;
      level_transition(stars, stats, ship_position, frame_time);
      game_start_time = time(NULL);
    }

    if (stats.lives_left == 0 || stats.current_level == MAX_LEVEL) {
      game_over(stats, false);
      goto START; /* Perhaps dumb? Goes backwards in code to where srand() is
                     ran and variables are being initialized. This is done to
                     reset all progress in the game in case the player chooses
                     to play again. */
    }

    draw_background();

    // Star animation loop.
    for (int i = 0; i < STAR_AMOUNT; ++i) {
      draw_star(stars[i].x, stars[i].y);
      move_star(&stars[i].y, &stars[i].velocity);
      // Set star y to 0 if it exits the screen
      if (stars[i].y >= gfx_screenHeight()) {
        stars[i].y = 0;
      }
    }

    // Give the player +1 life every 10 hit enemies.
    if (stats.enemies_hit_counter % HITS_TO_GAIN_LIFE == 0 &&
        stats.enemies_hit_counter != 0 && counter_control == 1 &&
        stats.lives_left < MAX_LIVES) {
      stats.lives_left += 1;
      counter_control = 0;
    }

    draw_stats(stats);
    draw_ship(ship_position, stats.lives_left);

    /* enemy animation loop. In addition it controls the time at which a given
       enemy should appear. */
    for (int j = 0; j < levels[stats.current_level].max_enemies; ++j) {
      if (game_start_time + enemies[j].time_to_appear - frame_start_time == 0) {
        enemies[j].visible = true;
      }

      if (enemies[j].visible == true) {
        draw_enemy(enemies[j].x, enemies[j].y, enemy_scales[1]);
        move_enemy(&enemies[j].x, &enemies[j].y, enemies[j].multiplier,
                   levels[stats.current_level].enemy_characteristic,
                   enemies[j].direction);

        if (enemies[j].x > gfx_screenWidth() || enemies[j].x - 1 < 0) {
          enemies[j].direction *= -1;
        }
      }
    }

    // enemies' bullets loop
    for (int j = 0; j < levels[stats.current_level].max_enemies; ++j) {
      if (rand() % ENEMY_SHOOT_CHANCE -
                  levels[stats.current_level].enemy_characteristic * 30 <
              10 &&
          enemy_bullets[j].is_visible == false && enemies[j].visible == true) {
        enemy_bullets[j].should_shoot = true;
      }

      if (enemy_bullets[j].should_shoot == true) {
        enemy_bullets[j].is_visible = true;
        enemy_bullets[j].x = enemies[j].x;
        enemy_bullets[j].y = enemies[j].y;
        enemy_bullets[j].should_shoot = false;
      }

      if (enemy_bullets[j].is_visible == true) {
        draw_enemy_bullet(enemy_bullets[j].x, enemy_bullets[j].y);
        // moves enemies' bullets
        enemy_bullets[j].y +=
            enemy_bullets[j].velocity +
            levels[stats.current_level].enemy_characteristic * 0.6;
      }
    }

    // Hides the enemies bullet if it exits the screen.
    for (int j = 0; j < levels[stats.current_level].max_enemies; ++j) {
      if (enemy_bullets[j].y >= gfx_screenHeight()) {
        enemy_bullets[j].is_visible = false;
      }
    }

    // Explosion animation loop.
    for (int j = 0; j < levels[stats.current_level].max_enemies; ++j) {
      explosions[j].scale = EXPLOSION_FRAMES - explosions[j].frames_left;
      if (explosions[j].frames_left != 0) {
        draw_explosion(explosions[j].x, explosions[j].y, explosions[j].scale);
        explosions[j].frames_left -= 1;
      }

      enemy_bullets_explosions[j].scale =
          EXPLOSION_FRAMES - enemy_bullets_explosions[j].frames_left;
      if (enemy_bullets_explosions[j].frames_left != 0) {
        draw_explosion(enemy_bullets_explosions[j].x,
                       enemy_bullets_explosions[j].y,
                       enemy_bullets_explosions[j].scale);
        enemy_bullets_explosions[j].frames_left -= 1;
      }
    }

    if (gfx_isKeyDown(SDLK_RIGHT)) {
      ship_position += SHIP_SPEED;
    }

    if (gfx_isKeyDown(SDLK_LEFT)) {
      ship_position -= SHIP_SPEED;
    }

    set_ship_boundary(&ship_position);

    if (should_shoot == true) {
      for (int i = 0; i < MAX_BULLETS; ++i) {
        if (bullets[i].visible == false) {
          bullets[i].visible = true;
          shoot(&bullets[i], ship_position);
          break;
        }
      }

      stats.bullet_counter += 1;
      should_shoot = false;
    }

    // Bullet animating loop.
    for (int i = 0; i < MAX_BULLETS; ++i) {
      bullets[i].x = bullets[i].fire_position;
      bullets[i].y = gfx_screenHeight() - bullets[i].distance;

      if (bullets[i].visible == true) {
        draw_bullet(bullets[i].x, bullets[i].y);
        bullets[i].distance += BULLET_VELOCITY;
      }

      if (is_bullet_out_of_bounds(bullets[i].x, bullets[i].y)) {
        bullets[i].visible = false;
      }
    }

    // Checks if anyone is hit and provides approperiate consequences.
    for (int j = 0; j < levels[stats.current_level].max_enemies; ++j) {
      for (int i = 0; i < MAX_BULLETS; ++i) {
        if (bullets[i].visible == true &&
            is_hit(enemies[j].x, enemies[j].y, bullets[i].x, bullets[i].y) &&
            enemies[j].visible == true) {

          stats.enemies_hit_counter += 1;
          counter_control = 1;

          explosions[j].x = enemies[j].x;
          explosions[j].y = enemies[j].y;
          explosions[j].frames_left = EXPLOSION_FRAMES;

          levels[stats.current_level].current_enemies -= 1;
          enemies[j].visible = false;

          destroy_bullet(&bullets[i].visible);
          destroy_enemy(&enemies[j].x, &enemies[j].direction);
        }

        if (enemy_bullets[j].is_visible == true &&
            player_is_hit(enemy_bullets[j].x, enemy_bullets[j].y,
                          ship_position) == true) {

          stats.lives_left -= 1;

          enemy_bullets_explosions[j].x = enemy_bullets[j].x;
          enemy_bullets_explosions[j].y = enemy_bullets[j].y;
          enemy_bullets_explosions[j].frames_left = EXPLOSION_FRAMES;

          destroy_enemy_bullet(&enemy_bullets[j].is_visible);
        }
      }
    }

    time_t frame_end_time = time(NULL);
    frame_time = frame_start_time - frame_end_time;

    gfx_updateScreen();
    SDL_Delay(16.6666 - frame_time);
  }
  return 0;
}
