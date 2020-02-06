#include "primlib.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (gfx_init())
    exit(3);
  /* clear screen */
  gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLUE);
  gfx_filledCircle(gfx_screenWidth() / 2, gfx_screenHeight() / 4, 100, YELLOW);
  gfx_updateScreen();
  gfx_getkey();
  return 0;
}
