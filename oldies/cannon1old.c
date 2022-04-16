#include "primlib.h"
#include <math.h>
#include <stdlib.h>

void draw_target(int x_target, int y_target, double y_target_sin) {
  gfx_filledCircle(x_target, y_target + 20 * y_target_sin, 10, MAGENTA);
}

void draw_bullet(int x_bullet, int y_bullet) {
  gfx_filledCircle(gfx_screenWidth() / 2 + x_bullet,
                   gfx_screenHeight() - y_bullet, 10, RED);
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

int main() {
  if (gfx_init())
    exit(3);

  double angle = 90.0 * (M_PI / 180.0);
  double delta_angle = 2.0 * (M_PI / 180.0);

  double bullet1_distance;
  double bullet2_distance;
  int is_shooting = 0;
  double fire_angle1;
  double fire_angle2;
  int bullet_count = 0;

  int x_bullet1;
  int y_bullet1;

  int x_bullet2;
  int y_bullet2;

  int x1_barrel;
  int y1_barrel;
  int x2_barrel;
  int y2_barrel;

  int x_target = 0;
  int y_target = 80;

  double y_target_sin;
  double path_angle = 0;

  while (1) {
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

    if (gfx_isKeyDown(SDLK_RIGHT))
      angle -= 1.0 * (M_PI / 180.0);
    if (gfx_isKeyDown(SDLK_LEFT))
      angle += 1.0 * (M_PI / 180.0);
    if (gfx_isKeyDown(SDLK_SPACE)) {
      bullet_count = 1; //this value has to increase with every bullet
      bullet1_distance = 170.0;
      fire_angle1 = angle;
    }

    if (bullet_count == 1) {
      x_bullet1 = bullet1_distance * cos(fire_angle1);
      y_bullet1 = bullet1_distance * sin(fire_angle1);
      draw_bullet(x_bullet1, y_bullet1);
      bullet1_distance += 10.0;
    }

    double bullet1_to_enemy_distance =
        hypot((x_target - (gfx_screenWidth() / 2 + x_bullet1)),
              (y_target - (gfx_screenHeight() - y_bullet1)));

    double bullet2_to_enemy_distance =
        hypot((x_target - (gfx_screenWidth() / 2 + x_bullet2)),
              (y_target - (gfx_screenHeight() - y_bullet2)));

    if (bullet1_to_enemy_distance <= 20) {
      int scale = 1;

      while (scale <= 20) {
        draw_scene(x1_barrel, y1_barrel, x2_barrel, y2_barrel);
        draw_explosion(x_target, y_target, scale);
        gfx_updateScreen();
        SDL_Delay(10);
        scale += 1;
      }

      x_target = 0;
      bullet1_distance = 0; //should happen only if bullet1 hit the target, doesnt work that way right now
      bullet2_distance = 0; //same as up
      bullet_count = 0;     //should decrease by 1 is a bullet hit the target or went off screen
      continue;
    }

    gfx_updateScreen();

    SDL_Delay(10);
  }
  return 0;
}
