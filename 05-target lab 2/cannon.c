#include "primlib.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <math.h>
#include <stdbool.h>

#define INITIAL_BULLET_DISTANCE_FROM_CANNON 170
#define MAX_BULLETS 2
#define MIN_DISTANCE_FOR_HIT 20
#define EXPLOSION_FRAMES 20

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

void draw_target(int x_target, int y_target, double y_target_sin) {
  gfx_filledCircle(x_target, y_target + 20 * y_target_sin, 10, MAGENTA);
}

// Removes the bullet that hit a target.
void destroy_bullet(bool *bullet, int *bullet_count) {
  *bullet_count -= 1;
  *bullet = false;
}
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
  gfx_filledCircle(gfx_screenWidth() / 2 + x_bullet,
                   gfx_screenHeight() - y_bullet, 10, RED);
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
  int y_target = 80;

  double y_target_sin;
  double path_angle = 0;

  int bullet_count = 0;

  int explosion_frame_counter = 0;
  int x_explosion = 0;
  int y_explosion = 0;

  while (1) {
    bool should_shoot = false;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_SPACE && bullet_count < 2) {
          should_shoot = true;
        }
        break;
      case SDL_QUIT:
        exit(3);
      }
    }

    if (bullet_count < 0) {
      bullet_count = 0;
    }

    x1_barrel = 150 * cos(angle - delta_angle);
    y1_barrel = 150 * sin(angle - delta_angle);
    x2_barrel = 150 * cos(angle + delta_angle);
    y2_barrel = 150 * sin(angle + delta_angle);

    draw_scene(x1_barrel, y1_barrel, x2_barrel, y2_barrel);

    draw_target(x_target, y_target, y_target_sin);

    if (explosion_frame_counter != 0) {
      int scale = EXPLOSION_FRAMES - explosion_frame_counter;
      draw_explosion(x_explosion, y_explosion, scale);
      explosion_frame_counter -= 1;
    }

    x_target += 3;

    y_target_sin = sin(path_angle);
    path_angle += 0.1;

    if (x_target > gfx_screenWidth()) {
      x_target = 0;
      y_target = 80;
    }

    if (gfx_isKeyDown(SDLK_RIGHT)) {
      angle -= 1.0 * (M_PI / 180.0);
    }

    if (gfx_isKeyDown(SDLK_LEFT)) {
      angle += 1.0 * (M_PI / 180.0);
    }

    if (should_shoot == true) {
      bullet_count += 1;
      if (bullets[0].visible == false) {
        bullets[0].visible = true;
        shoot(&bullets[0], angle);
      } else if (bullets[1].visible == false) {
        bullets[1].visible = true;
        shoot(&bullets[1], angle);
      }
      should_shoot = false;
    }

    bullets[0].x = bullets[0].distance * cos(bullets[0].fire_angle);
    bullets[0].y = bullets[0].distance * sin(bullets[0].fire_angle);

    if (bullets[0].visible == true) {
      draw_bullet(bullets[0].x, bullets[0].y);
      bullets[0].distance += 10.0;
    }

    bullets[1].x = bullets[1].distance * cos(bullets[1].fire_angle);
    bullets[1].y = bullets[1].distance * sin(bullets[1].fire_angle);

    if (bullets[1].visible == true) {
      draw_bullet(bullets[1].x, bullets[1].y);
      bullets[1].distance += 15;
    }

    if (bullets[0].y > gfx_screenHeight() ||
        bullets[0].x > gfx_screenWidth() / 2) {
      bullets[0].visible = false;
      bullet_count -= 1;
    }

    if (bullets[1].y > gfx_screenHeight()) {
      bullets[1].visible = false;
      bullet_count -= 1;
    }

    for (int i = 0; i <= MAX_BULLETS; ++i) {
      if (bullets[i].visible == true &&
          is_hit(x_target, y_target, gfx_screenWidth() / 2 + bullets[i].x,
                 gfx_screenHeight() - bullets[i].y)) {

        x_explosion = x_target;
        y_explosion = y_target;

        destroy_bullet(&bullets[i].visible, &bullet_count);
        explosion_frame_counter = EXPLOSION_FRAMES;
        destroy_target(&x_target);
      }
    }

    printf("%d", bullet_count);
    printf("Space \n");

    gfx_updateScreen();

    SDL_Delay(10);
  }
  return 0;
}
