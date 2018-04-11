#include "types.h"
#include "user.h"
#include "font.h"

#define FONT_HEIGHT_MULTIPLIER 2
#define FONT_WIDTH_MULTIPLIER 1
#define WINDOW_HEIGHT 300
#define WINDOW_WIDTH 400
#define FONT_HEIGHT ((FONT_HEIGHT_MULTIPLIER)*8)
#define FONT_WIDTH ((FONT_WIDTH_MULTIPLIER)*8)
#define TERM_HEIGHT ((WINDOW_HEIGHT)/(FONT_HEIGHT))
#define TERM_WIDTH ((WINDOW_WIDTH)/(FONT_WIDTH))

#define COORD_TO_LINEAR(r, c, w) ((r)*(w)+(c))

struct term_line {
	unsigned char data[TERM_WIDTH];
	unsigned int write_head;
};

struct term_line term_lines[TERM_HEIGHT];
unsigned int active_term_line = 0;

int shell_pipefd_r[2];
int shell_pipefd_w[2];
char *shell_argv[] = {"sh", 0};
char my_buf;

static void init_term();
static void draw_term(volatile ushort *);
static void write_term(unsigned char);

int
main(int argc, char *argv[])
{
  if(1) {
    init_term();
    volatile ushort *video_buffer = malloc(300*400*2);
    unsigned long long event;
    if(initwindow() != 0) {
      printf(1, "something's wrong!\n");
      exit();
    }
    while(1) {
      if(getinput(&event) != -1) {
        if(((event >> 48) & 0xFFFF) & 32) {
          write_term(event & 0xFFFF);
          if(write(shell_pipefd_w[1], &event, 1) == -1) {
            printf(1, "WRITE ERROR!\n");
          }
          draw_term(video_buffer);
          if(drawwindow((void*)video_buffer) != 0) {
            printf(1, "something's wrong!\n");
            exit();
          }
        }
      }
      while(1) {
        if(!pipebytes(shell_pipefd_r[0])) {
          break;
        }
        read(shell_pipefd_r[0], &my_buf, 1);
        write_term(my_buf);
        draw_term(video_buffer);
        if(drawwindow((void*)video_buffer) != 0) {
          printf(1, "something's wrong!\n");
          exit();
        }
      }
      /* write(shell_pipefd[1], &event, 1); */
    }
  } else {
    exit();
  }
}

static void
init_term()
{
  for(int i = 0; i < TERM_HEIGHT; i++) {
    term_lines[i].write_head = 0;
    for(int j = 0; j < TERM_WIDTH; j++) {
      term_lines[i].data[j] = 32;
    }
  }
  if(pipe(shell_pipefd_r) != 0) {
    printf(1, "something's wrong!\n");
    exit();
  }
  if(pipe(shell_pipefd_w) != 0) {
    printf(1, "something's wrong!\n");
    exit();
  }
  if(!fork()) {
    close(0);
    dup(shell_pipefd_w[0]);
    close(shell_pipefd_w[0]);
    close(shell_pipefd_w[1]);
    close(1);
    dup(shell_pipefd_r[1]);
    close(2);
    dup(shell_pipefd_r[1]);
    close(shell_pipefd_r[0]);
    close(shell_pipefd_r[1]);
    exec("sh", shell_argv);
  } else {
    close(shell_pipefd_w[0]);
    close(shell_pipefd_r[1]);
  }
}

static void
draw_term(volatile ushort *video_buffer)
{
  unsigned long long character, char_row, char_col;
  for(unsigned int curr_line = 0; curr_line < TERM_HEIGHT; curr_line++) {
    for(unsigned int curr_col = 0; curr_col < TERM_WIDTH; curr_col++) {
      character = font[term_lines[curr_line].data[curr_col]];
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

static void write_term(unsigned char write_data)
{
  // Need to shift?
  if(active_term_line >= TERM_HEIGHT) {
    for(int i = 0; i < TERM_HEIGHT-1; i++) {
      term_lines[i] = term_lines[i+1];
    }
    for(int i = 0; i < TERM_WIDTH; i++) {
      term_lines[TERM_HEIGHT-1].data[i] = 32;
    }
    term_lines[TERM_HEIGHT-1].write_head = 0;
    active_term_line = TERM_HEIGHT-1;
  }
  if(write_data != '\n') {
    term_lines[active_term_line].data[term_lines[active_term_line].write_head] = write_data;
    term_lines[active_term_line].write_head += 1;
  }
  if((term_lines[active_term_line].write_head >= TERM_WIDTH) ||
     (write_data == '\n') || (write_data == '\r')) {
    active_term_line++;
  }
}
