// Rectangle and compositing demo.
// Tests various edge cases.

//
// Includes

#include "imgcjt.h"

using namespace std;
using namespace imgcjt;


//
// Constants

#define WIDTH 160
#define HEIGHT 120
#define MAXVAL 0xff
//#define MAXVAL 0xffff

#define TILESIZE 32


// Palette and colours.

const uint64_t colblk =
  pix_from_argb_components( MAXVAL, 0, 0, 0 );
const uint64_t colwht =
  pix_from_argb_components( MAXVAL, MAXVAL, MAXVAL, MAXVAL );

// Mandelbrot palette.
const uint64_t colred =
  pix_from_argb_components( MAXVAL, MAXVAL, 0, 0 );
const uint64_t colorg =
  pix_from_argb_components( MAXVAL, MAXVAL, MAXVAL >> 1, 0 );
const uint64_t colyel =
  pix_from_argb_components( MAXVAL, MAXVAL, MAXVAL, 0 );
const uint64_t colgrn =
  pix_from_argb_components( MAXVAL, 0, MAXVAL, 0 );
const uint64_t colcyn =
  pix_from_argb_components( MAXVAL, 0, MAXVAL, MAXVAL );
const uint64_t colblu =
  pix_from_argb_components( MAXVAL, MAXVAL >> 2, MAXVAL >> 2 , MAXVAL );
const uint64_t colind =
  pix_from_argb_components( MAXVAL, MAXVAL >> 1, 0, MAXVAL );
const uint64_t colmag =
  pix_from_argb_components( MAXVAL, MAXVAL, 0, MAXVAL );

// Partly-transparent white.
const uint64_t colxparwht =
  pix_from_argb_components( MAXVAL >> 1, MAXVAL, MAXVAL, MAXVAL );

// Transparent black.
const uint64_t colxpar =
  pix_from_argb_components( 0, 0, 0, 0 );



//
// Functions


void make_rectangle_test(gfxplane &img)
{
  long pos1, pos2, pos3, pos4, poshalf;

  poshalf = TILESIZE >> 1;
  pos1 = TILESIZE;
  pos2 = pos1 + poshalf;
  pos3 = pos1 + pos1;
  pos4 = pos3 + poshalf;

  img.setrect(0, 0, WIDTH-1, HEIGHT-1, colblk);

  // Normal drawing.
  img.setrect( pos1, pos1, pos3, pos3, colred );

  // Reverse coordinates.
  img.setrect( pos4, pos4, pos2, pos2, colorg );

  // Overlapping each of the corners.
  img.setrect( -poshalf, -poshalf, poshalf, poshalf, colyel );
  img.setrect( WIDTH - poshalf, -poshalf, WIDTH + poshalf, poshalf, colgrn );
  img.setrect( WIDTH - poshalf, HEIGHT - poshalf,
    WIDTH + poshalf, HEIGHT + poshalf, colcyn );
  img.setrect( -poshalf, HEIGHT - poshalf, poshalf, HEIGHT + poshalf,
    colblu );

  // Blending.
  img.blendrect( pos2, pos1, pos4, pos3, colxparwht );
}



void make_glyph(gfxplane &img)
{
  long hmax, vmax, hofs, vofs;

  hmax = img.get_width() - 1;
  vmax = img.get_height() - 1;
  hofs = hmax - (hmax >> 2);
  vofs = vmax - (vmax >> 1) + (vmax >> 3);

  // Draw a shape with an alpha-blended boundary.
  // A box with a hatch mark so we can see if it's rotated/mirrored.

  // Use nested rectangles to make life easier.
  img.setrect(0, 0, hmax, vmax, colxparwht);
  img.setrect(1, 1, hmax-1, vmax-1, colwht);
  img.setrect(2, 2, hmax-2, vmax-2, colmag);
  img.setrect(3, 3, hmax-3, vmax-3, colwht);
  img.setrect(4, 4, hmax-4, vmax-4, colxparwht);
  img.setrect(5, 5, hmax-5, vmax-5, colxpar);

  // Draw the hatch mark.
  img.setrect(hofs-2, vofs-2, hmax-4, vofs+2, colxparwht);
  img.setrect(hofs-1, vofs-1, hmax-3, vofs+1, colwht);
  img.setrect(hofs, vofs, hmax-2, vofs, colmag);
}



void make_background(gfxplane &img)
{
  long h, v, thisval;
  uint16_t r, g, b;

  img.setrect( 0, 0, WIDTH-1, HEIGHT-1, colblk );

  // Make a gradient background that contrasts with itself.
  for (v = 0; v < HEIGHT; v++)
    for (h = 0; h < WIDTH; h++)
    {
      thisval = (h * MAXVAL) / WIDTH;
      r = (uint16_t) thisval;

      thisval = (v * MAXVAL) / HEIGHT;
      g = (uint16_t) thisval;

      b = MAXVAL >> 1;

      img.setpix_argb( h, v, MAXVAL, r, g, b );
    }
}



