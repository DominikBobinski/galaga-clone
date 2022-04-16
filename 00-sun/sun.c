#include "primlib.h"
#include <stdlib.h>
#include <math.h>

const int radius = 250;

int main() {
  float angle = 0;
  int stop = 0;

  if (gfx_init())
    exit(3);
  /* clear screen */

  int sun_x = gfx_screenWidth() / 2;
  int sun_y = gfx_screenHeight() / 2;

  while(stop == 0) {
    gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);

    gfx_filledCircle(sun_x, sun_y, 100, YELLOW);
    gfx_filledCircle(sun_x + cos(angle*(M_PI/180)) * radius, sun_y + sin(angle*(M_PI/180)) * radius, 30, BLUE);
    
    gfx_updateScreen();
  
    angle += 0.2;   

    stop = gfx_isKeyDown(SDLK_SPACE);

    SDL_Delay(1);
  }
  return 0;
}
