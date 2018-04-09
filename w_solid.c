#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  short color;
  if(argc != 2) {
    printf(2, "Usage: %s <color-name>\n", argv[0]);
    printf(2, "Available colors:\n", argv[0]);
    printf(2, "========================\n", argv[0]);
    printf(2, "red, green, blue\n", argv[0]);
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
  volatile ushort *video_buffer = malloc(300*400*2);

  if(!fork()) {
    for(int i = 0; i < 300*400; i++) {
      video_buffer[i] = color;
    }
    if(initwindow() != 0) {
      printf(1, "something's wrong!\n");
      exit();
    }
    if(initwindow() != -1) {
      printf(1, "something's wrong!\n");
      exit();
    }
    if(drawwindow((void*)video_buffer) != 0) {
	    printf(1, "something's wrong!\n");
    }
    while(1) {}
  } else {
    exit();
  }
}
