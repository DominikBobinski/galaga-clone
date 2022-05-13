#include "primlib.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define INITIAL_BULLET_DISTANCE_FROM_CANNON 110
#define MAX_BULLETS 3
#define MIN_DISTANCE_FOR_HIT 27
#define EXPLOSION_FRAMES 20
#define BULLET_VELOCITY 15
#define TARGET_VELOCITY 3
#define AVERAGE_TARGET_HEIGHT 80
#define SCOREBOARD_COUNTER_MAX_DIGITS 3
#define MAX_TARGETS 6
#define MAX_TARGET_WAIT_TIME 15 // Sets time in seconds until target appears.
#define CANNON_SPEED 4
#define STAR_AMOUNT 60
#define CANNON_RELATIVE_Y (gfx_screenHeight() - 50)
#define MAX_LIVES 5
#define STARTING_LIVES 3

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

struct Target_bullet {
  int y;
  int x;
  bool should_shoot;
  int velocity;
  bool is_visible;
};

// Sets the initial bullet-cannon distance and the bullet position.
void shoot(struct Bullet *bullet, int cannon_position) {
  bullet->distance = INITIAL_BULLET_DISTANCE_FROM_CANNON;
  bullet->fire_position = cannon_position;
}

void draw_target(float x_target, float y_target, float scale) {
  gfx_circle(x_target, y_target, 15 * scale, GREEN);
  gfx_filledCircle(x_target, y_target, 12 * scale, GREEN);

  gfx_filledTriangle(x_target + 6 * scale, y_target - 6 * scale,
                     x_target + 10 * scale, y_target - 1 * scale,
                     x_target + 15 * scale, y_target - 10 * scale, GREEN);
  gfx_filledTriangle(x_target - 6 * scale, y_target - 6 * scale,
                     x_target - 10 * scale, y_target - 1 * scale,
                     x_target - 15 * scale, y_target - 10 * scale, GREEN);
  gfx_filledTriangle(x_target + 2 * scale, y_target - 4 * scale,
                     x_target + 6 * scale, y_target + 2 * scale,
                     x_target + 6 * scale, y_target - 16 * scale, GREEN);
  gfx_filledTriangle(x_target - 2 * scale, y_target - 4 * scale,
                     x_target - 6 * scale, y_target + 2 * scale,
                     x_target - 6 * scale, y_target - 16 * scale, GREEN);

  gfx_filledCircle(x_target, y_target - 2 * scale, 4 * scale, WHITE);
  gfx_filledCircle(x_target, y_target - 1 * scale, 2 * scale, BLACK);

  gfx_line(x_target - 5 * scale, y_target - 7 * scale, x_target - 1 * scale,
           y_target - 6 * scale, BLACK);
  gfx_line(x_target + 5 * scale, y_target - 7 * scale, x_target + 1 * scale,
           y_target - 6 * scale, BLACK);

  gfx_filledTriangle(x_target - 12 * scale, y_target, x_target - 1 * scale,
                     y_target + 13 * scale, x_target - 6 * scale,
                     y_target + 18 * scale, GREEN);
  gfx_filledTriangle(x_target + 12 * scale, y_target, x_target + 1 * scale,
                     y_target + 13 * scale, x_target + 6 * scale,
                     y_target + 18 * scale, GREEN);

  gfx_filledTriangle(x_target - 3 * scale, y_target + 12 * scale,
                     x_target + 3 * scale, y_target + 12 * scale, x_target,
                     y_target + 15 * scale, WHITE);
  gfx_filledTriangle(x_target - 6 * scale, y_target + 16 * scale,
                     x_target - 4 * scale, y_target + 16 * scale,
                     x_target - 2 * scale, y_target + 18 * scale, WHITE);
  gfx_filledTriangle(x_target + 6 * scale, y_target + 16 * scale,
                     x_target + 4 * scale, y_target + 16 * scale,
                     x_target + 2 * scale, y_target + 18 * scale, WHITE);

  gfx_filledTriangle(x_target - 6 * scale, y_target + 11 * scale,
                     x_target - 1 * scale, y_target + 13 * scale,
                     x_target - 6 * scale, y_target + 20 * scale, RED);
  gfx_filledTriangle(x_target + 6 * scale, y_target + 11 * scale,
                     x_target + 1 * scale, y_target + 13 * scale,
                     x_target + 6 * scale, y_target + 20 * scale, RED);
}

// Move target in a sinusoid path.
void move_target(float *x_target, float *y_target, float target_multiplier) {
  const int y_amplitude = 20;
  const double vertical_displacement = y_amplitude * sin(*x_target * 0.02);
  *y_target =
      (AVERAGE_TARGET_HEIGHT + vertical_displacement) * target_multiplier;
  *x_target += TARGET_VELOCITY + target_multiplier * 0.1;
}

// Removes the bullet that hit a target.
void destroy_bullet(bool *bullet) { *bullet = false; }

// Resets the hit target's position.
void destroy_target(float *x_target) { *x_target = 0; }

