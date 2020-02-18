#include "primlib.h"
#include <stdlib.h>

int main() {
  if (gfx_init())
    exit(3);

  double angle = 90.0 * (M_PI/180.0);

  double bullet_distance;
  int is_shooting = 0;
  double fire_angle;

  int x_target = 0;
  int y_target = 40;
  while(1)
  {
    double delta_angle= 2.0 * (M_PI/180.0);
    int x1_barrel= 150 * cos(angle-delta_angle);
    int y1_barrel = 150 * sin(angle-delta_angle);
    int x2_barrel = 150 * cos(angle+delta_angle);
    int y2_barrel = 150 * sin(angle+delta_angle);
    gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLUE);
    gfx_filledCircle(gfx_screenWidth() / 2, gfx_screenHeight(), 100, YELLOW);
    gfx_filledTriangle(gfx_screenWidth() / 2, gfx_screenHeight() , gfx_screenWidth() / 2 + x1_barrel, gfx_screenHeight() - y1_barrel,
                       gfx_screenWidth() / 2 + x2_barrel, gfx_screenHeight() - y2_barrel, YELLOW);


    gfx_filledRect(x_target - 10, y_target - 10, x_target + 10, y_target + 10, MAGENTA);

    if(is_shooting)
    {
      int x_bullet = bullet_distance * cos(fire_angle);
      int y_bullet = bullet_distance * sin(fire_angle);
    
      gfx_filledCircle(gfx_screenWidth() / 2 + x_bullet, gfx_screenHeight() - y_bullet, 10, RED);
    }

    gfx_updateScreen();

    if(is_shooting)
      bullet_distance += 5.0;

    x_target += 3;
    if(x_target > gfx_screenWidth())
      x_target = 0;

    if(gfx_isKeyDown(SDLK_RIGHT))
     angle -= 1.0 * (M_PI/180.0);
    if(gfx_isKeyDown(SDLK_LEFT))
     angle += 1.0 * (M_PI/180.0);
    if(gfx_isKeyDown(SDLK_SPACE))
    {
     is_shooting = 1;
     bullet_distance = 170.0;
     fire_angle = angle;
    }
    SDL_Delay(10);
  };
  return 0;
}
