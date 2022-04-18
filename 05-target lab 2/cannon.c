#include "primlib.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <math.h>
#include <stdbool.h>

#define INITIAL_BULLET_DISTANCE_FROM_CANNON 170
#define MAX_BULLETS 2
#define MIN_DISTANCE_FOR_HIT 20
#define EXPLOSION_FRAMES 20
#define BULLET_VELOCITY 15
#define TARGET_VELOCITY 3
#define AVERAGE_TARGET_HEIGHT 80

struct Bullet {
  int x;
  int y;
  double fire_angle;
  double distance;
  bool visible;
};

// Sets the initial bullet-cannon distance and the bullet angle.
void shoot(struct Bullet *bullet, double angle) {
  bullet->distance = INITIAL_BULLET_DISTANCE_FROM_CANNON;
  bullet->fire_angle = angle;
}

void draw_target(int x_target, int y_target) {
  gfx_filledCircle(x_target, y_target, 10, MAGENTA);
}

void move_target(int *x_target, int *y_target) {
  const int y_amplitude = 20;
  const double vertical_displacement = y_amplitude * sin(*x_target * 0.02);
  *y_target = AVERAGE_TARGET_HEIGHT + vertical_displacement;
  *x_target += TARGET_VELOCITY;
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

void draw_scene(int x1_barrel, int y1_barrel, int x2_barrel, int y2_barrel) {
  gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLUE);
  gfx_filledCircle(gfx_screenWidth() / 2, gfx_screenHeight(), 100, YELLOW);
  gfx_filledTriangle(gfx_screenWidth() / 2, gfx_screenHeight(),
                     gfx_screenWidth() / 2 + x1_barrel,
                     gfx_screenHeight() - y1_barrel,
                     gfx_screenWidth() / 2 + x2_barrel,
                     gfx_screenHeight() - y2_barrel, YELLOW);
}

void draw_bullet(int x_bullet, int y_bullet) {
  gfx_filledCircle(x_bullet, y_bullet, 10, RED);
}

bool is_bullet_out_of_bounds(int bullet_x, int bullet_y) {
  return bullet_y <= 0 || bullet_x <= 0 || bullet_x >= gfx_screenWidth();
}

int main() {
  if (gfx_init())
    exit(3);

  struct Bullet bullets[MAX_BULLETS] = {
      {.x = 0, .y = 0, .fire_angle = 0, .distance = 0, .visible = false},
      {.x = 0, .y = 0, .fire_angle = 0, .distance = 0, .visible = false}};

  double angle = 90.0 * (M_PI / 180.0);
  const double delta_angle = 2.0 * (M_PI / 180.0);

  int x1_barrel;
  int y1_barrel;
  int x2_barrel;
  int y2_barrel;

  int x_target = 0;
  int y_target = AVERAGE_TARGET_HEIGHT;

  int explosion_frame_counter = 0;
  int x_explosion = 0;
  int y_explosion = 0;

  bool should_shoot = false;

  while (1) {
    if (gfx_pollkey() == SDLK_SPACE)
      should_shoot = true;

    x1_barrel = 150 * cos(angle - delta_angle);
    y1_barrel = 150 * sin(angle - delta_angle);
    x2_barrel = 150 * cos(angle + delta_angle);
    y2_barrel = 150 * sin(angle + delta_angle);

    draw_scene(x1_barrel, y1_barrel, x2_barrel, y2_barrel);

    draw_target(x_target, y_target);
    move_target(&x_target, &y_target);

    if (x_target > gfx_screenWidth()) {
      x_target = 0;
    }

    if (explosion_frame_counter != 0) {
      int scale = EXPLOSION_FRAMES - explosion_frame_counter;
      draw_explosion(x_explosion, y_explosion, scale);
      explosion_frame_counter -= 1;
    }

    if (gfx_isKeyDown(SDLK_RIGHT)) {
      angle -= 1.0 * (M_PI / 180.0);
    }

    if (gfx_isKeyDown(SDLK_LEFT)) {
      angle += 1.0 * (M_PI / 180.0);
    }

    if (should_shoot == true) {
      if (bullets[0].visible == false) {
        bullets[0].visible = true;
        shoot(&bullets[0], angle);
      } else if (bullets[1].visible == false) {
        bullets[1].visible = true;
        shoot(&bullets[1], angle);
      }
      should_shoot = false;
    }

    for (int i = 0; i <= MAX_BULLETS; ++i) {
      bullets[i].x = gfx_screenWidth() / 2 +
                     bullets[i].distance * cos(bullets[i].fire_angle);
      bullets[i].y =
          gfx_screenHeight() - bullets[i].distance * sin(bullets[i].fire_angle);

      if (bullets[i].visible == true) {
        draw_bullet(bullets[i].x, bullets[i].y);
        bullets[i].distance += BULLET_VELOCITY;
      }
    }

    for (int i = 0; i <= MAX_BULLETS; ++i)
      if (is_bullet_out_of_bounds(bullets[i].x, bullets[i].y)) {
        bullets[i].visible = false;
      }

    for (int i = 0; i <= MAX_BULLETS; ++i) {
      if (bullets[i].visible == true &&
          is_hit(x_target, y_target, bullets[i].x, bullets[i].y)) {

        x_explosion = x_target;
        y_explosion = y_target;

        destroy_bullet(&bullets[i].visible);
        explosion_frame_counter = EXPLOSION_FRAMES;
        destroy_target(&x_target);
      }
    }

    gfx_updateScreen();

    SDL_Delay(10);
  }
  return 0;
}
