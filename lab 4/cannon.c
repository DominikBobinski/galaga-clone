#include "primlib.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define INITIAL_BULLET_DISTANCE_FROM_CANNON 170
#define MAX_BULLETS 2
#define MIN_DISTANCE_FOR_HIT 20
#define EXPLOSION_FRAMES 20
#define BULLET_VELOCITY 15
#define TARGET_VELOCITY 3
#define AVERAGE_TARGET_HEIGHT 80
#define SCOREBOARD_WIDTH 150
#define SCOREBOARD_HEIGHT 70
#define SCOREBOARD_COUNTER_MAX_DIGITS 3
#define MAX_TARGETS 6
#define MAX_TARGET_WAIT_TIME 15
#define CANNON_SPEED 2

struct Bullet {
  int x;
  int y;
  double fire_position;
  double distance;
  bool visible;
};

struct Target {
  float x;
  float y;
  float multiplier;
  bool visible;
  int time_to_appear;
  bool is_exploding;
  int x_boom;
  int y_boom;
};

// Sets the initial bullet-cannon distance and the bullet angle.
void shoot(struct Bullet *bullet, int cannon_position) {
  bullet->distance = INITIAL_BULLET_DISTANCE_FROM_CANNON;
  bullet->fire_position = cannon_position;
}

void draw_target(int x_target, int y_target) {
  gfx_filledCircle(x_target, y_target, 12, GREEN);
  gfx_filledCircle(x_target, y_target - 2, 4, WHITE);
  gfx_filledCircle(x_target, y_target - 1, 2, BLACK);

  gfx_line(x_target - 5, y_target - 7, x_target - 1, y_target - 6, BLACK);
  gfx_line(x_target + 5, y_target - 7, x_target + 1, y_target - 6, BLACK);

  gfx_filledTriangle(x_target - 12, y_target, x_target - 1, y_target + 13,
                     x_target - 6, y_target + 18, GREEN);
  gfx_filledTriangle(x_target + 12, y_target, x_target + 1, y_target + 13,
                     x_target + 6, y_target + 18, GREEN);

  gfx_filledTriangle(x_target - 3, y_target + 12, x_target + 3, y_target + 12,
                     x_target, y_target + 15, WHITE);
  gfx_filledTriangle(x_target - 7, y_target + 16, x_target - 4, y_target + 16,
                     x_target - 2, y_target + 18, WHITE);
  gfx_filledTriangle(x_target + 7, y_target + 16, x_target + 4, y_target + 16,
                     x_target + 2, y_target + 18, WHITE);

  gfx_filledTriangle(x_target - 6, y_target + 11, x_target - 1, y_target + 13,
                     x_target - 6, y_target + 20, RED);
  gfx_filledTriangle(x_target + 6, y_target + 11, x_target + 1, y_target + 13,
                     x_target + 6, y_target + 20, RED);
}

void move_target(float *x_target, float *y_target, float target_multiplier) {
  const int y_amplitude = 20;
  const double vertical_displacement = y_amplitude * sin(*x_target * 0.02);
  *y_target =
      (AVERAGE_TARGET_HEIGHT + vertical_displacement) * target_multiplier;
  *x_target += TARGET_VELOCITY + target_multiplier * 0.1;
}

// Removes the bullet that hit a target.
void destroy_bullet(bool *bullet) { *bullet = false; }

// Resets the target's position.
void destroy_target(int *x_target) { *x_target = 0; }

// Detects if a bullet came in contact with a target, returns true or false.
bool is_hit(int x_target, int y_target, int bullet_x, int bullet_y) {
  double bullet_to_enemy_distance =
      hypot((x_target - bullet_x), (y_target - bullet_y));

  return bullet_to_enemy_distance <= MIN_DISTANCE_FOR_HIT;
}

void draw_explosion(int x_target, int y_target, int scale) {
  gfx_circle(x_target, y_target, 5 * scale / 2, RED);
  gfx_circle(x_target, y_target, 3 * scale / 4, YELLOW);
}

