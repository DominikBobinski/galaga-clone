#include "primlib.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (gfx_init())
    exit(3);
  /* clear screen */
  for (int i = -99; i < 100; ++i) {
    gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
    gfx_filledCircle(gfx_screenWidth() / 2 + i, gfx_screenHeight() / 2, 100, YELLOW);
    gfx_textout(gfx_screenWidth() / 2 - i, gfx_screenHeight() / 2, "This is a text", RED);
    gfx_updateScreen();
    SDL_Delay(10);
  }
  gfx_getkey();
  return 0;
}