// Detects if a bullet came in contact with a target, returns true or false.
bool is_hit(float x_target, float y_target, int bullet_x, int bullet_y) {
  double bullet_to_enemy_distance =
      hypot((x_target - bullet_x), (y_target - bullet_y));

  return bullet_to_enemy_distance <= MIN_DISTANCE_FOR_HIT;
}

void draw_explosion(float x_target, float y_target, int scale) {
  gfx_circle(x_target, y_target, 5 * scale / 2, RED);
  gfx_circle(x_target, y_target, 3 * scale / 4, YELLOW);
}

void draw_background() {
  gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
}

void draw_cannon(int cannon_position, int lives_left) {
  /* Additional relative coordinate system is needed because
  the cannon starts it's movement from the center of the screen. */
  int relative_x = cannon_position;
  int relative_y = CANNON_RELATIVE_Y;

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
void draw_stats(int bullet_counter, int enemies_hit_counter) {
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
              SDL_itoa(bullet_counter, bullet_count_buffer, 10), WHITE);
  gfx_textout(gfx_screenWidth() - 30, gfx_screenHeight() - 25,
              SDL_itoa(enemies_hit_counter, enemies_hit_buffer, 10), WHITE);
}

// Calculates the maximum number avaible given the amount of digits.
int max_num_from_digits(int digits) {
  int max_num = 0;
  for (int i = 0; i < digits; ++i) {
    max_num = max_num * 10 + 9;
  }
  return max_num;
}

// Limits the cannons movement to the screen width.
void set_cannon_boundary(int *cannon_position) {
  if (*cannon_position <= 0) {
    *cannon_position = 0;
  }
  if (*cannon_position >= gfx_screenWidth()) {
    *cannon_position = gfx_screenWidth();
  }
}

// Chooses random coordinates for a star to be placed in later on.
void generate_star_pattern(int *star_x, float *star_y) {
  *star_x = rand() % gfx_screenWidth();
  *star_y = rand() % gfx_screenHeight();
}

// Draws a rectangular star at given coordinates.
void draw_stars(int x, float y) {
  gfx_filledRect(x - 1, y - 1, x + 1, y + 1, WHITE);
}

// Moves the created stars along the Y direction.
void move_stars(float *star_y, int *star_velocity) {
  *star_y += *star_velocity;
}

void draw_target_bullet(int x, int y) {
  gfx_filledCircle(x, y + 2, 5, CYAN);
  gfx_filledTriangle(x - 5, y, x + 5, y, x, y - 15, CYAN);
  gfx_filledCircle(x, y + 2, 3, MAGENTA);
  gfx_filledTriangle(x - 3, y, x + 3, y, x, y - 13, MAGENTA);
}

bool player_is_hit(int bullet_x, int bullet_y, int cannon_position) {
  return (hypot((cannon_position - bullet_x), (CANNON_RELATIVE_Y - bullet_y)) <=
              40 ||
          hypot((cannon_position - bullet_x - 50),
                (CANNON_RELATIVE_Y - bullet_y)) <= 25 ||
          hypot((cannon_position - bullet_x + 50),
                (CANNON_RELATIVE_Y - bullet_y)) <= 25 ||
          hypot((cannon_position - bullet_x - 70),
                (CANNON_RELATIVE_Y - bullet_y + 15)) <= 10 ||
          hypot((cannon_position - bullet_x + 70),
                (CANNON_RELATIVE_Y - bullet_y + 15)) <= 10 ||
          hypot((cannon_position - bullet_x - 80),
                (CANNON_RELATIVE_Y - bullet_y + 20)) <= 3 ||
          hypot((cannon_position - bullet_x + 80),
                (CANNON_RELATIVE_Y - bullet_y + 20)) <= 3);
}

void destroy_target_bullet(bool *bullet) { *bullet = false; }

