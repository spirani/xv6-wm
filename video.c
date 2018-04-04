#include "types.h"
#include "defs.h"
#include "images.h"
#include "video.h"

static unsigned short *buffer;
static Window *windows;
static char video_initialized = 0;

static int prev_mouse_x = 1;
static int prev_mouse_y = 1;
static char prev_mouse_left_down = 0;
static char prev_mouse_right_down = 0;

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

  // Make an example window
  windows[0].x_pos = 100;
  windows[0].y_pos = 100;
  windows[0].open = 1;

  video_initialized = 1;
}

void
video_updatescreen(void)
{
  if(!video_initialized)
    return;

  // Record a fixed set of mouse data for this iteration
  int curr_mouse_x = mouse_x;
  int curr_mouse_y = mouse_y;
  int curr_mouse_left_down = mouse_left_down;
  int curr_mouse_right_down = mouse_right_down;

  // Copy and clear buffer
  for(int i = 0; i < VIDEO_HEIGHT*VIDEO_WIDTH*2/8; i++) {
    ((volatile ull *)VIDEO_BASE)[i] = ((volatile ull *)buffer)[i];
    ((volatile ull *)buffer)[i] = 0;
  }

  // Draw example window
  video_draw_window(&windows[0]);

  // Draw cursor
  for(int r = 0; r < MIN(IMAGE_CURSOR_HEIGHT, VIDEO_HEIGHT-curr_mouse_y); r++) {
    for(int c = 0; c < MIN(IMAGE_CURSOR_WIDTH, VIDEO_WIDTH-curr_mouse_x); c++) {
      buffer[COORD_TO_LINEAR(curr_mouse_y+r, curr_mouse_x+c, VIDEO_WIDTH)] =
        IMAGE_CURSOR_DATA[COORD_TO_LINEAR(r, c, IMAGE_CURSOR_WIDTH)];
    }
  }

  // See if example window can be dragged
  if(curr_mouse_left_down) {
    if((prev_mouse_x >= windows[0].x_pos) &&
       (prev_mouse_x <= (windows[0].x_pos + WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2))) &&
       (prev_mouse_y >= windows[0].y_pos) &&
       (prev_mouse_y <= (windows[0].y_pos + WINDOW_TITLEBAR_HEIGHT))) {
         if((windows[0].x_pos + curr_mouse_x - prev_mouse_x >= 0) &&
            (windows[0].x_pos + curr_mouse_x - prev_mouse_x <=
             (VIDEO_WIDTH-WINDOW_WIDTH-(WINDOW_BORDER_SIZE*2)))) {
               windows[0].x_pos += (curr_mouse_x - prev_mouse_x);
         }
         if((windows[0].y_pos + curr_mouse_y - prev_mouse_y >= 0) &&
            (windows[0].y_pos + curr_mouse_y - prev_mouse_y <=
             (VIDEO_HEIGHT-WINDOW_HEIGHT-WINDOW_BORDER_SIZE-WINDOW_TITLEBAR_HEIGHT))) {
               windows[0].y_pos += (curr_mouse_y - prev_mouse_y);
         }
    }
  }

  // Update prev mouse events
  prev_mouse_x = curr_mouse_x;
  prev_mouse_y = curr_mouse_y;
  prev_mouse_left_down = curr_mouse_left_down;
  prev_mouse_right_down = curr_mouse_right_down;
}

static void
video_draw_window(Window *w)
{
  int counter;
  int pixel_counter = 0;

  // Draw decorations
  // A window has:
  // - A titlebar (with which it can be dragged)
  // - A small 4px border on the left, right, and bottom
  //   - No top border because there's a titlebar there
  // Total height: WINDOW_HEIGHT + 4 + titlebar height
  // Total width: WINDOW_WIDTH + 4 + 4

  // Draw titlebar
  counter = COORD_TO_LINEAR(w->y_pos, w->x_pos, VIDEO_WIDTH);
  for(int r = 0; r < WINDOW_TITLEBAR_HEIGHT; r++) {
    for(int c = 0; c < WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2); c++) {
      buffer[counter] = TITLEBAR_COLOR;
      counter++;
    }
    counter = counter - (WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2)) + VIDEO_WIDTH;
  }

  // Draw side borders
  for(int r = 0; r < WINDOW_HEIGHT; r++) {
    for(int c = 0; c < WINDOW_BORDER_SIZE; c++) {
      buffer[counter] = TITLEBAR_COLOR;
      counter++;
    }
    for(int c = 0; c < WINDOW_WIDTH; c++) {
      buffer[counter] = w->pixels[pixel_counter];
      counter++;
      pixel_counter++;
    }
    for(int c = 0; c < WINDOW_BORDER_SIZE; c++) {
      buffer[counter] = TITLEBAR_COLOR;
      counter++;
    }
    counter = counter - (WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2)) + VIDEO_WIDTH;
  }

  // Draw bottom
  for(int r = 0; r < WINDOW_BORDER_SIZE; r++) {
    for(int c = 0; c < WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2); c++) {
      buffer[counter] = TITLEBAR_COLOR;
      counter++;
    }
    counter = counter - (WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2)) + VIDEO_WIDTH;
  }
}