void do_glyph_test(gfxplane &img, gfxplane &glyph, string prefix)
{
  long poshalf, posless, posmore, tmax;
  long pos1, pos2, posh1, posh2, posv1, posv2;

  poshalf = TILESIZE >> 1;
  posless = poshalf - (TILESIZE >> 3);
  posmore = poshalf + (TILESIZE >> 3);

  pos1 = TILESIZE;
  pos2 = TILESIZE * 3;

  posh1 = TILESIZE;
  posh2 = TILESIZE * 2 + poshalf;
  posv1 = TILESIZE - (TILESIZE >> 3);
  posv2 = TILESIZE * 2 + (TILESIZE >> 3);

  tmax = TILESIZE-1;


  // Copying; middle, each corner, and off-image on the destination.
  // Corners are offset a bit, to catch wrapping.
  // Source is the whole glyph.

  make_background(img);

  // Middle.
  img.copyfrom(glyph, 0, 0, tmax, tmax, pos1, pos1);

  // Corners.
  img.copyfrom(glyph, 0, 0, tmax, tmax, -posless, -posless);
  img.copyfrom(glyph, 0, 0, tmax, tmax, -poshalf, HEIGHT-poshalf);
  img.copyfrom(glyph, 0, 0, tmax, tmax, WIDTH-posmore, -posmore);
  img.copyfrom(glyph, 0, 0, tmax, tmax, WIDTH-poshalf, HEIGHT-posless);

  // Off-image.
  img.copyfrom(glyph, 0, 0, tmax, tmax, WIDTH+pos1, HEIGHT+pos1);
  img.copyfrom(glyph, 0, 0, tmax, tmax, -pos1, -pos1);

  // Middle swapped.
  img.copyfrom(glyph, tmax, tmax, 0, 0, pos2, pos2);

  write_ppm(img, prefix + "copy.ppm");


  // Blending; middle, each corner, and off-image on the destination.
  // Corners are offset a bit, to catch wrapping.
  // Source is the whole glyph.

  make_background(img);

  // Middle.
  img.blendfrom(glyph, 0, 0, tmax, tmax, pos1, pos1);

  // Corners.
  img.blendfrom(glyph, 0, 0, tmax, tmax, -posless, -posless);
  img.blendfrom(glyph, 0, 0, tmax, tmax, -poshalf, HEIGHT-poshalf);
  img.blendfrom(glyph, 0, 0, tmax, tmax, WIDTH-posmore, -posmore);
  img.blendfrom(glyph, 0, 0, tmax, tmax, WIDTH-poshalf, HEIGHT-posless);

  // Off-image.
  img.blendfrom(glyph, 0, 0, tmax, tmax, WIDTH+pos1, HEIGHT+pos1);
  img.blendfrom(glyph, 0, 0, tmax, tmax, -pos1, -pos1);

  // Middle swapped.
  img.blendfrom(glyph, tmax, tmax, 0, 0, pos2, pos2);

  write_ppm(img, prefix + "blend.ppm");


  // Incomplete copies from the glyph itself.

  make_background(img);

  // Edges of both the glyph and the destination.

  img.blendfrom(glyph, posmore, 0, tmax+posmore, tmax, WIDTH-posmore, posv1);
  img.blendfrom(glyph, posless, 0, tmax+posless, tmax, WIDTH-posless, posv2);

  img.blendfrom(glyph, -posmore, 0, tmax-posmore, tmax, -posless, posv1);
  img.blendfrom(glyph, -posless, 0, tmax-posless, tmax, -posmore, posv2);

  img.blendfrom(glyph, 0, posmore, tmax, tmax+posmore, posh1, HEIGHT-posmore);
  img.blendfrom(glyph, 0, posless, tmax, tmax+posless, posh2, HEIGHT-posless);

  img.blendfrom(glyph, 0, -posmore, tmax, tmax-posmore, posh1, -posless);
  img.blendfrom(glyph, 0, -posless, tmax, tmax-posless, posh2, -posmore);

  // Off-image on the source.
  img.blendfrom(glyph, -TILESIZE, -TILESIZE, -1, -1, posh1, posh1);
  img.blendfrom(glyph, TILESIZE, TILESIZE, TILESIZE+tmax, TILESIZE+tmax,
    posh2, posh2);

  write_ppm(img, prefix + "offblend.ppm");
}



void make_self_copy_test(gfxplane &img)
{
  long hsrc, vsrc, hdst, vdst, dsthinc, dstvinc, srchinc, srcvinc, tmax;

  dsthinc = TILESIZE >> 2;
  dstvinc = TILESIZE >> 4;

  srchinc = -dsthinc;
  srcvinc = dstvinc;

  tmax = TILESIZE-1;


  make_background(img);

  hsrc = WIDTH+TILESIZE;
  vsrc = TILESIZE >> 1;
  vdst = TILESIZE >> 1;

  for (hdst = -TILESIZE; hdst <= WIDTH; hdst += dsthinc)
  {
    img.copyfrom(img, hsrc, vsrc, hsrc+tmax, vsrc+tmax, hdst, vdst);

    hsrc += srchinc;
    vsrc += srcvinc;
    vdst += dstvinc;
  }
}



//
// Main Program

int main(void)
{
  gfxplane img(WIDTH, HEIGHT, MAXVAL);
  gfxplane charimg(TILESIZE, TILESIZE, MAXVAL);
  gfxplane srcimg(WIDTH, HEIGHT, MAXVAL);


  // Rectangle drawing tests.
  make_rectangle_test(img);
  write_ppm(img, "output/rectdraw.ppm");


  // A glyph to use with copying tests.
  make_glyph(charimg);
  write_ppm_pgm(charimg, "output/rectglyph.ppm", "output/rectglyph.pgm");
  write_pam(charimg, "output/rectglyph.pam");

  // Sprite copying tests, from the glyph image.
  // This writes to multiple files.
  do_glyph_test(img, charimg, "output/rectcopy");


  // Self-copying tests.
  make_self_copy_test(img);
  write_ppm(img, "output/rectcopyself.ppm");


  // Report success.
  return 0;
}



//
// This is the end of the file.
