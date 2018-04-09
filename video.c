#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "images.h"
#include "video.h"

// Window management data structures
static unsigned short *buffer;
static Window *windows;
static Window **window_stack;
static unsigned int windows_active = 0;
static char video_initialized = 0;
static Window *dragged_window = 0;

// Tracked mouse data
static int curr_mouse_x;
static int curr_mouse_y;
static int curr_mouse_left_down;
static int curr_mouse_right_down;
static int prev_mouse_x = 1;
static int prev_mouse_y = 1;
static char prev_mouse_left_down = 0;
static char prev_mouse_right_down = 0;

// Mutex lock for data structures
struct spinlock window_lock;

void
video_init(void)
{
  initlock(&window_lock, "window");

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

  // Allocate space for window_stack
  for(int i = 0; i < ROUND_UP_PAGE_BOUND(sizeof(Window*)*NUM_WINDOWS); i++) {
    base_mem = kalloc();
  }
  window_stack = base_mem;

  // Clear initial window status data
  for(int i = 0; i < NUM_WINDOWS; i++) {
    windows[i].x_pos = 0;
    windows[i].y_pos = 0;
    windows[i].active = 0;
    windows[i].proc = 0;
    window_stack[i] = 0;
  }

  video_initialized = 1;
}

void
video_updatescreen(void)
{
  if(!video_initialized)
    return;

  // Record a fixed set of mouse data for this iteration
  curr_mouse_x = mouse_x;
  curr_mouse_y = mouse_y;
  curr_mouse_left_down = mouse_left_down;
  curr_mouse_right_down = mouse_right_down;

  // Copy and clear buffer
  for(int i = 0; i < VIDEO_HEIGHT*VIDEO_WIDTH*2/8; i++) {
    ((volatile ull *)VIDEO_BASE)[i] = ((volatile ull *)buffer)[i];
    ((volatile ull *)buffer)[i] = 0;
  }

  // Draw windows
  for(int i = NUM_WINDOWS-1; i >= 1; i--) {
    if(window_stack[i]) {
      video_window_draw(window_stack[i], WINDOW_INACTIVE_COLOR);
    }
  }
  if(window_stack[0]) {
    video_window_draw(window_stack[0], WINDOW_ACTIVE_COLOR);
  }

  // Draw cursor
  for(int r = 0; r < MIN(IMAGE_CURSOR_HEIGHT, VIDEO_HEIGHT-curr_mouse_y); r++) {
    for(int c = 0; c < MIN(IMAGE_CURSOR_WIDTH, VIDEO_WIDTH-curr_mouse_x); c++) {
      buffer[COORD_TO_LINEAR(curr_mouse_y+r, curr_mouse_x+c, VIDEO_WIDTH)] =
        IMAGE_CURSOR_DATA[COORD_TO_LINEAR(r, c, IMAGE_CURSOR_WIDTH)];
    }
  }

  video_mouse_handle();

  // Update prev mouse events
  prev_mouse_x = curr_mouse_x;
  prev_mouse_y = curr_mouse_y;
  prev_mouse_left_down = curr_mouse_left_down;
  prev_mouse_right_down = curr_mouse_right_down;
}

static void
video_mouse_handle()
{
  if(curr_mouse_left_down && !prev_mouse_left_down) {
    // Mouse down now but not down previously?
    // Is cursor inside window?
    for(int i = 0; i < NUM_WINDOWS; i++) {
      if(window_stack[i]) {
        if((prev_mouse_x >= window_stack[i]->x_pos) &&
           (prev_mouse_x <= (window_stack[i]->x_pos +
                             WINDOW_WIDTH +
                             (WINDOW_BORDER_SIZE*2))) &&
           (prev_mouse_y >= window_stack[i]->y_pos) &&
           (prev_mouse_y <= (window_stack[i]->y_pos +
                             WINDOW_TITLEBAR_HEIGHT +
                             WINDOW_HEIGHT +
                             WINDOW_BORDER_SIZE))) {
          // Repush this window to top of stack
          Window *temp = window_stack[i];
          for(int j = i-1; j >= 0; j--) {
            window_stack[j+1] = window_stack[j];
          }
          window_stack[0] = temp;

          // Is cursor inside titlebar?
          if(prev_mouse_y <= (window_stack[0]->y_pos +
                              WINDOW_TITLEBAR_HEIGHT)) {
            if(prev_mouse_x >= (window_stack[0]->x_pos +
                                WINDOW_BORDER_SIZE +
                                WINDOW_WIDTH -
                                IMAGE_CLOSE_BUTTON_WIDTH)) {
              // Clicking on close button?
              video_window_destroy(window_stack[0]);
            } else {
              // Initiate dragging operation
              dragged_window = window_stack[0];
            }
          }
          break;
        }
      }
    }
  } else if(curr_mouse_left_down && prev_mouse_left_down) {
    // Mouse down now and down previously?
    // Currently dragging?
    if(dragged_window) {
      // If so, handle drag.
      if(((int)(dragged_window->x_pos) + curr_mouse_x - prev_mouse_x >= 0) &&
         (dragged_window->x_pos + curr_mouse_x - prev_mouse_x <=
          (VIDEO_WIDTH-WINDOW_WIDTH-(WINDOW_BORDER_SIZE*2)))) {
        dragged_window->x_pos += (curr_mouse_x - prev_mouse_x);
      }
      if(((int)(dragged_window->y_pos) + curr_mouse_y - prev_mouse_y >= 0) &&
         (dragged_window->y_pos + curr_mouse_y - prev_mouse_y <=
          (VIDEO_HEIGHT-WINDOW_HEIGHT-WINDOW_BORDER_SIZE-WINDOW_TITLEBAR_HEIGHT))) {
        dragged_window->y_pos += (curr_mouse_y - prev_mouse_y);
      }
    }
  } else if(!curr_mouse_left_down && prev_mouse_left_down) {
    // Mouse not down now but down previously?
    // Cancel any dragging operation
    dragged_window = 0;
  }
}

