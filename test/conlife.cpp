// Conway's Life simulator.
// Tests pixel reads and writes and saving.

//
// Includes

#include "imgcjt.h"

#include <iostream>
#include <iomanip>

using namespace std;
using namespace imgcjt;


//
// Switches

#define DEBUGCELLCOUNTS 1


//
// Constants

#define BOARDWIDTH 20
#define BOARDHEIGHT 15

#define ITERMAX 20

#define PIXMAX 15

#define STARTSIZE 5
typedef int startboard_t[STARTSIZE*STARTSIZE];

// Evolves into a ring of blinkers.
const startboard_t board_red =
{
  0, 0, 0, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 1, 1, 0,
  0, 0, 1, 0, 0,
  0, 0, 0, 0, 0
};

// Evolves into a ring of beehives.
const startboard_t board_green =
{
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 1, 0, 0
};

// Glider.
const startboard_t board_blue =
{
  0, 0, 0, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 0, 1, 0,
  0, 1, 1, 1, 0,
  0, 0, 0, 0, 0
};

// Explodes.
const startboard_t board_alpha =
{
  0, 0, 0, 0, 0,
  0, 0, 1, 1, 0,
  0, 1, 1, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 0, 0, 0
};



//
// Helper functions.


// Handles wrapping and selects a colour plane.
uint16_t get_component(gfxplane &board, int h, int v, int comp)
{
  uint16_t r, g, b, a;
  uint16_t result;

  result = 0;

  // FIXME - Cheat by knowing that negative values will be small.
  // Otherwise we have to deal with modulus of negative numbers, which
  // is ostensibly standard but actually implementation-dependent.
  h = (h + BOARDWIDTH) % BOARDWIDTH;
  v = (v + BOARDHEIGHT) % BOARDHEIGHT;

  // Avoid compile-time errors by initializing to safe values.
  r = 0;
  g = 0;
  b = 0;
  a = 0;

  board.getpix_argb(h, v, a, r, g, b);

  switch (comp)
  {
    case 0: result = r; break;
    case 1: result = g; break;
    case 2: result = b; break;
    default: result = a; break;
  }

  return result;
}



// Handles wrapping and selects a colour plane.
void set_component(gfxplane &board, int h, int v, int comp, uint16_t newval)
{
  uint16_t r, g, b, a;

  // FIXME - Cheat by knowing that negative values will be small.
  // Otherwise we have to deal with modulus of negative numbers, which
  // is ostensibly standard but actually implementation-dependent.
  h = (h + BOARDWIDTH) % BOARDWIDTH;
  v = (v + BOARDHEIGHT) % BOARDHEIGHT;

  // Avoid compile-time errors by initializing to safe values.
  r = 0;
  g = 0;
  b = 0;
  a = 0;

  // Fetch the old values.
  board.getpix_argb(h, v, a, r, g, b);

  // Overwrite the desired component.
  switch (comp)
  {
    case 0: r = newval; break;
    case 1: g = newval; break;
    case 2: b = newval; break;
    default: a = newval; break;
  }

  // Save the modified values.
  board.setpix_argb(h, v, a, r, g, b);
}



//
// Main Program

int main(void)
{
  gfxplane board(BOARDWIDTH, BOARDHEIGHT, PIXMAX);
  int h, v, oset;
  int hstart, vstart;
  uint16_t r, g, b, a;
  int icount, colidx;
  int hofs, vofs, cellcount;
  uint16_t newboard[BOARDHEIGHT][BOARDWIDTH];
  int totalcount;

  // Initialize.
  board.setrect(0, 0, BOARDWIDTH-1, BOARDHEIGHT-1, 0);

  // Draw the starting patterns.

  hstart = (BOARDWIDTH - STARTSIZE) >> 1;
  vstart = (BOARDHEIGHT - STARTSIZE) >> 1;

  oset = 0;
  for (v = 0; v < STARTSIZE; v++)
    for (h = 0; h < STARTSIZE; h++)
    {
      r = board_red[oset] * PIXMAX;
      g = board_green[oset] * PIXMAX;
      b = board_blue[oset] * PIXMAX;
      a = board_alpha[oset] * PIXMAX;
      oset++;

      board.setpix_argb(h + hstart, v + vstart, a, r, g, b);
    }


  // Update each plane separately.

  for (icount = 0; icount < ITERMAX; icount++)
  {
    // Diagnostics.
#if DEBUGCELLCOUNTS
    cout << "Iteration " << setw(3) << icount << ":  ";
#endif

    for (colidx = 0; colidx < 4; colidx++)
    {
      // Diagnostics.
#if DEBUGCELLCOUNTS
      totalcount = 0;
      for (v = 0; v < BOARDHEIGHT; v++)
        for (h = 0; h < BOARDWIDTH; h++)
          totalcount += get_component(board, h, v, colidx);
      totalcount /= PIXMAX;
      cout << setw(4) << totalcount;
#endif

      // Compute new cell values.
      // The component accessors handle wrapping.
      for (v = 0; v < BOARDHEIGHT; v++)
        for (h = 0; h < BOARDWIDTH; h++)
        {
          cellcount = 0;
          for (vofs = -1; vofs <= 1; vofs++)
            for (hofs = -1; hofs <= 1; hofs++)
              if ( (hofs != 0) || (vofs != 0) )
                if (0 != get_component(board, h+hofs, v+vofs, colidx))
                  cellcount++;

          newboard[v][h] = get_component(board, h, v, colidx);
          if ( (cellcount < 2) || (cellcount > 3) )
            newboard[v][h] = 0;
          else if (3 == cellcount)
            newboard[v][h] = PIXMAX;
        }

      // Overwrite the old board.
      for (v = 0; v < BOARDHEIGHT; v++)
        for (h = 0; h < BOARDWIDTH; h++)
          set_component(board, h, v, colidx, newboard[v][h]);
    }

    // Diagnostics.
#if DEBUGCELLCOUNTS
    cout << "\n";
#endif
  }


  // Save the resulting image.
  write_ppm_pgm(board, "output/conlife.ppm", "output/conlife.pgm");
  write_pam(board, "output/conlife.pam");


  // Ended successfully.
  return 0;
}


//
// This is the end of the file.
