#include "primlib.h"
#include <math.h>
#include <stdlib.h>

void draw_target(int x_target, int y_target) {
  gfx_filledRect(x_target - 10, y_target - 10, x_target + 10, y_target + 10,
                 MAGENTA);
}

void draw_bullet(int x_bullet, int y_bullet) {
  gfx_filledCircle(gfx_screenWidth() / 2 + x_bullet,
                   gfx_screenHeight() - y_bullet, 10, RED);
}

void draw_explosion(int x_target, int y_target, int explosion_scale) {
  gfx_circle(x_target, y_target, 5 * explosion_scale / 2, RED);
  gfx_circle(x_target, y_target, 3 * explosion_scale / 4, YELLOW);
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

int main() {
  if (gfx_init())
    exit(3);

  double angle = 90.0 * (M_PI / 180.0);
  double delta_angle = 2.0 * (M_PI / 180.0);

  double bullet_distance;
  int is_shooting = 0;
  double fire_angle;

  int x_target = 0;
  int y_target = 40;

  int x_bullet;
  int y_bullet;

  int x1_barrel;
  int y1_barrel;
  int x2_barrel;
  int y2_barrel;

  while (1) {
    x1_barrel = 150 * cos(angle - delta_angle);
    y1_barrel = 150 * sin(angle - delta_angle);
    x2_barrel = 150 * cos(angle + delta_angle);
    y2_barrel = 150 * sin(angle + delta_angle);

    draw_scene(x1_barrel, y1_barrel, x2_barrel, y2_barrel);

    draw_target(x_target, y_target);

    x_target += 3;

    if (x_target > gfx_screenWidth()) {
      x_target = 0;
    }

    x_bullet = bullet_distance * cos(fire_angle);
    y_bullet = bullet_distance * sin(fire_angle);

    if (gfx_isKeyDown(SDLK_RIGHT))
      angle -= 1.0 * (M_PI / 180.0);
    if (gfx_isKeyDown(SDLK_LEFT))
      angle += 1.0 * (M_PI / 180.0);
    if (gfx_isKeyDown(SDLK_SPACE)) {
      is_shooting = 1;
      bullet_distance = 170.0;
      fire_angle = angle;
    }

    if (is_shooting) {
      draw_bullet(x_bullet, y_bullet);
      bullet_distance += 10.0;
    }

    double bullet_to_enemy_distance =
        hypot((x_target - (gfx_screenWidth() / 2 + x_bullet)),
              (y_target - (gfx_screenHeight() - y_bullet)));

    if (bullet_to_enemy_distance <= 20) {

      is_shooting = 0;

      int explosion_scale = 1;

      while (explosion_scale <= 20) {
        draw_scene(x1_barrel, y1_barrel, x2_barrel, y2_barrel);
        draw_explosion(x_target, y_target, explosion_scale);
        gfx_updateScreen();
        SDL_Delay(10);
        explosion_scale += 1;
      }

      x_target = 0;
      bullet_distance = 0;
      continue;
    }

    gfx_updateScreen();

    SDL_Delay(10);
  }
  return 0;
}