int main() {
  if (gfx_init())
    exit(3);

  struct Bullet bullets[MAX_BULLETS];

  struct Target targets[MAX_TARGETS];

  struct Star stars[STAR_AMOUNT];

  struct Explosion explosions[MAX_TARGETS];

  struct Target_bullet target_bullets[MAX_TARGETS];

  struct Explosion target_bullets_explosions[MAX_TARGETS];

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

  // Initializes some variables of targets and explosions.
  for (int i = 0; i < MAX_TARGETS; ++i) {
    targets[i].x = 0;
    targets[i].y = AVERAGE_TARGET_HEIGHT;
    targets[i].multiplier = rand() % 5 + 1;
    targets[i].time_to_appear = rand() % MAX_TARGET_WAIT_TIME;

    target_bullets[i].y = 0;
    target_bullets[i].x = 0;
    target_bullets[i].should_shoot = false;
    target_bullets[i].velocity = 4;
    target_bullets[i].is_visible = false;

    explosions[i].x = 0;
    explosions[i].y = 0;
    explosions[i].frames_left = 0;
    explosions[i].scale = 0;

    target_bullets_explosions[i].x = 0;
    target_bullets_explosions[i].y = 0;
    target_bullets_explosions[i].frames_left = 0;
    target_bullets_explosions[i].scale = 0;
  }

  /* The default position is in the center of the screen */
  int cannon_position = gfx_screenWidth() / 2;
  ;

  bool should_shoot = false;

  int bullet_counter = 0;
  int enemies_hit_counter = 0;

  int lives_left = STARTING_LIVES;

  /* Scales that can be used for target sizes. Have to be careful as they aren't
     connectd in any way to the MIN_DISTANCE_FOR_HIT. */
  float target_scales[4] = {1, 1.5, 2, 2.5};

  int counter_control = 0;

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

    draw_background();

    // Star animation loop.
    for (int i = 0; i < STAR_AMOUNT; ++i) {
      draw_stars(stars[i].x, stars[i].y);
      move_stars(&stars[i].y, &stars[i].velocity);
      if (stars[i].y >= gfx_screenHeight()) {
        stars[i].y = 0;
      }
    }

    if (enemies_hit_counter % 10 == 0 & enemies_hit_counter != 0 &&
        counter_control == 1 && lives_left < MAX_LIVES) {
      lives_left += 1;
      counter_control = 0;
    }

    draw_stats(bullet_counter, enemies_hit_counter);
    draw_cannon(cannon_position, lives_left);

    // Fetches the time at which the current frame is created.
    time_t current_time = time(NULL);

    /* Target animation loop. In addition it controls the time at which a given
       target should appear. */
    for (int j = 0; j < MAX_TARGETS; ++j) {
      if (reference_time + targets[j].time_to_appear - current_time == 0) {
        targets[j].visible = true;
      }

      if (targets[j].visible == true) {
        draw_target(targets[j].x, targets[j].y, target_scales[1]);
        move_target(&targets[j].x, &targets[j].y, targets[j].multiplier);

        if (targets[j].x > gfx_screenWidth()) {
          targets[j].x = 0;
        }
      }
    }

    // Targets' bullets loop
    for (int j = 0; j < MAX_TARGETS; ++j) {
      if (targets[j].visible == true) {

        if (rand() % 2500 < 10 && target_bullets[j].is_visible == false) {
          target_bullets[j].should_shoot = true;
        }

        if (target_bullets[j].should_shoot == true) {
          target_bullets[j].is_visible = true;
          target_bullets[j].x = targets[j].x;
          target_bullets[j].y = targets[j].y;
          target_bullets[j].should_shoot = false;
        }

        if (target_bullets[j].is_visible == true) {
          draw_target_bullet(target_bullets[j].x, target_bullets[j].y);
          target_bullets[j].y +=
              target_bullets[j].velocity; // moves targets' bullets
        }
      }
    }

    for (int j = 0; j < MAX_TARGETS; ++j) {
      if (target_bullets[j].y >= gfx_screenHeight()) {
        target_bullets[j].is_visible = false;
      }
    }

    // Explosion animation loop.
    for (int j = 0; j < MAX_TARGETS; ++j) {
      explosions[j].scale = EXPLOSION_FRAMES - explosions[j].frames_left;
      if (explosions[j].frames_left != 0) {
        draw_explosion(explosions[j].x, explosions[j].y, explosions[j].scale);
        explosions[j].frames_left -= 1;
      }

      target_bullets_explosions[j].scale =
          EXPLOSION_FRAMES - target_bullets_explosions[j].frames_left;
      if (target_bullets_explosions[j].frames_left != 0) {
        draw_explosion(target_bullets_explosions[j].x,
                       target_bullets_explosions[j].y,
                       target_bullets_explosions[j].scale);
        target_bullets_explosions[j].frames_left -= 1;
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
      /* If more bullets are needed a new 'else if' segment has to be manually
         added. */
      if (bullets[0].visible == false) {
        bullets[0].visible = true;
        shoot(&bullets[0], cannon_position);
      } else if (bullets[1].visible == false) {
        bullets[1].visible = true;
        shoot(&bullets[1], cannon_position);
      } else if (bullets[2].visible == false) {
        bullets[2].visible = true;
        shoot(&bullets[2], cannon_position);
      }

      bullet_counter += 1;
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

      // Checks if any target was hit and provides approperiate consequences.
      for (int j = 0; j < MAX_TARGETS; ++j) {
        if (bullets[i].visible == true &&
            is_hit(targets[j].x, targets[j].y, bullets[i].x, bullets[i].y)) {

          enemies_hit_counter += 1;
          counter_control = 1;

          explosions[j].x = targets[j].x;
          explosions[j].y = targets[j].y;
          explosions[j].frames_left = EXPLOSION_FRAMES;

          destroy_bullet(&bullets[i].visible);
          destroy_target(&targets[j].x);
        }
        if (target_bullets[j].is_visible == true &&
            player_is_hit(target_bullets[j].x, target_bullets[j].y,
                          cannon_position) == true) {

          lives_left -= 1;

          target_bullets_explosions[j].x = target_bullets[j].x;
          target_bullets_explosions[j].y = target_bullets[j].y;
          target_bullets_explosions[j].frames_left = EXPLOSION_FRAMES;

          destroy_target_bullet(&target_bullets[j].is_visible);
        }
      }
    }
    gfx_updateScreen();

    SDL_Delay(10);
  }
  return 0;
}
