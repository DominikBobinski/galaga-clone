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
#define EXPLOSION_FRAMES 20
#define BULLET_VELOCITY 16
#define ENEMY_VELOCITY 4
#define AVERAGE_ENEMY_HEIGHT 80
#define SCOREBOARD_COUNTER_MAX_DIGITS 3
#define MAX_ENEMIES 5
#define MAX_ENEMY_WAIT_TIME 15 // Sets time in seconds until enemy appears.
#define SHIP_SPEED 5
#define STAR_AMOUNT 60
#define SHIP_RELATIVE_Y (gfx_screenHeight() - 50)
#define MAX_LIVES 5
#define STARTING_LIVES 2
#define ENEMY_BULLET_VELOCITY 5
#define HITS_TO_GAIN_LIFE 20
#define ENEMY_SHOOT_CHANCE 2000 // Higher value lowers the chance of shooting.

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
void move_enemy(float *x_enemy, float *y_enemy, float enemy_multiplier) {
  const int y_amplitude = 20;
  const double vertical_displacement = y_amplitude * sin(*x_enemy * 0.02);
  *y_enemy = (AVERAGE_ENEMY_HEIGHT + vertical_displacement) * enemy_multiplier;
  *x_enemy += ENEMY_VELOCITY + enemy_multiplier * 0.1;
}

// Removes the bullet that hit a enemy.
void destroy_bullet(bool *bullet) { *bullet = false; }

// Resets the hit enemy's position.
void destroy_enemy(float *x_enemy) { *x_enemy = 0; }

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
  return bullet_y <= 0 || bullet_x <= 0 || bullet_x >= gfx_screenWidth();
}