static Window *
video_window_create(int win_x, int win_y)
{
  // Find an unused window slot
  Window *new_window = 0;

  acquire(&window_lock);

  for(int i = 0; i < NUM_WINDOWS; i++) {
    if(windows[i].active == 0) {
      new_window = &windows[i];
      break;
    }
  }

  // Return null if all windows are used.
  if(!new_window) {
    return 0;
  }

  // Set up structures, mark newest window as topmost
  new_window->active = 1;
  new_window->x_pos = win_x;
  new_window->y_pos = win_y;
  for(int i = windows_active-1; i >= 0; i--) {
    window_stack[i+1] = window_stack[i];
  }
  window_stack[0] = new_window;
  windows_active++;

  release(&window_lock);

  return new_window;
}

int
video_window_init(struct proc *p)
{
  // Does this process already have a window?
  for(int i = 0; i < NUM_WINDOWS; i++) {
    if(windows[i].active && windows[i].proc == p) {
      return -1;
    }
  }

  // If not, give it a new window
  Window *temp = video_window_create(100, 100);
  temp->proc = p;
  return 0;
}

void
video_window_copy_window(struct proc *p, void *window_buffer)
{
  // Find this process's window
  Window *w = 0;
  for(int i = 0; i < NUM_WINDOWS; i++) {
    if(windows[i].active && windows[i].proc == p) {
      w = &windows[i];
      break;
    }
  }
  if(w == 0)
    return;

  // If found, copy window data over
  void *pixels = (void *)(w->pixels);
  for(int i = 0; i < WINDOW_HEIGHT*WINDOW_WIDTH*2/8; i++) {
    ((volatile ull *)pixels)[i] = ((volatile ull *)window_buffer)[i];
  }
}

static void
video_window_destroy(Window *w)
{
  acquire(&window_lock);

  // Mark it as free
  w->active = 0;
  windows_active--;

  // Remove from stack
  for(int i = 0; i < NUM_WINDOWS; i++) {
    if(window_stack[i] == w) {
      for(int j = i+1; j < NUM_WINDOWS; j++) {
        window_stack[j-1] = window_stack[j];
      }
      window_stack[NUM_WINDOWS-1] = 0;
      break;
    }
  }

  // Kill its process
  if(w->proc) {
    w->proc->killed = 1;
    w->proc = 0;
  }

  release(&window_lock);
}

static void
video_window_draw(Window *w, unsigned short color)
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
      buffer[counter] = color;
      counter++;
    }
    counter = counter - (WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2)) + VIDEO_WIDTH;
  }

  // Draw side borders
  for(int r = 0; r < WINDOW_HEIGHT; r++) {
    for(int c = 0; c < WINDOW_BORDER_SIZE; c++) {
      buffer[counter] = color;
      counter++;
    }
    for(int c = 0; c < WINDOW_WIDTH; c++) {
      buffer[counter] = w->pixels[pixel_counter];
      counter++;
      pixel_counter++;
    }
    for(int c = 0; c < WINDOW_BORDER_SIZE; c++) {
      buffer[counter] = color;
      counter++;
    }
    counter = counter - (WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2)) + VIDEO_WIDTH;
  }

  // Draw bottom
  for(int r = 0; r < WINDOW_BORDER_SIZE; r++) {
    for(int c = 0; c < WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2); c++) {
      buffer[counter] = color;
      counter++;
    }
    counter = counter - (WINDOW_WIDTH+(WINDOW_BORDER_SIZE*2)) + VIDEO_WIDTH;
  }

  // Draw close button
  counter = COORD_TO_LINEAR(
    w->y_pos+2,
    w->x_pos+WINDOW_WIDTH + WINDOW_BORDER_SIZE - IMAGE_CLOSE_BUTTON_WIDTH,
    VIDEO_WIDTH);
  pixel_counter = 0;
  for(int r = 0; r < IMAGE_CLOSE_BUTTON_HEIGHT; r++) {
    for(int c = 0; c < IMAGE_CLOSE_BUTTON_WIDTH; c++) {
      buffer[counter] = IMAGE_CLOSE_BUTTON_DATA[pixel_counter];
      counter++;
      pixel_counter++;
    }
    counter = counter + VIDEO_WIDTH - IMAGE_CLOSE_BUTTON_WIDTH;
  }
}
