#include "types.h"
#include "user.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define COORD_TO_LINEAR(r, c, w) ((r)*(w)+(c))

int
main(int argc, char *argv[])
{
  int mod1 = 0;
  int mod2 = 0;
  if(argc != 2) {
    printf(2, "Usage: %s <gradient-number>\n", argv[0]);
    printf(2, "Available gradients:\n", argv[0]);
    printf(2, "========================\n", argv[0]);
    printf(2, "1: red and blue\n", argv[0]);
    printf(2, "2: red and green\n", argv[0]);
    printf(2, "3: green and blue\n", argv[0]);
    exit();
  }
  if(!strcmp(argv[1], "1")) {
    mod1 = 10;
    mod2 = 0;
  } else if(!strcmp(argv[1], "2")) {
    mod1 = 5;
    mod2 = 0;
  } else if(!strcmp(argv[1], "3")) {
    mod1 = 10;
    mod2 = 5;
  } else {
    printf(2, "Invalid color selected.\n");
    exit();
  }

  if(!fork()) {
    volatile ushort *real_video_buffer = malloc(384*400*2);
    volatile ushort *video_buffer = real_video_buffer;
    int count = 0;
    int row = 0;
    for(int iter = 0; iter < 6; iter++) {
      for(int r = row; r < (row+32); r++) {
        int col1 = 31 - (r % 32);
        int col2 = 31 - col1;
        int realcol = (col2 << mod1) + (col1 << mod2);
        for(int c = 0; c < 400; c++) {
          real_video_buffer[COORD_TO_LINEAR(r, c, 400)] = realcol;
        }
      }
      row += 32;
      for(int r = row; r < (row+32); r++) {
        int col1 = 31 - (r % 32);
        int col2 = 31 - col1;
        int realcol = (col1 << mod1) + (col2 << mod2);
        for(int c = 0; c < 400; c++) {
          real_video_buffer[COORD_TO_LINEAR(r, c, 400)] = realcol;
        }
      }
      row += 32;
    }
    if(initwindow() != 0) {
      printf(1, "something's wrong!\n");
      exit();
    }
    while(1) {
      video_buffer = real_video_buffer + (400 * count);
      if(drawwindow((void*)video_buffer) != 0) {
        printf(1, "something's wrong!\n");
      }
      sleep(1);
      count = (count + 1) % 64;
    }
  } else {
    exit();
  }
}
