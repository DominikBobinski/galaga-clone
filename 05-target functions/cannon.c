#include "primlib.h"
#include <math.h>
#include <stdlib.h>

struct target {
  int x;
  int y;
  int velocity;
  int is_explosion;
  int explosion_counter;
  int color;
};

void drawExplosion(int x, int y, float scaling) {
        gfx_circle(x, y, 5 / 2 * scaling, RED);
        gfx_circle(x, y, 3 / 4 * scaling, YELLOW);
}

int main() {
    if (gfx_init())
        exit(3);


    while(1) {
        
    }

    return 0;
}