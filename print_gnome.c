#include "types.h"
#include "user.h"
#include "font.h"
#include "printf.h"

#define FONT_HEIGHT_MULTIPLIER 2
#define FONT_WIDTH_MULTIPLIER 1
#define WINDOW_HEIGHT 300
#define WINDOW_WIDTH 400
#define FONT_HEIGHT ((FONT_HEIGHT_MULTIPLIER)*8)
#define FONT_WIDTH ((FONT_WIDTH_MULTIPLIER)*8)
#define WIN_HEIGHT ((WINDOW_HEIGHT)/(FONT_HEIGHT))
#define WIN_WIDTH ((WINDOW_WIDTH)/(FONT_WIDTH))

#define COORD_TO_LINEAR(r, c, w) ((r)*(w)+(c))

struct win_line {
  unsigned char data[WIN_WIDTH];
  unsigned char write_head;
};

struct win_line win_lines[WIN_HEIGHT];
unsigned int active_win_line = 0;
const char* buf;
char* gnome = "	          ______\n .-'      |\n /      <\\|\n /        \\'\n | __.-  o-o\n /  C  -.__) \\ \n / ',         |\n |   '-,_,___,'\n (,,) ====[_]=|\n ' .  _____ /\n | -|-|_\n |___)__)\n\0";

static void
draw_win(volatile ushort *video_buffer)
{
  unsigned long long character, char_row, char_col;
  for(unsigned int curr_line = 0; curr_line < WIN_HEIGHT; curr_line++) {
    for(unsigned int curr_col = 0; curr_col < WIN_WIDTH; curr_col++) {
      character = font[win_lines[curr_line].data[curr_col]];
      char_row = 0;
      for(unsigned int video_row = curr_line*FONT_HEIGHT; video_row < (curr_line+1)*FONT_HEIGHT; video_row++) {
        char_col = 0;
        for(unsigned int video_col = curr_col*FONT_WIDTH; video_col < (curr_col+1)*FONT_WIDTH; video_col++) {
          video_buffer[COORD_TO_LINEAR(video_row, video_col, WINDOW_WIDTH)] =
            ((character >> (56 - 8*(char_row/FONT_HEIGHT_MULTIPLIER))) &
             (1 << (char_col/FONT_WIDTH_MULTIPLIER))) ? 0x7FFF : 0;
          char_col++;
        }
        char_row++;
      }
    }
  }
}

static void write_win(unsigned char write_data)
{
  // Need to shift?
  if(active_win_line >= WIN_HEIGHT) {
    for(int i = 0; i < WIN_HEIGHT-1; i++) {
      win_lines[i] = win_lines[i+1];
    }
    for(int i = 0; i < WIN_WIDTH; i++) {
      win_lines[WIN_HEIGHT-1].data[i] = 32;
      }
      win_lines[WIN_HEIGHT-1].write_head = 0;
      active_win_line = WIN_HEIGHT-1;
  }
  if(write_data != '\n') {
    win_lines[active_win_line].data[win_lines[active_win_line].write_head] = write_data;
    win_lines[active_win_line].write_head += 1;
    win_lines[active_win_line].data[win_lines[active_win_line].write_head] = 0;
  }
  if((win_lines[active_win_line].write_head >= WIN_WIDTH) ||
      (write_data == '\n') || (write_data == '\r')) {
    win_lines[active_win_line].data[win_lines[active_win_line].write_head] = 32;
    active_win_line++;
  }
}

static void
init_win()
{
  for (int i = 0; i < WIN_HEIGHT; i++) {
    win_lines[i].write_head = 0;
    for (int j = 0; j < WIN_WIDTH; j++) {
      win_lines[i].data[j] = 32;
    }
  }
}

int
main(int argc, char *argv[])
{
  init_win();
  volatile ushort* video_buffer = malloc(300*400*2);
  if (initwindow()) {
    printf(1, "Something's wrong");
    exit();
  }

  while(*gnome != '\0') {
    write_win(((unsigned long long) *gnome++) & 0xFFFF);
    draw_win((void*)video_buffer);
    if(drawwindow((void*)video_buffer) != 0) {
      printf(1, "something's wrong!\n");
      exit();
    }
  }
  exit();
}