// Draws the score table.
void draw_stats(struct Stats stats) {
  const char bullets_shot_text[14] = "Bullets shot:";
  const char enemies_hit_text[13] = "Enemies hit:";
  char bullet_count_buffer[SCOREBOARD_COUNTER_MAX_DIGITS + 1];
  char enemies_hit_buffer[SCOREBOARD_COUNTER_MAX_DIGITS + 1];

  gfx_rect(gfx_screenWidth() - 150, gfx_screenHeight() - 57, gfx_screenWidth(),
           gfx_screenHeight(), WHITE);
  gfx_textout(gfx_screenWidth() - 140, gfx_screenHeight() - 45,
              bullets_shot_text, WHITE);
  gfx_textout(gfx_screenWidth() - 140, gfx_screenHeight() - 25,
              enemies_hit_text, WHITE);
  gfx_textout(gfx_screenWidth() - 30, gfx_screenHeight() - 45,
              SDL_itoa(stats.bullet_counter, bullet_count_buffer, 10), WHITE);
  gfx_textout(gfx_screenWidth() - 30, gfx_screenHeight() - 25,
              SDL_itoa(stats.enemies_hit_counter, enemies_hit_buffer, 10),
              WHITE);
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
  gfx_filledRect(x - 1, y - 1, x + 1, y + 1, WHITE);
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
void game_over(struct Stats stats) {
  while (1) {
    int key_pressed = gfx_pollkey();

    const char game_over[10] = "GAME OVER";
    const char press_esc[19] = "PRESS ESC TO QUIT";
    const char press_space[26] = "PRESS SPACE TO PLAY AGAIN";
    const char final_score[13] = "Final Score:";

    draw_background();

    gfx_rect(gfx_screenWidth() / 2 - 500, gfx_screenHeight() / 2 - 200,
             gfx_screenWidth() / 2 + 500, gfx_screenHeight() / 2 + 200, WHITE);
    gfx_rect(gfx_screenWidth() / 2 - 495, gfx_screenHeight() / 2 - 195,
             gfx_screenWidth() / 2 + 495, gfx_screenHeight() / 2 + 195, WHITE);

    gfx_textout(gfx_screenWidth() / 2 - 35, gfx_screenHeight() / 2 - 100,
                game_over, WHITE);
    gfx_line(gfx_screenWidth() / 2 - 45, gfx_screenHeight() / 2 - 90,
             gfx_screenWidth() / 2 + 45, gfx_screenHeight() / 2 - 90, WHITE);

    gfx_textout(gfx_screenWidth() / 2 - 45, gfx_screenHeight() / 2 - 20,
                final_score, WHITE);
    gfx_rect(gfx_screenWidth() / 2 - 80, gfx_screenHeight() / 2 - 30,
             gfx_screenWidth() / 2 + 80, gfx_screenHeight() / 2 + 40, WHITE);

    gfx_textout(gfx_screenWidth() / 2 - 100, gfx_screenHeight() / 2 + 80,
                press_space, WHITE);

    gfx_textout(gfx_screenWidth() / 2 - 68, gfx_screenHeight() / 2 + 150,
                press_esc, WHITE);

    const char bullets_shot_text[14] = "Bullets shot:";
    const char enemies_hit_text[13] = "Enemies hit:";
    char bullet_count_buffer[SCOREBOARD_COUNTER_MAX_DIGITS + 1];
    char enemies_hit_buffer[SCOREBOARD_COUNTER_MAX_DIGITS + 1];

    gfx_textout(gfx_screenWidth() / 2 - 63, gfx_screenHeight() / 2,
                bullets_shot_text, WHITE);
    gfx_textout(gfx_screenWidth() / 2 - 63, gfx_screenHeight() / 2 + 20,
                enemies_hit_text, WHITE);
    gfx_textout(gfx_screenWidth() / 2 + 45, gfx_screenHeight() / 2,
                SDL_itoa(stats.bullet_counter, bullet_count_buffer, 10), WHITE);
    gfx_textout(gfx_screenWidth() / 2 + 45, gfx_screenHeight() / 2 + 20,
                SDL_itoa(stats.enemies_hit_counter, enemies_hit_buffer, 10),
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
}

// Calculates the maximum number avaible given the amount of digits.
int max_num_from_digits(int digits) {
  int max_num = 0;
  for (int i = 0; i < digits; ++i) {
    max_num = max_num * 10 + 9;
  }
  return max_num;
}

// Sets the bullet or enemies hit counters to 0 if they exceed max digit limits.
void control_digit_amount_in_scoreboard(struct Stats *stats) {
  if (stats->bullet_counter >=
      max_num_from_digits(SCOREBOARD_COUNTER_MAX_DIGITS)) {
    stats->bullet_counter = 0;
  }

  if (stats->enemies_hit_counter >=
      max_num_from_digits(SCOREBOARD_COUNTER_MAX_DIGITS)) {
    stats->enemies_hit_counter = 0;
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

START:;

  int current_enemies = 3;

  struct Stats stats = {.bullet_counter = 0, .enemies_hit_counter = 0};

  srand(time(0));

  // Fetches the time at which the game starts.
  time_t reference_time = time(NULL);

  for (int i = 0; i < MAX_BULLETS; ++i) {
    bullets[i].x = 0;
    bullets[i].y = 0;
    bullets[i].fire_position = gfx_screenWidth() / 2;
    bullets[i].visible = false;
  }

  /* Generates random coordinates for the given STAR_AMOUNT and gives each star
  a random velocity from the range <0, 4> */
  for (int i = 0; i < STAR_AMOUNT; ++i) {
    generate_star_pattern(&stars[i].x, &stars[i].y);
    stars[i].velocity = rand() % 5;
  }

  // Initializes some variables of enemies and explosions.
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    enemies[i].x = 0;
    enemies[i].y = AVERAGE_ENEMY_HEIGHT;
    enemies[i].multiplier = rand() % 5 + 1;
    enemies[i].time_to_appear = rand() % MAX_ENEMY_WAIT_TIME;

    enemy_bullets[i].y = 0;
    enemy_bullets[i].x = 0;
    enemy_bullets[i].should_shoot = false;
    enemy_bullets[i].velocity = ENEMY_BULLET_VELOCITY;
    enemy_bullets[i].is_visible = false;

    explosions[i].x = 0;
    explosions[i].y = 0;
    explosions[i].frames_left = 0;
    explosions[i].scale = 0;

    enemy_bullets_explosions[i].x = 0;
    enemy_bullets_explosions[i].y = 0;
    enemy_bullets_explosions[i].frames_left = 0;
    enemy_bullets_explosions[i].scale = 0;
  }

  /* The default position is in the center of the screen */
  int ship_position = gfx_screenWidth() / 2;

  bool should_shoot = false;

  int lives_left = STARTING_LIVES;

  /* Scales that can be used for enemy sizes. Have to be careful as they aren't
     connectd in any way to the MIN_DISTANCE_FOR_HIT. */
  float enemy_scales[4] = {1, 1.5, 2, 2.5};

  int counter_control = 0;

  while (1) {
    if (gfx_pollkey() == SDLK_SPACE)
      should_shoot = true;

    control_digit_amount_in_scoreboard(&stats);

    if (lives_left == 0) {
      game_over(stats);
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
        lives_left < MAX_LIVES) {
      lives_left += 1;
      counter_control = 0;
    }

    draw_stats(stats);
    draw_ship(ship_position, lives_left);

    // Fetches the time at which the current frame is created.
    time_t current_time = time(NULL);

    /* enemy animation loop. In addition it controls the time at which a given
       enemy should appear. */
    for (int j = 0; j < current_enemies; ++j) {
      if (reference_time + enemies[j].time_to_appear - current_time == 0) {
        enemies[j].visible = true;
      }

      if (enemies[j].visible == true) {
        draw_enemy(enemies[j].x, enemies[j].y, enemy_scales[1]);
        move_enemy(&enemies[j].x, &enemies[j].y, enemies[j].multiplier);

        if (enemies[j].x > gfx_screenWidth()) {
          enemies[j].x = 0;
        }
      }
    }

    // enemies' bullets loop
    for (int j = 0; j < current_enemies; ++j) {
      if (rand() % ENEMY_SHOOT_CHANCE < 10 &&
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
        enemy_bullets[j].y += enemy_bullets[j].velocity;
      }
    }

    // Hides the enemies bullet if it exits the screen.
    for (int j = 0; j < current_enemies; ++j) {
      if (enemy_bullets[j].y >= gfx_screenHeight()) {
        enemy_bullets[j].is_visible = false;
      }
    }

    // Explosion animation loop.
    for (int j = 0; j < current_enemies; ++j) {
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
    for (int j = 0; j < current_enemies; ++j) {
      for (int i = 0; i < MAX_BULLETS; ++i) {
        if (bullets[i].visible == true &&
            is_hit(enemies[j].x, enemies[j].y, bullets[i].x, bullets[i].y)) {

          stats.enemies_hit_counter += 1;
          counter_control = 1;

          explosions[j].x = enemies[j].x;
          explosions[j].y = enemies[j].y;
          explosions[j].frames_left = EXPLOSION_FRAMES;

          enemies[j].visible = false;

          destroy_bullet(&bullets[i].visible);
          destroy_enemy(&enemies[j].x);
        }

        if (enemy_bullets[j].is_visible == true &&
            player_is_hit(enemy_bullets[j].x, enemy_bullets[j].y,
                          ship_position) == true) {

          lives_left -= 1;

          enemy_bullets_explosions[j].x = enemy_bullets[j].x;
          enemy_bullets_explosions[j].y = enemy_bullets[j].y;
          enemy_bullets_explosions[j].frames_left = EXPLOSION_FRAMES;

          destroy_enemy_bullet(&enemy_bullets[j].is_visible);
        }
      }
    }

    gfx_updateScreen();

    SDL_Delay(10);
  }
  return 0;
}
