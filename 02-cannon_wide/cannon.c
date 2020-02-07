#include "primlib.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (gfx_init())
    exit(3);

  double angle = 90.0 * (M_PI/180.0);
  while(1)
  {
    double delta_angle= 2.0 * (M_PI/180.0);
    int x1 = 150 * cos(angle-delta_angle);
    int y1 = 150 * sin(angle-delta_angle);
    int x2 = 150 * cos(angle+delta_angle);
    int y2 = 150 * sin(angle+delta_angle);
    gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLUE);
    gfx_filledCircle(gfx_screenWidth() / 2, gfx_screenHeight(), 100, YELLOW);
    gfx_filledTriangle(gfx_screenWidth() / 2, gfx_screenHeight() , gfx_screenWidth() / 2 + x1, gfx_screenHeight() - y1,
                       gfx_screenWidth() / 2 + x2, gfx_screenHeight() - y2, YELLOW); 
    gfx_updateScreen();
    int key=gfx_getkey();
    if (key == SDLK_RIGHT)
     angle -= 1.0 * (M_PI/180.0);
    if (key == SDLK_LEFT)
     angle += 1.0 * (M_PI/180.0);
  };
  return 0;
}
