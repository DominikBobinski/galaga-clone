#include "primlib.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (gfx_init())
    exit(3);

  double angle = 90.0 * (M_PI/180.0);
  while(1)
  {
    int x = 150 * cos(angle);
    int y = 150 * sin(angle);
    gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLUE);
    gfx_filledCircle(gfx_screenWidth() / 2, gfx_screenHeight(), 100, YELLOW);
    gfx_line(gfx_screenWidth() / 2, gfx_screenHeight() , gfx_screenWidth() / 2 + x, gfx_screenHeight() - y, YELLOW); 
    gfx_updateScreen();
    int key=gfx_getkey();
    if (key == SDLK_RIGHT)
     angle -= 1.0 * (M_PI/180.0);
    if (key == SDLK_LEFT)
     angle += 1.0 * (M_PI/180.0);
  };
  return 0;
}
