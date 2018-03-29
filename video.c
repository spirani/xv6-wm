#include "types.h"
#include "defs.h"

#define VIDEO_BASE 0xFD000000

void
video_updatescreen(void)
{
  ushort color = 0x7FFF;
  if(mouse_left_down) {
    color = 0x03B0;
  } else if (mouse_right_down) {
    color = 0x001F;
  }
  *((unsigned short *)VIDEO_BASE + (1024*mouse_y+mouse_x)) = color;
}
