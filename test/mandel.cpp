// Mandelbrot renderer.
// Tests pixel writes, saving, and loading.

//
// Includes

#include "imgcjt.h"

#include <math.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace imgcjt;


//
// Switches

#define DEBUGPALETTE 0


//
// Constants

#define IMGWIDTH 192
#define IMGHEIGHT 108

// This is also the pixel depth.

//#define ITERMAX 64 /* Testing power-of-two oddness for PAM colour. */
//#define ITERMAX 65 /* Testing power-of-two oddness for PAM colour. */
#define ITERMAX 255 /* 8-bit test */
//#define ITERMAX 1024 /* Minimal 16-bit test. */
//#define ITERMAX 65535 /* Maximal 16-bit test. */

// So, "display" shows white instead of black for maximum opacity with
// ITERMAX that's not (2^n-1), but "gimp" shows correct output.

#define XMIN -2.0
#define XMAX 1.2
#define YMIN -0.56
#define YMAX 1.6

#define PALSIZE 8

const float palette[PALSIZE][3] =
{ { 1.0, 0.0, 0.0 },
  { 1.0, 0.5, 0.0 },
  { 1.0, 1.0, 0.0 },
  { 0.0, 1.0, 0.0 },
  { 0.0, 1.0, 1.0 },
  { 0.3, 0.3, 1.0 },
  { 0.5, 0.0, 1.0 },
  { 1.0, 0.0, 1.0 } };



//
// Helper Functions


void do_palette_test(void)
{
  int pidx;
  float rval, gval, bval;
  uint16_t r, g, b, a;
  uint64_t pixval;

  cout << "Palette test:\n";

  for (pidx = 0; pidx < PALSIZE; pidx++)
  {
    rval = palette[pidx][0];
    gval = palette[pidx][1];
    bval = palette[pidx][2];

    r = (uint16_t) round(rval * ITERMAX);
    g = (uint16_t) round(gval * ITERMAX);
    b = (uint16_t) round(bval * ITERMAX);
    a = 0;

    pixval = pix_from_argb_components(a, r, g, b);

    cout << pidx << ":   " << fixed << setprecision(1)
      << rval << " " << gval << " " << bval
      << "    " << setw(16) << setfill('0') << hex << pixval;

    r = 0;
    g = 0;
    b = 0;
    a = 0;

    argb_components_from_pix(pixval, a, r, g, b);

    cout << "    " << setw(4) << a << "  " << setw(4) << r
      << "  " << setw(4) << g << "  " << setw(4) << b
      << "\n";
  }

  cout << "End of palette test.\n";
}


void image_compare(gfxplane &imgsrc, gfxplane *copyptr,
  string label, bool has_alpha)
{
  long width, height;
  uint16_t maxval;
  long h, v;
  uint16_t r1, g1, b1, a1, r2, g2, b2, a2;
  bool is_match;

  cout << ".. Comparing original/reloaded for " << label << ":    ";

  width = imgsrc.get_width();
  height = imgsrc.get_height();
  maxval = imgsrc.get_maxval();

  if (NULL == copyptr)
    cout << "NULL image\n";
  else if ( (copyptr->get_width() != width)
    || (copyptr->get_height() != height)
    || (copyptr->get_maxval() != maxval) )
  {
    cout << "metadata mismatch\n";
  }
  else
  {
    is_match = true;

    // Initialize to avoid compiler warnings.
    r1 = 0;
    r2 = 0;
    g1 = 0;
    g2 = 0;
    b1 = 0;
    b2 = 0;
    a1 = 0;
    a2 = 0;

    for (v = 0; v < height; v++)
      for (h = 0; h < width; h++)
      {
        imgsrc.getpix_argb(h, v, a1, r1, g1, b1);
        copyptr->getpix_argb(h, v, a2, r2, g2, b2);

        if ( (r1 != r2) || (g1 != g2) || (b1 != b2) )
          is_match = false;

        if ( has_alpha && (a1 != a2) )
          is_match = false;
      }

    if (is_match)
      cout << "match\n";
    else
      cout << "pixel data mismatch\n";
  }
}


int get_mandel(float cr, float ci, int imax)
{
  int icount;
  float zr, zi, d2, z2r, z2i;
  bool done;

  icount = 0;
  zr = cr;
  zi = ci;
  do
  {
    d2 = zr * zr + zi * zi;
    done = true;
    if (d2 <= 4.0)
    {
      icount++;
      if (icount < imax)
      {
        done = false;
        z2r = zr * zr - zi * zi;
        z2i = zr * zi;
        z2i += z2i;
        zr = z2r + cr;
        zi = z2i + ci;
      }
    }
  }
  while (!done);

  return icount;
}


uint64_t iter_to_pix(int icount)
{
  int pidx;
  float rval, gval, bval;
  uint16_t r, g, b, a;

  // Trust that icount is in the range 0..ITERMAX and that our component
  // maximum is also ITERMAX.

  // Default to black.
  r = 0;
  g = 0;
  b = 0;

  // Opacity is iteration count.
  a = icount;

  // If we're not at the maximum iteration count, use the palette.
  if (icount < ITERMAX)
  {
    pidx = icount % PALSIZE;

    rval = palette[pidx][0];
    gval = palette[pidx][1];
    bval = palette[pidx][2];

    r = (uint16_t) round(rval * ITERMAX);
    g = (uint16_t) round(gval * ITERMAX);
    b = (uint16_t) round(bval * ITERMAX);
  }

  return pix_from_argb_components(a, r, g, b);
}



//
// Main Program

int main(void)
{
  gfxplane img(IMGWIDTH, IMGHEIGHT, ITERMAX);
  float x, y, xstart, ystart, xinc, yinc;
  int h, v;
  int thiscount;
  gfxplane *readbackimg;


  // Do a palette test to check conversion functions.
#if DEBUGPALETTE
  do_palette_test();
#endif


  // Render the image.

  xstart = XMIN;
  xinc = (XMAX - XMIN);
  xinc /= (IMGWIDTH - 1);

  ystart = YMAX;
  yinc = (YMIN - YMAX);
  yinc /= (IMGHEIGHT - 1);

  y = ystart;
  for (v = 0; v < IMGHEIGHT; v++)
  {
    x = xstart;
    for (h = 0; h < IMGWIDTH; h++)
    {
      thiscount = get_mandel(x, y, ITERMAX);
      img.setpix( h, v, iter_to_pix(thiscount) );
      x += xinc;
    }
    y += yinc;
  }


  // Save the image.

  write_ppm_pgm(img, "output/mandel.ppm", "output/mandel.pgm");
  write_pam(img, "output/mandel.pam");


  // Try loading each of these images and comparing with the original.

  readbackimg = read_ppm("output/mandel.ppm");
  image_compare(img, readbackimg, "PPM", false);
  if (NULL != readbackimg)
    delete readbackimg;

  readbackimg = read_ppm("output/mandel.ppm", "output/mandel.pgm");
  image_compare(img, readbackimg, "PPM+PGM", true);
  if (NULL != readbackimg)
    delete readbackimg;

  readbackimg = read_ppm("output/mandel.pam");
  image_compare(img, readbackimg, "PAM", true);

  // FIXME - Diagnostics.
  if (NULL != readbackimg)
    write_ppm_pgm(*readbackimg, "output/mandel-readback.ppm",
      "output/mandel-readback.pgm");

  if (NULL != readbackimg)
    delete readbackimg;


  // Ended successfully.
  return 0;
}
