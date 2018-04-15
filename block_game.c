#include "types.h"
#include "user.h"
#include "font.h"

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

#define COORD_TO_LINEAR(r, c, w) ((r)*(w)+(c))

long a = 2521490391;
long c = 11;
long previous = 0;
int score = 0;

static void get_new_square_coords(int*, int*);
static int abs(int);
static int collision(int, int, int, int);
static void update_score(volatile ushort *);

int
main(int argc, char *argv[])
{
  if(!fork()) {
    volatile ushort *video_buffer = malloc(300*400*2);
    unsigned long long event;
    int x_coord = 0;
    int y_coord = 0;
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
    update_score(video_buffer);
    get_new_square_coords(&x_coord_sq, &y_coord_sq);
    while(1) {
      for(int i = 0; i < 300*400; i++) {
        video_buffer[i] = black;
      }
      unsigned long long character = font[48];
      video_buffer[200] = character;
      if(getinput(&event) != -1) {
        int type = (event >> 48) & 0xFFFF;
        if(type & 16) {
          x_coord = (event >> 32) & 0xFFFF;
          y_coord = (event >> 16) & 0xFFFF;
        }
      }
      if (collision(x_coord, x_coord_sq, y_coord, y_coord_sq)) {
        get_new_square_coords(&x_coord_sq, &y_coord_sq);
        update_score(video_buffer);
      }
      get_new_square_coords(&x_coord_sq, &y_coord_sq);
      for(int i = MIN(x_coord_sq, 400-SQUARE_SIZE); i < MIN(x_coord_sq+SQUARE_SIZE, 400); i++) {
        for(int j = MIN(y_coord_sq, 300-SQUARE_SIZE); j < MIN(y_coord_sq+SQUARE_SIZE, 300); j++) {
          video_buffer[j*400+i] = blue;
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
  long r = a * previous + c;
  previous = r;
  *x = r % 300;
  *y = r % 400;
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
  int threshold = 10;
  return (abs(x1 - x2) < threshold && abs(y1-y2) < threshold);
}

static void
update_score(volatile ushort *video_buffer)
{
  unsigned long long character, char_row, char_col;
  character = font[48 + score];
  char_row = 0;
  for(unsigned int video_row = FONT_HEIGHT; video_row < 1+FONT_HEIGHT; video_row++) {
    char_col = 0;
    for(unsigned int video_col = FONT_WIDTH; video_col < 1+FONT_WIDTH; video_col++) {
      video_buffer[COORD_TO_LINEAR(video_row, video_col, WINDOW_WIDTH)] =
        ((character >> (56 - 8*(char_row/FONT_HEIGHT_MULTIPLIER))) &
         (1 << (char_col/FONT_WIDTH_MULTIPLIER))) ? 0x7FFF : 0;
      char_col++;
    }
    char_row++;
  }
}

