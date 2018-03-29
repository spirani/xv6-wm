#include "types.h"
#include "defs.h"
#include "mouse.h"
#include "traps.h"
#include "x86.h"

unsigned char mouse_cycle = 0;
char mouse_byte[3];
char mouse_x;
char mouse_y;

// Wait for bit 1 of port 0x64 to become clear before writing
static inline void
mouse_wait_to_write(void)
{
  unsigned int timeout = MOUSE_IO_TIMEOUT;
  while(((inb(0x64) & 2) != 0) || (timeout > 0)) {
    timeout--;
  }
}

// Wait for bit 0 of port 0x64 to become set before reading
static inline void
mouse_wait_to_read(void)
{
  unsigned int timeout = MOUSE_IO_TIMEOUT;
  while(((inb(0x64) & 1) == 0) || (timeout > 0)) {
    timeout--;
  }
}

// Tell mouse we're sending a command, then send it
static inline void
mouse_write(unsigned char data)
{
  mouse_wait_to_write();
  outb(0x64, 0xD4);
  mouse_wait_to_write();
  outb(0x60, data);
}

// Reads data from mouse
static unsigned char
mouse_read(void)
{
  mouse_wait_to_read();
  return inb(0x60);
}

// Initialize the mouse, called from main
void
mouseinit(void)
{
  unsigned char status;

  // Get Compaq status byte
  mouse_wait_to_write();
  outb(0x64, 0x20);
  mouse_wait_to_read();
  status = inb(0x60);

  // Enable mouse to generate IRQ12, and enable mouse clock
  status = status | 2;
  status = status & ~32;
  mouse_wait_to_write();
  outb(0x64, 0x60);
  mouse_wait_to_write();
  outb(0x60, status);

  // Set default mouse config
  mouse_write(0xF6);
  mouse_read(); // ACK

  // Automatically generate interrupt when mouse moved/clicked
  mouse_write(0xF4);
  mouse_read(); // ACK

  // Enable interrupt in our controllers
  picenable(IRQ_MOUSE);
  ioapicenable(IRQ_MOUSE, 0);
}

void
mouseintr(void)
{
  switch(mouse_cycle) {
  case 0:
    mouse_byte[0] = inb(0x60);
    mouse_cycle++;
    break;
  case 1:
    mouse_byte[1] = inb(0x60);
    mouse_cycle++;
    break;
  case 2:
    mouse_byte[2] = inb(0x60);
    mouse_x = mouse_byte[1];
    mouse_y = mouse_byte[2];
    mouse_cycle = 0;
    break;
  }
}