void draw_bckg_and_cannon(int x1_barrel, int y1_barrel, int x2_barrel,
                          int y2_barrel, int cannon_position) {
  gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
  gfx_filledCircle(gfx_screenWidth() / 2 + cannon_position, gfx_screenHeight(),
                   100, YELLOW);
  gfx_filledTriangle(gfx_screenWidth() / 2 + cannon_position,
                     gfx_screenHeight(),
                     gfx_screenWidth() / 2 + x1_barrel + cannon_position,
                     gfx_screenHeight() - y1_barrel,
                     gfx_screenWidth() / 2 + x2_barrel + cannon_position,
                     gfx_screenHeight() - y2_barrel, YELLOW);
}

void draw_bullet(int x_bullet, int y_bullet) {
  gfx_filledCircle(x_bullet, y_bullet + 25, 8, RED);
  gfx_filledCircle(x_bullet, y_bullet + 25, 5, YELLOW);
  gfx_filledRect(x_bullet - 6, y_bullet - 5, x_bullet + 6, y_bullet + 25,
                 WHITE);
  gfx_filledTriangle(x_bullet - 10, y_bullet + 25, x_bullet + 10, y_bullet + 25,
                     x_bullet, y_bullet, WHITE);
  gfx_filledTriangle(x_bullet - 9, y_bullet - 5, x_bullet + 9, y_bullet - 5,
                     x_bullet, y_bullet - 15, WHITE);
}

bool is_bullet_out_of_bounds(int bullet_x, int bullet_y) {
  return bullet_y <= 0 || bullet_x <= 0 || bullet_x >= gfx_screenWidth();
}

void draw_score(int bullet_counter, int enemies_hit_counter) {
  char bullets_shot_text[] = "Bullets shot:";
  char enemies_hit_text[] = "Enemies hit:";
  char bullet_count_buffer[SCOREBOARD_COUNTER_MAX_DIGITS + 1];
  char enemies_hit_buffer[SCOREBOARD_COUNTER_MAX_DIGITS + 1];

  gfx_filledRect(gfx_screenWidth() - SCOREBOARD_WIDTH, 0, gfx_screenWidth(),
                 SCOREBOARD_HEIGHT, WHITE);
  gfx_textout(gfx_screenWidth() - SCOREBOARD_WIDTH + 10, 20, bullets_shot_text,
              BLACK);
  gfx_textout(gfx_screenWidth() - SCOREBOARD_WIDTH + 10, 40, enemies_hit_text,
              BLACK);
  gfx_textout(gfx_screenWidth() - SCOREBOARD_WIDTH + 120, 20,
              SDL_itoa(bullet_counter, bullet_count_buffer, 10), BLACK);
  gfx_textout(gfx_screenWidth() - SCOREBOARD_WIDTH + 120, 40,
              SDL_itoa(enemies_hit_counter, enemies_hit_buffer, 10), BLACK);
}

int max_num_from_digits(int digits) {
  int max_num = 0;
  for (int i = 0; i < digits; ++i) {
    max_num = max_num * 10 + 9;
  }
  return max_num;
}

void set_cannon_boundary(int *cannon_position) {
  if (gfx_screenWidth() / 2 + *cannon_position <= 0) {
    *cannon_position = -(gfx_screenWidth() / 2);
  }
  if (gfx_screenWidth() / 2 + *cannon_position >= gfx_screenWidth()) {
    *cannon_position = gfx_screenWidth() / 2;
  }
}

