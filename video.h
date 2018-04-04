#define VIDEO_BASE 0xFD000000
#define VIDEO_WIDTH 1024
#define VIDEO_HEIGHT 768

#define NUM_WINDOWS 10
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 400

#define COORD_TO_LINEAR(r, c, w) ((r)*(w)+(c))
#define ROUND_UP_PAGE_BOUND(v) ((((v) & 0xFFF) == 0) ? ((v) >> 12) : (((v) >> 12) + 1))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
  unsigned short pixels[WINDOW_WIDTH*WINDOW_HEIGHT];
  unsigned int x_pos;
  unsigned int y_pos;
} Window;
