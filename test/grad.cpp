// Gradient test.
// Tests maxval rescaling.

//
// Includes

#include "imgcjt.h"

using namespace std;
using namespace imgcjt;


//
// Constants

// Remember that counting starts at 0.
#define SMALLMAX 2
//#define BIGMAX 4
// This highlights the "end values have half the range" case.
// Want 4/4/4 cells, get 3/6/3 cells with naive scaling.
#define BIGMAX 11

#define COLWIDTH 10
#define SEGHEIGHT 10

#define IMGWIDTH (COLWIDTH*4)
#define SMALLHEIGHT (SEGHEIGHT*(SMALLMAX+1))
#define BIGHEIGHT (SEGHEIGHT*(BIGMAX+1))


//
// Main Program

int main(void)
{
  gfxplane imgsmall(IMGWIDTH, SMALLHEIGHT, SMALLMAX);
  gfxplane imgsame(IMGWIDTH, SMALLHEIGHT, SMALLMAX);
  gfxplane imgbig(IMGWIDTH, BIGHEIGHT, BIGMAX);
  long h, v, c;


  // Initialize.
  imgsmall.setrect(0, 0, IMGWIDTH-1, SMALLHEIGHT-1, 0);
  imgsame.setrect(0, 0, IMGWIDTH-1, SMALLHEIGHT-1, 0);
  imgbig.setrect(0, 0, IMGWIDTH-1, BIGHEIGHT-1, 0);


  // Build gradient strips.

  for (v = 0; v < SMALLHEIGHT; v++)
  {
    c = v / SEGHEIGHT;
    for (h = 0; h < COLWIDTH; h++)
    {
      imgsmall.setpix_argb(h, v, c, SMALLMAX, SMALLMAX, SMALLMAX);
      imgsmall.setpix_argb(h + COLWIDTH, v, SMALLMAX, c, 0, 0);
      imgsmall.setpix_argb(h + 2*COLWIDTH, v, SMALLMAX, 0, c, 0);
      imgsmall.setpix_argb(h + 3*COLWIDTH, v, SMALLMAX, 0, 0, c);
    }

    for (h = 0; h < IMGWIDTH; h++)
      imgsame.setpix( h, v, imgsmall.getpix(h, v) );
  }

  for (v = 0; v < BIGHEIGHT; v++)
  {
    c = v / SEGHEIGHT;
    for (h = 0; h < COLWIDTH; h++)
    {
      imgbig.setpix_argb(h, v, c, BIGMAX, BIGMAX, BIGMAX);
      imgbig.setpix_argb(h + COLWIDTH, v, BIGMAX, c, 0, 0);
      imgbig.setpix_argb(h + 2*COLWIDTH, v, BIGMAX, 0, c, 0);
      imgbig.setpix_argb(h + 3*COLWIDTH, v, BIGMAX, 0, 0, c);
    }
  }


  // Save, rescale, and resave.

  write_ppm_pgm(imgsmall,
    "output/grad-small-orig.ppm", "output/grad-small-orig.pgm");
  write_ppm_pgm(imgsame,
    "output/grad-same-orig.ppm", "output/grad-same-orig.pgm");
  write_ppm_pgm(imgbig,
    "output/grad-big-orig.ppm", "output/grad-big-orig.pgm");

  imgsmall.rescale_maxval(BIGMAX);
  imgsame.rescale_maxval(SMALLMAX);
  imgbig.rescale_maxval(SMALLMAX);

  write_ppm_pgm(imgsmall,
    "output/grad-small-to-big.ppm", "output/grad-small-to-big.pgm");
  write_ppm_pgm(imgsame,
    "output/grad-same-to-same.ppm", "output/grad-same-to-same.pgm");
  write_ppm_pgm(imgbig,
    "output/grad-big-to-small.ppm", "output/grad-big-to-small.pgm");


  // Ended successfully.
  return 0;
}


//
// This is the end of the file.
