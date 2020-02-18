#include "primlib.h"
#include <stdlib.h>

int main() {
  if (gfx_init())
    exit(3);

  double angle = 90.0 * (M_PI/180.0);

  double bullet_distance = 170.0;
  while(1)
  {
    double delta_angle= 2.0 * (M_PI/180.0);
    int x1_barrel = 150 * cos(angle-delta_angle);
    int y1_barrel = 150 * sin(angle-delta_angle);
    int x2_barrel = 150 * cos(angle+delta_angle);
    int y2_barrel = 150 * sin(angle+delta_angle);
    gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLUE);
    gfx_filledCircle(gfx_screenWidth() / 2, gfx_screenHeight(), 100, YELLOW);
    gfx_filledTriangle(gfx_screenWidth() / 2, gfx_screenHeight() , gfx_screenWidth() / 2 + x1_barrel, gfx_screenHeight() - y1_barrel,
                       gfx_screenWidth() / 2 + x2_barrel, gfx_screenHeight() - y2_barrel, YELLOW);

    int x_bullet = bullet_distance * cos(angle);
    int y_bullet = bullet_distance * sin(angle);
    
    gfx_filledCircle(gfx_screenWidth() / 2 + x_bullet, gfx_screenHeight() - y_bullet, 10, RED);
    
    gfx_updateScreen();

    bullet_distance += 1.0;

    if(gfx_isKeyDown(SDLK_RIGHT))
     angle -= 1.0 * (M_PI/180.0);
    if(gfx_isKeyDown(SDLK_LEFT))
     angle += 1.0 * (M_PI/180.0);
    SDL_Delay(100);
  };
  return 0;
}
