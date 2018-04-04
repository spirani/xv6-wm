#include "types.h"
#include "defs.h"
#include "images.h"
#include "video.h"

unsigned short *buffer;
Window *windows;
char video_initialized = 0;

void
video_init(void)
{
  // Allocate space for a video buffer
  void *base_mem;
  for(int i = 0; i < ROUND_UP_PAGE_BOUND(VIDEO_HEIGHT*VIDEO_WIDTH*2); i++) {
    base_mem = kalloc();
  }
  buffer = base_mem;

  // Allocate space for NUM_WINDOWS windows
  for(int i = 0; i < ROUND_UP_PAGE_BOUND(sizeof(Window)*NUM_WINDOWS); i++) {
    base_mem = kalloc();
  }
  windows = base_mem;
  video_initialized = 1;
}

void
video_updatescreen(void)
{
  if(!video_initialized)
    return;

  // Copy and clear buffer
  for(int i = 0; i < VIDEO_HEIGHT*VIDEO_WIDTH*2/8; i++) {
    ((volatile ull *)VIDEO_BASE)[i] = ((volatile ull *)buffer)[i];
    ((volatile ull *)buffer)[i] = 0;
  }

  // Draw cursor
  for(int r = 0; r < MIN(IMAGE_CURSOR_HEIGHT, VIDEO_HEIGHT-mouse_y); r++) {
    for(int c = 0; c < MIN(IMAGE_CURSOR_WIDTH, VIDEO_WIDTH-mouse_x); c++) {
      buffer[COORD_TO_LINEAR(mouse_y+r, mouse_x+c, VIDEO_WIDTH)] =
        IMAGE_CURSOR_DATA[COORD_TO_LINEAR(r, c, IMAGE_CURSOR_WIDTH)];
    }
  }
}
