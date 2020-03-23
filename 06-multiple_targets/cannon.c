
#include "primlib.h"
#include <math.h>
#include <stdlib.h>

struct target {
  int x;
  int y;
  int speed_x;
  int is_explosion;
  int explosion_counter; // ile razy wyswietlalismy eksplozje
  int color;
};

// Draw explosion with the center at coordinates (x,y)
void drawExplosion(int x, int y, float scaling) {
  int x_triangle = x;
  int y_triangle = y;

  gfx_filledTriangle(x_triangle - 7 * scaling, y_triangle - 7 * scaling,
                     x_triangle + 7 * scaling, y_triangle + 7 * scaling,
                     x_triangle + 22 * scaling, y_triangle - 22 * scaling, RED);
  gfx_filledTriangle(x_triangle - 7 * scaling, y_triangle - 7 * scaling,
                     x_triangle + 7 * scaling, y_triangle + 7 * scaling,
                     x_triangle - 22 * scaling, y_triangle + 22 * scaling, RED);
  gfx_filledTriangle(x_triangle - 7 * scaling, y_triangle + 7 * scaling,
                     x_triangle + 7 * scaling, y_triangle - 7 * scaling,
                     x_triangle - 22 * scaling, y_triangle - 22 * scaling, RED);
  gfx_filledTriangle(x_triangle - 7 * scaling, y_triangle + 7 * scaling,
                     x_triangle + 7 * scaling, y_triangle - 7 * scaling,
                     x_triangle + 22 * scaling, y_triangle + 22 * scaling, RED);

  gfx_filledTriangle(x_triangle - 13 * scaling, y_triangle,
                     x_triangle + 13 * scaling, y_triangle, x_triangle,
                     y_triangle - 35 * scaling, RED);
  gfx_filledTriangle(x_triangle - 13 * scaling, y_triangle,
                     x_triangle + 13 * scaling, y_triangle, x_triangle,
                     y_triangle + 35 * scaling, RED);
  gfx_filledTriangle(x_triangle, y_triangle - 13 * scaling, x_triangle,
                     y_triangle + 13 * scaling, x_triangle - 35 * scaling,
                     y_triangle, RED);
  gfx_filledTriangle(x_triangle, y_triangle - 13 * scaling, x_triangle,
                     y_triangle + 13 * scaling, x_triangle + 35 * scaling,
                     y_triangle, RED);
}


void draw_target(struct target *t) {
  if (!t->is_explosion)
      gfx_filledRect(t->x - 10, t->y - 10, t->x + 10, t->y + 10,
                     t->color); // cel

  if (t->is_explosion) {
    drawExplosion(t->x, t->y + t->explosion_counter * 10,
                  t->explosion_counter / 5.0);
  }

}


void move_target(struct target *t) {
  if(!t->is_explosion)
  {
    t->x += t->speed_x;

    if (t->x > gfx_screenWidth())
      t->x = 0;

    if (t->x < 0)
      t->x = gfx_screenWidth() - 1;
  }

  if (t->is_explosion) {
    t->explosion_counter += 1;
  }

  if (t->is_explosion && (t->explosion_counter == 10)) {
    t->is_explosion = 0;
    t->x = 0;
  }
}

int main() {
  if (gfx_init())
    exit(3);

  double angle = 90.0 * (M_PI / 180.0);

  double bullet_distance;
  int is_shooting = 0;
  double fire_angle;

  const int num_targets = 3;

  struct target t[num_targets];

  t[0].x = 0;
  t[0].y = 40;
  t[0].speed_x = 3;
  t[0].is_explosion = 0;
  t[0].color = MAGENTA;

  t[1].x = 1000;
  t[1].y = 80;
  t[1].speed_x = -5;
  t[1].is_explosion = 0;
  t[1].color = WHITE;

  t[2].x = 100;
  t[2].y = 120;
  t[2].speed_x = 7;
  t[2].is_explosion = 0;
  t[2].color = GREEN;

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

    for (int i = 0; i < num_targets; ++i)
      draw_target(&(t[i]));


    int x_bullet = bullet_distance * cos(fire_angle);
    int y_bullet = bullet_distance * sin(fire_angle);
    int x_bullet_pot = gfx_screenWidth() / 2 + x_bullet;
    int y_bullet_pot = gfx_screenHeight() - y_bullet;

    if (is_shooting) {
      //      if(!is_explosion)
      gfx_filledCircle(gfx_screenWidth() / 2 + x_bullet,
                       gfx_screenHeight() - y_bullet, 10, RED); // pocisk


    }

    gfx_updateScreen();

    if (is_shooting)
    {
      for (int i = 0; i < num_targets; ++i) {
        if (hypot(x_bullet_pot - t[i].x, y_bullet_pot - t[i].y) < 50) {
          t[i].is_explosion = 1;
          t[i].explosion_counter = 0;
        }
      }
      bullet_distance += 10.0;
    }

    for (int i = 0; i < num_targets; ++i)
        move_target(&(t[i]));

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
