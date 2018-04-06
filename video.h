#define VIDEO_BASE 0xFD000000
#define VIDEO_WIDTH 1024
#define VIDEO_HEIGHT 768

#define NUM_WINDOWS 10
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300
#define WINDOW_TITLEBAR_HEIGHT 20
#define WINDOW_BORDER_SIZE 4

#define WINDOW_ACTIVE_COLOR 0x3DEF
#define WINDOW_INACTIVE_COLOR 0x1CE7

#define COORD_TO_LINEAR(r, c, w) ((r)*(w)+(c))
#define ROUND_UP_PAGE_BOUND(v) ((((v) & 0xFFF) == 0) ? ((v) >> 12) : (((v) >> 12) + 1))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
  volatile unsigned short pixels[WINDOW_WIDTH*WINDOW_HEIGHT];
  unsigned int x_pos;
  unsigned int y_pos;
  char active;
} Window;

static void video_mouse_handle();
static Window *video_window_create(int win_x, int win_y);
static void video_window_destroy(Window *w);
static void video_window_draw(Window *w, unsigned short color);
