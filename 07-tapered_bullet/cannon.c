#include "primlib.h"
#include <math.h>
#include <stdlib.h>

struct target {
  int x;
  int y;
  int speed_x;
  int is_explosion;
  int explosion_counter; // how many times we have already displayed an
                         // explosion animation
  int color;
};

// Draw explosion with the center at coordinates (x,y)
void drawExplosion(int x, int y, float scaling) {

  gfx_filledTriangle(x - 7 * scaling, y - 7 * scaling, x + 7 * scaling,
                     y + 7 * scaling, x + 22 * scaling, y - 22 * scaling, RED);
  gfx_filledTriangle(x - 7 * scaling, y - 7 * scaling, x + 7 * scaling,
                     y + 7 * scaling, x - 22 * scaling, y + 22 * scaling, RED);
  gfx_filledTriangle(x - 7 * scaling, y + 7 * scaling, x + 7 * scaling,
                     y - 7 * scaling, x - 22 * scaling, y - 22 * scaling, RED);
  gfx_filledTriangle(x - 7 * scaling, y + 7 * scaling, x + 7 * scaling,
                     y - 7 * scaling, x + 22 * scaling, y + 22 * scaling, RED);

  gfx_filledTriangle(x - 13 * scaling, y, x + 13 * scaling, y, x,
                     y - 35 * scaling, RED);
  gfx_filledTriangle(x - 13 * scaling, y, x + 13 * scaling, y, x,
                     y + 35 * scaling, RED);
  gfx_filledTriangle(x, y - 13 * scaling, x, y + 13 * scaling, x - 35 * scaling,
                     y, RED);
  gfx_filledTriangle(x, y - 13 * scaling, x, y + 13 * scaling, x + 35 * scaling,
                     y, RED);
}

void draw_target(struct target *t) {
  if (!t->is_explosion)
    gfx_filledRect(t->x - 10, t->y - 10, t->x + 10, t->y + 10,
                   t->color); // target

  if (t->is_explosion) {
    drawExplosion(t->x, t->y + t->explosion_counter * 10,
                  t->explosion_counter / 5.0);
  }
}

void move_target(struct target *t) {
  if (!t->is_explosion) {
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

// draw isosceles trapezoid with the center of larger base at (x1,y1), center of smaller base at (x2,y2)
// width - width of larger base, taper - ratio of smaller base to larger base
void draw_bullet(int x1, int y1, int x2, int y2, int width, double taper, int color) {
  // define vector representing the central line
  int dx = x2 - x1;
  int dy = y2 - y1;

  // define vector perpendicular to the central line
  int dx_p = dy;
  int dy_p = -dx;
  double len = hypot(dx_p, dy_p);

  // scale its length to proper width
  int dx_p_s = width / (2.0 * len) * dx_p;
  int dy_p_s = width / (2.0 * len) * dy_p;


  // scale the vector by taper
  int dx_p_s_t = dx_p_s * taper;
  int dy_p_s_t = dy_p_s * taper;

  // compute corners

  int x1c = x2 - dx_p_s_t;
  int y1c = y2 - dy_p_s_t;

  int x2c = x2 + dx_p_s_t;
  int y2c = y2 + dy_p_s_t;

  int x3c = x1 - dx_p_s;
  int y3c = y1 - dy_p_s;

  int x4c = x1 + dx_p_s;
  int y4c = y1 + dy_p_s;

  gfx_filledTriangle(x1c, y1c, x2c, y2c, x3c, y3c, color);
  gfx_filledTriangle(x4c, y4c, x2c, y2c, x3c, y3c, color);
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

    // bullet position wrt the center of the bottom of the screen
    int x_bullet_begin = bullet_distance * cos(fire_angle);
    int y_bullet_begin = bullet_distance * sin(fire_angle);

    int x_bullet_end = (bullet_distance + 30) * cos(fire_angle);
    int y_bullet_end = (bullet_distance + 30) * sin(fire_angle);

    // bullet position wrt the top-left corner
    int x_bullet_begin_tlc = gfx_screenWidth() / 2 + x_bullet_begin;
    int y_bullet_begin_tlc = gfx_screenHeight() - y_bullet_begin;

    int x_bullet_end_tlc = gfx_screenWidth() / 2 + x_bullet_end;
    int y_bullet_end_tlc = gfx_screenHeight() - y_bullet_end;

    if (is_shooting) {
      draw_bullet(x_bullet_begin_tlc, y_bullet_begin_tlc, x_bullet_end_tlc,
                  y_bullet_end_tlc, 10, 0.5, RED);
    }

    gfx_updateScreen();

    if (is_shooting) {
      for (int i = 0; i < num_targets; ++i) {
        if (hypot(x_bullet_begin_tlc - t[i].x, y_bullet_begin_tlc - t[i].y) <
            50) {
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
