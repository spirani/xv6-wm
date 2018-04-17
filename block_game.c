#include "types.h"
#include "user.h"
#include "font.h"

#define green 0x0f00
#define black 0xffff
#define blue  0x00ff
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SQUARE_SIZE 30

#define FONT_HEIGHT_MULTIPLIER 2
#define FONT_WIDTH_MULTIPLIER 1
#define WINDOW_HEIGHT 300
#define WINDOW_WIDTH 400
#define FONT_HEIGHT ((FONT_HEIGHT_MULTIPLIER)*8)
#define FONT_WIDTH ((FONT_WIDTH_MULTIPLIER)*8)
#define TERM_HEIGHT ((WINDOW_HEIGHT)/(FONT_HEIGHT))
#define TERM_WIDTH ((WINDOW_WIDTH)/(FONT_WIDTH))

#define KEY_W 119
#define KEY_A 97
#define KEY_S 115
#define KEY_D 100
#define MOVE 5

#define COORD_TO_LINEAR(r, c, w) ((r)*(w)+(c))

unsigned int nSeed = 5323;
int score = 0;

static void get_new_square_coords(int*, int*);
static int abs(int);
static int collision(int, int, int, int);
//static void update_score(volatile ushort *);
static void move_block(int key, int*, int*);

int
main(int argc, char *argv[])
{
  if(!fork()) {
    volatile ushort *video_buffer = malloc(300*400*2);
    unsigned long long event;
    int x_coord = 50;
    int y_coord = 50;
    int x_coord_sq = 0;
    int y_coord_sq = 0;
    if(initwindow() != 0) {
      printf(1, "something's wrong!\n");
      exit();
    }
    if(initwindow() != -1) {
      printf(1, "something's wrong!\n");
      exit();
    }
   // update_score(video_buffer);
    get_new_square_coords(&x_coord_sq, &y_coord_sq);
    get_new_square_coords(&x_coord, &y_coord);
    while(1) {
      for(int i = 0; i < 300*400; i++) {
        video_buffer[i] = black;
      }
      unsigned long long character = font[48];
      video_buffer[200] = character;
      if(getinput(&event) != -1) {
        if (((event >> 48) & 0xffff) & 32) {
          move_block(event & 0xffff, &x_coord_sq, &y_coord_sq);
        }
      }
      if (collision(x_coord, x_coord_sq, y_coord, y_coord_sq)) {
        get_new_square_coords(&x_coord_sq, &y_coord_sq);
	get_new_square_coords(&x_coord, &y_coord);
      }
      for(int i = MIN(x_coord_sq, 400-SQUARE_SIZE); i < MIN(x_coord_sq+SQUARE_SIZE, 400); i++) {
        for(int j = MIN(y_coord_sq, 300-SQUARE_SIZE); j < MIN(y_coord_sq+SQUARE_SIZE, 300); j++) {
          video_buffer[j*400+i] = blue;
        }
      }

      for (int i = MIN(x_coord, 400-SQUARE_SIZE); i < MIN(x_coord+SQUARE_SIZE, 400); i++) {
	for (int j = MIN(y_coord, 300-SQUARE_SIZE); j < MIN(y_coord + SQUARE_SIZE, 300); j++) {
	  video_buffer[j*400+i] = green;
        }
      }
      if(drawwindow((void*)video_buffer) != 0) {
        printf(1, "something's wrong!\n");
      }
    }
  } else {
    exit();
  }
}

static void
get_new_square_coords(int* x, int* y)
{
  nSeed = (8253729 * nSeed + 2396403);
  int r = nSeed % 32767;
  *x = r % (400 - SQUARE_SIZE);
  *y = r % (300 - SQUARE_SIZE);
}

static int
abs(int x)
{
  if (x < 0)
    return x*-1;
  return x;
}

static int
collision(int x1, int x2, int y1, int y2)
{
  int threshold = SQUARE_SIZE;
  return (abs(x1 - x2) < threshold && abs(y1-y2) < threshold);
}



static void
move_block(int key, int* x, int* y)
{
  if (key == KEY_W) {
    *y = *y - MOVE;
    if (*y < 0)
      *y = 0;
  }
  if (key == KEY_A) {
    *x = *x - MOVE;
    if (*x < 0)
      *x = 0;
  }
  if (key == KEY_S) {
    *y = *y + MOVE;
    if (*y > (300-SQUARE_SIZE))
      *y = (300-SQUARE_SIZE);
  }
  if (key == KEY_D) {
    *x = *x + MOVE;
    if (*x > (400-SQUARE_SIZE))
      *x = (400-SQUARE_SIZE);
  }
}