int main() {
  if (gfx_init())
    exit(3);

  struct Bullet bullets[MAX_BULLETS] = {
      {.x = 0, .y = 0, .fire_position = 0, .distance = 0, .visible = false},
      {.x = 0, .y = 0, .fire_position = 0, .distance = 0, .visible = false}};

  struct Target targets[MAX_TARGETS];

  srand(time(0));

  time_t reference_time = time(NULL);

  for (int i = 0; i < MAX_TARGETS; ++i) {
    targets[i].x = 0;
    targets[i].y = AVERAGE_TARGET_HEIGHT;
    targets[i].multiplier = rand() % 5 + 1;
    targets[i].time_to_appear = rand() % MAX_TARGET_WAIT_TIME;
    targets[i].is_exploding = false;
  }

  int cannon_position = 0;

  const int x1_barrel = 150 * cos(90.0 * (M_PI / 180.0));
  const int y1_barrel = 150 * sin(90.0 * (M_PI / 180.0));
  const int x2_barrel = 150 * cos(90.0 * (M_PI / 180.0));
  const int y2_barrel = 150 * sin(90.0 * (M_PI / 180.0));

  int explosion_frame_counter = 0;

  bool should_shoot = false;

  int bullet_counter = 0;
  int enemies_hit_counter = 0;

  while (1) {
    if (gfx_pollkey() == SDLK_SPACE)
      should_shoot = true;

    if (bullet_counter >= max_num_from_digits(SCOREBOARD_COUNTER_MAX_DIGITS)) {
      bullet_counter = 0;
    }

    if (enemies_hit_counter >=
        max_num_from_digits(SCOREBOARD_COUNTER_MAX_DIGITS)) {
      enemies_hit_counter = 0;
    }

    draw_bckg_and_cannon(x1_barrel, y1_barrel, x2_barrel, y2_barrel,
                         cannon_position);
    draw_score(bullet_counter, enemies_hit_counter);

    time_t current_time = time(NULL);

    for (int j = 0; j < MAX_TARGETS; ++j) {
      if (reference_time + targets[j].time_to_appear - current_time == 0) {
        targets[j].visible = true;
      }
      if (targets[j].visible == true) {
        draw_target(targets[j].x, targets[j].y);
        move_target(&targets[j].x, &targets[j].y, targets[j].multiplier);
      }

      if (targets[j].x > gfx_screenWidth()) {
        targets[j].x = 0;
      }
    }

    if (explosion_frame_counter != 0) {
      int scale = EXPLOSION_FRAMES - explosion_frame_counter;
      for (int j = 0; j < MAX_TARGETS; ++j) {
        if (targets[j].is_exploding == true) {
          draw_explosion(targets[j].x_boom, targets[j].y_boom, scale);
        }
      }
      explosion_frame_counter -= 1;
    } else {
      for (int j = 0; j < MAX_TARGETS; ++j) {
        targets[j].is_exploding = false;
      }
    }

    if (gfx_isKeyDown(SDLK_RIGHT)) {
      cannon_position += CANNON_SPEED;
    }

    if (gfx_isKeyDown(SDLK_LEFT)) {
      cannon_position -= CANNON_SPEED;
    }

    set_cannon_boundary(&cannon_position);

    if (should_shoot == true) {
      if (bullets[0].visible == false) {
        bullets[0].visible = true;
        shoot(&bullets[0], cannon_position);
      } else if (bullets[1].visible == false) {
        bullets[1].visible = true;
        shoot(&bullets[1], cannon_position);
      }
      bullet_counter += 1;
      should_shoot = false;
    }

    for (int i = 0; i < MAX_BULLETS; ++i) {
      bullets[i].x = gfx_screenWidth() / 2 + bullets[i].fire_position;
      bullets[i].y = gfx_screenHeight() - bullets[i].distance;

      if (bullets[i].visible == true) {
        draw_bullet(bullets[i].x, bullets[i].y);
        bullets[i].distance += BULLET_VELOCITY;
      }

      if (is_bullet_out_of_bounds(bullets[i].x, bullets[i].y)) {
        bullets[i].visible = false;
      }

      for (int j = 0; j < MAX_TARGETS; ++j) {
        if (bullets[i].visible == true &&
            is_hit(targets[j].x, targets[j].y, bullets[i].x, bullets[i].y)) {

          enemies_hit_counter += 1;

          targets[j].is_exploding = true;

          targets[j].x_boom = targets[j].x;
          targets[j].y_boom = targets[j].y;

          destroy_bullet(&bullets[i].visible);
          explosion_frame_counter = EXPLOSION_FRAMES;
          destroy_target(&targets[j].x);
        }
      }
    }
    gfx_updateScreen();

    SDL_Delay(10);
  }
  return 0;
}
