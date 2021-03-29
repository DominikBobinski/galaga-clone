#include "primlib.h"
#include <stdlib.h>

int main() {
  if (gfx_init())
    exit(3);

  double angle = 90.0 * (M_PI / 180.0);

  double bullet_distance;
  int is_shooting = 0;
  double fire_angle;

  struct target_data {
    int x;
    int y;
    int dir;
    enum color c;
    int speed;
    int size;
  };

  struct target_data td[3];

  td[0].x = 300;
  td[0].y = 40;
  td[0].c = MAGENTA;
  td[0].speed = 3;
  td[0].size = 8;
  td[0].dir = -1;

  td[1].x = 0;
  td[1].y = 70;
  td[1].c = YELLOW;
  td[1].speed = 2;
  td[1].size = 10;
  td[1].dir = +1;

  td[2].x = 0;
  td[2].y = 100;
  td[2].c = BLACK;
  td[2].speed = 4;
  td[2].size = 15;
  td[2].dir = +1;

  while (1) {
    double delta_angle = 2.0 * (M_PI / 180.0);
    int x1_barrel = 150 * cos(angle - delta_angle);
    int y1_barrel = 150 * sin(angle - delta_angle);
    int x2_barrel = 150 * cos(angle + delta_angle);
    int y2_barrel = 150 * sin(angle + delta_angle);
    gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLUE);
    gfx_filledCircle(gfx_screenWidth() / 2, gfx_screenHeight(), 100, YELLOW);
    gfx_filledTriangle(gfx_screenWidth() / 2, gfx_screenHeight(),
                       gfx_screenWidth() / 2 + x1_barrel,
                       gfx_screenHeight() - y1_barrel,
                       gfx_screenWidth() / 2 + x2_barrel,
                       gfx_screenHeight() - y2_barrel, YELLOW);

    { // display targets
      int i;
      for (i = 0; i < sizeof(td) / sizeof(td[0]); ++i) {
        gfx_filledRect(td[i].x - td[i].size, td[i].y - td[i].size,
                       td[i].x + td[i].size, td[i].y + td[i].size, td[i].c);
      }
    }

    if (is_shooting) {
      int x_bullet = bullet_distance * cos(fire_angle);
      int y_bullet = bullet_distance * sin(fire_angle);

      gfx_filledCircle(gfx_screenWidth() / 2 + x_bullet,
                       gfx_screenHeight() - y_bullet, 10, RED);
    }

    gfx_updateScreen();

    if (is_shooting)
      bullet_distance += 5.0;

    { // move targets
      int i;
      for (i = 0; i < sizeof(td) / sizeof(td[0]); ++i) {
        td[i].x += td[i].speed * td[i].dir;

        if (td[i].x > gfx_screenWidth()) {
          td[i].dir = -1;
        }

        if (td[i].x < 0) {
          td[i].dir = +1;
        }
      }
    }

    if (gfx_isKeyDown(SDLK_RIGHT))
      angle -= 1.0 * (M_PI / 180.0);
    if (gfx_isKeyDown(SDLK_LEFT))
      angle += 1.0 * (M_PI / 180.0);
    if (gfx_isKeyDown(SDLK_SPACE)) {
      is_shooting = 1;
      bullet_distance = 170.0;
      fire_angle = angle;
    }
    SDL_Delay(10);
  };
  return 0;
}