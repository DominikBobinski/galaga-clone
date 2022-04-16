#include "primlib.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <math.h>
#include <stdbool.h>

void draw_target(int x_target, int y_target, double y_target_sin) {
  gfx_filledCircle(x_target, y_target + 20 * y_target_sin, 10, MAGENTA);
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

struct Bullet {
  int x;
  int y;
  double fire_angle;
  double distance;
  double to_enemy_distance;
};



int main() {
  if (gfx_init())
    exit(3);

  double angle = 90.0 * (M_PI / 180.0);
  const double delta_angle = 2.0 * (M_PI / 180.0);

  struct Bullet bullet1;
  bullet1.x = 0;
  bullet1.y = 0;
  bullet1.fire_angle = 0;
  bullet1.distance = 0;

  struct Bullet bullet2;
  bullet2.x = 0;
  bullet2.y = 0;
  bullet2.fire_angle = 0;
  bullet2.distance = 0;

  int x1_barrel;
  int y1_barrel;
  int x2_barrel;
  int y2_barrel;

  int x_target = 0;
  int y_target = 80;

  double y_target_sin;
  double path_angle = 0;

  int bullet_count = 0;

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
      bullet1.distance = 170.0;
      bullet2.distance = 200.0;
      if (bullet_count == 1)
        bullet1.fire_angle = angle;
      if (bullet_count == 2)
        bullet2.fire_angle = angle;
      should_shoot = false;
    }

    bullet1.x = bullet1.distance * cos(bullet1.fire_angle);
    bullet1.y = bullet1.distance * sin(bullet1.fire_angle);

    if (bullet_count == 1 || bullet_count == 2) {
      draw_bullet(bullet1.x, bullet1.y);
      bullet1.distance += 10.0;
    }

    bullet2.x = bullet2.distance * cos(bullet2.fire_angle);
    bullet2.y = bullet2.distance * sin(bullet2.fire_angle);

    if (bullet_count == 2) {
      draw_bullet(bullet2.x, bullet2.y);
      bullet2.distance += 15;
    }

    if (bullet1.distance > gfx_screenHeight()) {
      bullet1.distance = 0;
      bullet_count -= 1;
    }

    if (bullet2.distance > gfx_screenHeight()) {
      bullet2.distance = 0;
      bullet_count -= 1;
    }

    bullet1.to_enemy_distance =
        hypot((x_target - (gfx_screenWidth() / 2 + bullet1.x)),
              (y_target - (gfx_screenHeight() - bullet1.y)));

    bullet2.to_enemy_distance =
        hypot((x_target - (gfx_screenWidth() / 2 + bullet2.x)),
              (y_target - (gfx_screenHeight() - bullet2.y)));

    int scale = 1;

    if (bullet1.to_enemy_distance <= 20) {
      bullet_count -= 1;
      bullet1.distance = 0;
      while (scale <= 20) {
        draw_scene(x1_barrel, y1_barrel, x2_barrel, y2_barrel);
        draw_explosion(x_target, y_target, scale);
        gfx_updateScreen();
        SDL_Delay(10);
        scale += 1;
      }
      x_target = 0;
      continue;
    }

    if (bullet2.to_enemy_distance <= 20) {
      bullet_count -= 1;
      bullet2.distance = 0;
      while (scale <= 20) {
        draw_scene(x1_barrel, y1_barrel, x2_barrel, y2_barrel);
        draw_explosion(x_target, y_target, scale);
        gfx_updateScreen();
        SDL_Delay(10);
        scale += 1;
      }
      x_target = 0;
      continue;
    }

    printf("%d", bullet_count);
    printf("Space \n");

    gfx_updateScreen();

    SDL_Delay(10);
  }
  return 0;
}
