#include "types.h"
#include "user.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SQUARE_COLOR 0x7FE0
#define SQUARE_SIZE 30

int
main(int argc, char *argv[])
{
  short color;
  if(argc != 2) {
    printf(2, "Usage: %s <color-name>\n", argv[0]);
    printf(2, "Available colors:\n", argv[0]);
    printf(2, "========================\n", argv[0]);
    printf(2, "red, green, blue\n", argv[0]);
    exit();
  }
  if(!strcmp(argv[1], "red")) {
    color = 0x3C00;
  } else if(!strcmp(argv[1], "green")) {
    color = 0x01E0;
  } else if(!strcmp(argv[1], "blue")) {
    color = 0x000F;
  } else {
    printf(2, "Invalid color selected.\n");
    exit();
  }

  if(!fork()) {
    volatile ushort *video_buffer = malloc(300*400*2);
    unsigned long long event;
    int x_coord = 0;
    int y_coord = 0;
    if(initwindow() != 0) {
      printf(1, "something's wrong!\n");
      exit();
    }
    if(initwindow() != -1) {
      printf(1, "something's wrong!\n");
      exit();
    }
    while(1) {
      for(int i = 0; i < 300*400; i++) {
        video_buffer[i] = color;
      }

      if(getinput(&event) != -1) {
        int type = (event >> 48) & 0xFFFF;
        if(type & 16) {
          x_coord = (event >> 32) & 0xFFFF;
          y_coord = (event >> 16) & 0xFFFF;
        }
      }
      for(int i = MIN(x_coord, 400-SQUARE_SIZE); i < MIN(x_coord+SQUARE_SIZE, 400); i++) {
        for(int j = MIN(y_coord, 300-SQUARE_SIZE); j < MIN(y_coord+SQUARE_SIZE, 300); j++) {
          video_buffer[j*400+i] = SQUARE_COLOR;
        }
      }
      if(drawwindow((void*)video_buffer) != 0) {
        printf(1, "something's wrong!\n");
      }
    }
  } else {
    wait();
  }
  exit();
}
