// Line demo.
// Tests line drawing, including a few special cases.

//
// Includes

#include "imgcjt.h"

using namespace std;
using namespace imgcjt;


//
// Constants

#define WIDTH 640
#define HEIGHT 480
#define MAXVAL 0xff
//#define MAXVAL 0xffff

#define PCOUNT 8

const long polyverts[PCOUNT][2] =
{
  { -50,   0 },
  { -35, -35 },
  {   0, -50 },
  {  35, -35 },
  {  50,   0 },
  {  35,  35 },
  {   0,  50 },
  { -35,  35 }
};

#define MULTSMALL 1
#define MULTBIG 10

#define HSTRIDECOARSE 8
#define HSTRIDEFINE 4
#define VSTRIDECOARSE 6
#define VSTRIDEFINE 3

#define BLEND_NO false
#define BLEND_YES true
#define AA_NO false
#define AA_YES true

// Transparent black.
//#define BGCOL 0
// Opaque black.
#define BGCOL pix_from_argb_components(MAXVAL, 0, 0, 0)



//
// Functions


void line_wrapper( gfxplane &img, long h1, long v1, long h2, long v2,
  uint64_t pixval, bool want_blend, bool want_aa )
{
  if (want_blend)
  {
    if (want_aa)
      img.blendline_aa(h1, v1, h2, v2, pixval);
    else
      img.blendline(h1, v1, h2, v2, pixval);
  }
  else
  {
    if (want_aa)
      img.setline_aa(h1, v1, h2, v2, pixval);
    else
      img.setline(h1, v1, h2, v2, pixval);
  }
}



void draw_circle( gfxplane &img, uint64_t pixval, uint64_t mult,
  bool want_blend, bool want_aa )
{
  long h, v, p1, p2, pskip;

  h = WIDTH >> 1;
  v = HEIGHT >> 1;
  pskip = (PCOUNT >> 1) - 1;
  for (; pskip > 0; pskip--)
    for (p1 = 0; p1 < PCOUNT; p1++)
    {
      p2 = (p1 + pskip) % PCOUNT;

      line_wrapper(img,
        h + polyverts[p1][0] * mult, v + polyverts[p1][1] * mult,
        h + polyverts[p2][0] * mult, v + polyverts[p2][1] * mult,
        pixval, want_blend, want_aa );
    }

  // Test a single-pixel line too.
  line_wrapper(img, h, v, h, v, pixval, want_blend, want_aa);
}



void draw_border( gfxplane &img, uint64_t pixval,
  uint64_t hstride, uint64_t vstride, bool want_blend, bool want_aa )
{
  int h, v;

  v = 0;
  for (h = 0; h < WIDTH; h+= hstride)
  {
    line_wrapper(img, 0, v, h+(hstride-1), (HEIGHT-1),
      pixval, want_blend, want_aa);
    line_wrapper(img, h, 0, (WIDTH-1), v+(vstride-1),
      pixval, want_blend, want_aa);

    v += vstride;
  }
}



void draw_eye( gfxplane &img, uint64_t eyecol, uint64_t lidcol,
  long eyemult, long lidhstride, long lidvstride,
  bool want_blend, bool want_aa, string basename )
{
  img.setrect(0, 0, WIDTH-1, HEIGHT-1, BGCOL);

  if (0 < eyemult)
    draw_circle(img, eyecol, eyemult, want_blend, want_aa);

  if ((0 < lidhstride) && (0 < lidvstride))
    draw_border(img, lidcol, lidhstride, lidvstride, want_blend, want_aa);

  write_ppm_pgm(img, basename + ".ppm", basename + ".pgm");
  write_pam(img, basename + ".pam");
}



//
// Main Program

int main(void)
{
  gfxplane img(WIDTH, HEIGHT, MAXVAL);
  uint64_t eyecol, lidcol;

  eyecol = pix_from_argb_components( (MAXVAL >> 1), 0, MAXVAL, 0 );
  lidcol = pix_from_argb_components( (MAXVAL >> 1), MAXVAL, 0, 0 );


  // Baseline image.
  draw_eye(img, eyecol, lidcol, MULTSMALL, HSTRIDECOARSE, VSTRIDECOARSE,
    BLEND_NO, AA_NO, "output/baseline");

  // Test out-of-bounds drawing.
  draw_eye(img, eyecol, lidcol, MULTBIG, 0, 0,
    BLEND_NO, AA_NO, "output/big");

  // Test AA, blended, and blended AA.
  draw_eye(img, eyecol, lidcol, MULTSMALL, HSTRIDECOARSE, VSTRIDECOARSE,
    BLEND_NO, AA_YES, "output/aa");
  draw_eye(img, eyecol, lidcol, MULTSMALL, HSTRIDEFINE, VSTRIDEFINE,
    BLEND_YES, AA_NO, "output/blend");
  draw_eye(img, eyecol, lidcol, MULTSMALL, HSTRIDEFINE, VSTRIDEFINE,
    BLEND_YES, AA_YES, "output/blend_aa");


  // Report success.
  return 0;
}



//
// This is the end of the file.
