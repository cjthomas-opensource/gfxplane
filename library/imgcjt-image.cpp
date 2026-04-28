// Simple image rendering and I/O library - Image and drawing primitives.
// Written by Christopher Thomas.
// This is a quick and dirty library for 2D graphics and PPM/PGM/PAM file I/O.

//
// Includes

#include "imgcjt.h"

using namespace imgcjt;



//
// Private Macros

#define BLEND_YES true
#define BLEND_NO false
#define AA_YES true
#define AA_NO false



//
// ARGB pixel manipulation functions.


uint64_t imgcjt::pix_from_argb_components(
  uint16_t a, uint16_t r, uint16_t g, uint16_t b)
{
  uint64_t pixval, scratch;

  pixval = 0;

  scratch = a;
  pixval |= scratch << 48;

  scratch = r;
  pixval |= scratch << 32;

  scratch = g;
  pixval |= scratch << 16;

  scratch = b;
  pixval |= scratch;

  return pixval;
}



void imgcjt::argb_components_from_pix(uint64_t pixval,
  uint16_t &a, uint16_t &r, uint16_t &g, uint16_t &b)
{
  uint64_t scratch;

  scratch = (pixval >> 48) & 0xffff;
  a = (uint16_t) scratch;

  scratch = (pixval >> 32) & 0xffff;
  r = (uint16_t) scratch;

  scratch = (pixval >> 16) & 0xffff;
  g = (uint16_t) scratch;

  scratch = pixval & 0xffff;
  b = (uint16_t) scratch;
}



// These are used when processing indexed colourmaps, among other things.
// The idea is to keep implementation details opaque while avoiding full
// component extraction/packing.

uint64_t imgcjt::pix_merge_alpha_rgb(uint64_t alpha_src, uint64_t rgb_src)
{
  uint64_t alpha_mask;

  alpha_mask = 0xffff;
  alpha_mask <<= 48;

  return (alpha_src & alpha_mask) | (rgb_src & (~alpha_mask));
}



void imgcjt::pix_split_alpha_rgb(uint64_t pixval,
  uint64_t &alpha_part, uint64_t &rgb_part)
{
  uint64_t alpha_mask;

  alpha_mask = 0xffff;
  alpha_mask <<= 48;

  alpha_part = pixval & alpha_mask;
  rgb_part = pixval & (~alpha_mask);
}



// NOTE - By convention, alpha 0 is transparent and alpha 1 is opaque.
// We need to explicitly specify the maximum value for alpha.

uint64_t imgcjt::pix_alpha_blend(
  uint64_t pixover, uint64_t pixunder, uint16_t maxval)
{
  uint64_t a1, r1, g1, b1, a2, r2, g2, b2, aout, rout, gout, bout, max64;
  uint64_t undercoeff;
  uint16_t atemp, rtemp, gtemp, btemp;
  uint64_t pixout;

  // Promote everything to 64-bit. In theory 32-bit is enough, but I don't
  // want to find out the hard way that there's an edge case, and we should
  // have 64-bit native math on all processors these days anyways.

  argb_components_from_pix(pixover, atemp, rtemp, gtemp, btemp);
  a1 = atemp;
  r1 = rtemp;
  g1 = gtemp;
  b1 = btemp;

  argb_components_from_pix(pixunder, atemp, rtemp, gtemp, btemp);
  a2 = atemp;
  r2 = rtemp;
  g2 = gtemp;
  b2 = btemp;

  max64 = maxval;

  // Formulae:
  //   Aout = A1 + A2 (1 - A1)
  //   cout = [ c1 A1 + c2 A2 (1 - A1) ] / Aout
  //
  // Remember A = a/maxval, or a = A * maxval.
  // So:
  //
  // Aout * maxval = A1 * maxval + A2 * maxval * (maxval - A1 * maxval) / max
  // aout = a1 + a2 * (maxval - a1) / maxval
  //
  // cout = [ c1 A1*max + c2 A2*max*(max - A1*max)/max ] / Aout*max
  // cout = [ c1 a1 + c2 a2 (maxval - a1)/maxval ] / aout


  // Precompute "a2 * (maxval - a1) / maxval".
  // We'll add half maxval to turn truncation into rounding. The downside
  // is that we'll have to clamp the computed values afterward.

  undercoeff = a2 * (max64 - a1);
  undercoeff = (undercoeff + (max64 >> 1)) / max64;

  // Compute the output alpha and component values.

  aout = a1 + undercoeff;
  if (aout > max64) aout = max64;

  // If aout is zero, it means a1 and a2 are both zero (fully transparent).
  // Copy the bottom colour in that circumstance.
  rout = r2;
  gout = g2;
  bout = b2;
  if (aout > 0)
  {
    rout = (r1 * a1 + r2 * undercoeff) / aout;
    gout = (g1 * a1 + g2 * undercoeff) / aout;
    bout = (b1 * a1 + b2 * undercoeff) / aout;
  }

  if (rout > max64) rout = max64;
  if (gout > max64) gout = max64;
  if (bout > max64) bout = max64;

  // Since all of these were clamped at maxval, they all are 16-bit compatible.
  atemp = (uint16_t) aout;
  rtemp = (uint16_t) rout;
  gtemp = (uint16_t) gout;
  btemp = (uint16_t) bout;

  pixout = pix_from_argb_components(atemp, rtemp, gtemp, btemp);

  return pixout;
}



//
// Class methods.
// ARGB graphics plane with 16-bit components.
// Adequate for just about all of my use-cases.
//


//
// Constructor and destructor.

imgcjt::gfxplane::gfxplane(long new_width, long new_height,
  uint16_t new_maxval)
{
  uint64_t pixcount;

  width = new_width;
  height = new_height;
  maxval = new_maxval;

  // Force sanity.
  if (width < 1) width = 1;
  if (height < 1) height = 1;
  if (maxval < 1) maxval = 1;

  // Store a uint64_t version for array indexing.
  pitch = width;

  // Allocate the frame buffer.
  pixcount = height;
  pixcount *= pitch;
  pixdata.resize(pixcount, defaultpixval);
}



// Default destructor is fine.



//
// Accessors.

long imgcjt::gfxplane::get_width(void)
{ return width; }

long imgcjt::gfxplane::get_height(void)
{ return height; }

uint16_t imgcjt::gfxplane::get_maxval(void)
{ return maxval; }



//
// Miscellaneous.

// All blending operations assume consistent maxval values.
// The user can call this to rescale an image's components if necessary.

void imgcjt::gfxplane::rescale_maxval(uint16_t new_maxval)
{
  std::vector<uint16_t> lut_old_to_new;
  uint32_t thisval, oldmax32, newmax32;
  long h, v, midx;
  uint16_t a, r, g, b;

  // Promote to 32-bit.
  // In the worst case, we're computing (maxval+1)*(maxval)+(maxval>>1)
  // for 0xffff. That gives 0xffff7fff.
  oldmax32 = maxval;
  newmax32 = new_maxval;

  // Build a lookup table so that we aren't doing a division for every
  // pixel.

  lut_old_to_new.resize(oldmax32, 0);

  for (midx = 0; midx <= oldmax32; midx++)
  {
    thisval = midx;

    if (newmax32 <= oldmax32)
    {
      // Multiple input values per output value.
      // Make each output bin have the same number of inputs.
      thisval *= newmax32 + 1;
      thisval /= oldmax32 + 1; // Truncate downward.
    }
    else
    {
      // Multiple possible output values per input value.
      // Force the extreme values to map to each other.
      thisval *= newmax32;
      thisval += oldmax32 >> 1; // Round rather than truncate.
      thisval /= oldmax32;
    }

    // Clamp just in case there's an edge case.
    if (thisval > newmax32)
      thisval = newmax32;

    lut_old_to_new[midx] = thisval;
  }

  // Remap image components.

  a = 0;
  r = 0;
  g = 0;
  b = 0;

  for (v = 0; v < height; v++)
    for (h = 0; h < width; h++)
    {
      getpix_argb(h, v, a, r, g, b);

      // Clamp just in case of bogus values in the array.
      if (a > maxval) a = maxval;
      if (r > maxval) r = maxval;
      if (g > maxval) g = maxval;
      if (b > maxval) b = maxval;

      a = lut_old_to_new[a];
      r = lut_old_to_new[r];
      g = lut_old_to_new[g];
      b = lut_old_to_new[b];

      // NOTE - We're counting on this not clamping to the old maxval.
      setpix_argb(h, v, a, r, g, b);
    }

  // Update metadata.
  maxval = new_maxval;
}



//
// Helper functions for drawing primitives.
// The idea is to avoid having to duplicate nontrivial rendering logic.


// Rectangle.

void imgcjt::gfxplane::helper_rect(long h1, long v1, long h2, long v2,
  uint64_t pixval, bool want_blend)
{
  long h, v, scratchlong;
  uint64_t rowstart, oset, scratch64;
  uint64_t thispix;

  // Force ordering.

  if (h1 > h2)
  {
    scratchlong = h1;
    h1 = h2;
    h2 = scratchlong;
  }

  if (v1 > v2)
  {
    scratchlong = v1;
    v1 = v2;
    v2 = scratchlong;
  }

  // Cull if the entire rectangle is outside the image.
  // We know that the coordinates are sorted, by this point.
  if ( (h1 >= width) || (h2 < 0) || (v1 >= height) || (v2 < 0) )
    return;

  // Crop to the image boundary.
  // We know that the coordinates are sorted and overlap the image.

  if (h1 < 0) h1 = 0;
  if (h2 >= width) h2 = width - 1;
  if (v1 < 0) v1 = 0;
  if (v2 >= height) v2 = height - 1;


  // Render the rectangle, optionally blending.
  // NOTE - Counting on optimization to pull the "if" statement outside
  // the loop.

  rowstart = v1;
  rowstart *= pitch;
  scratch64 = h1;
  rowstart += scratch64;

  for (v = v1; v <= v2; v++)
  {
    oset = rowstart;
    for (h = h1; h <= h2; h++)
    {
      if (want_blend)
      {
        thispix = pixdata[oset];
        thispix = pix_alpha_blend(pixval, thispix, maxval);
        pixdata[oset] = thispix;
      }
      else
        pixdata[oset] = pixval;

      oset++;
    }
    rowstart += pitch;
  }
}



// Line.

void imgcjt::gfxplane::helper_line(long h1, long v1, long h2, long v2,
  uint64_t pixval, bool want_blend, bool want_aa)
{
  // NOTE - Using an implementation that's as simple and clean as possible.
  // This means it will be slower than an optimized implementation.
  // Specifics:
  // - Treating H and V as functions of line coordinate T.
  // - Wrapping setpix to handle rendering.
  // - Drawing off-screen parts of the line instead of doing clipping.
  // - Anti-aliasing alpha is the Bresenham error (or 1-error).

  long h, dh, hinc, v, dv, vinc;
  uint64_t t, tmax, herr, verr, dh64, dv64;
  uint16_t a, r, g, b;
  uint64_t max64, acurrent, anext, newpixval, thiserr, maxerr;
  bool h_major;

  // Initialize Bresenham parameters.

  dh = h2 - h1;
  hinc = 1;
  if (0 > dh)
  {
    dh = -dh;
    hinc = -hinc;
  }

  dv = v2 - v1;
  vinc = 1;
  if (0 > dv)
  {
    dv = -dv;
    vinc = -vinc;
  }

  // dh and dv are guaranteed non-negative now.

  dh64 = dh;
  dv64 = dv;

  tmax = dh64;
  h_major = true;
  if (dv64 > tmax )
  {
    tmax = dv64;
    h_major = false;
  }


  // Initialize line-drawing.

  h = h1;
  v = v1;

  // If we're not anti-aliasing, start in the middle so that the end
  // segments are equal size.
  herr = tmax >> 1;
  verr = tmax >> 1;

  // Avoid dividing by zero even if dh = dv = 0 or 1.
  maxerr = 1;
  if (tmax >= 2)
    maxerr = tmax - 1;

  // If we're anti-aliasing, start at zero so that we only render on the
  // endpoint pixels, not past them.
  if (want_aa)
  {
    herr = 0;
    verr = 0;
  }

  // Avoid compiler warnings.
  a = 0;
  r = 0;
  g = 0;
  b = 0;

  // Cache a 64-bit version of maxval.
  max64 = maxval;


  // Draw the line.

  for (t = 0; t <= tmax; t++)
  {
    // Render this pixel.
    // The compiler should pull this if block outside the loop to optimize it.

    if (want_aa)
    {
      // Compute the anti-aliased alpha values based on the minor axis error.
      thiserr = herr;
      if (h_major)
        thiserr = verr;

      argb_components_from_pix( pixval, a, r, g, b );

      // Force opaque if we don't want blending. AA still uses blending.
      if (!want_blend)
        a = maxval;

      // We already guaranteed that maxerr is safe.
      // An error of 0 gives an alpha of maxval (opaque).

      acurrent = a;
      acurrent *= maxerr - thiserr;
      acurrent /= maxerr;

      anext = a;
      anext *= thiserr;
      anext /= maxerr;

      // Shouldn't ever be needed, but force sanity.
      if (acurrent > max64)
         acurrent = max64;
      if (anext > max64)
         acurrent = max64;


      // We're always blending when rendering, with or without source alpha.

      newpixval = pix_from_argb_components( (uint16_t) acurrent, r, g, b );
      blendpix(h, v, newpixval);

      newpixval = pix_from_argb_components( (uint16_t) anext, r, g, b );
      // Increment the minor axis.
      if (h_major)
        blendpix(h, v + vinc, newpixval);
      else
        blendpix(h + hinc, v, newpixval);
    }
    else
    {
      // No anti-aliasing.
      if (want_blend)
        blendpix(h, v, pixval);
      else
        setpix(h, v, pixval);
    }


    // Update position.

    herr += dh64;
    // Should happen 0 or 1 times. Tolerate "tmax = 0" too.
    if (herr >= tmax)
    {
      h += hinc;
      herr -= tmax;
    }

    verr += dv64;
    // Should happen 0 or 1 times. Tolerate "tmax = 0" too.
    if (verr >= tmax)
    {
      v += vinc;
      verr -= tmax;
    }
  }
}



// Copy/blend blit.

void imgcjt::gfxplane::helper_copy(gfxplane &src, long h1, long v1,
  long h2, long v2, long hdest, long vdest, bool want_blend)
{
  long scratchlong, hdest2, vdest2;
  long hidx, vidx;
  uint64_t srcrowstart, srcoset, srcpitch, dstrowstart, dstoset, scratch64;
  uint64_t srcpix, dstpix;


  // Force ordering.

  // NOTE - The target is always referenced with respect to h1,v1.
  // This is not necessarily the upper left corner.
  // So if we swap either coordinate, we need to adjust the target location.

  if (h1 > h2)
  {
    scratchlong = h1;
    h1 = h2;
    h2 = scratchlong;

    hdest -= (h2 - h1);
  }

  if (v1 > v2)
  {
    scratchlong = v1;
    v1 = v2;
    v2 = scratchlong;

    vdest -= (v2 - v1);
  }

  // We know that the coordinates are sorted, by this point.


  // NOTE - It would be much, much simpler just to iterate and call the
  // pixel-drawing routine and let it handle clipping. The problem is that
  // the pixel-drawing routine is much slower.

  // FIXME - It might still be best to do that. The clipping code may still
  // have bugs; one of them was only caught because of a typo in the test
  // program.



  // Cull if the entire rectangle is outside the source image.

  if ( (h1 >= src.width) || (h2 < 0)
    || (v1 >= src.height) || (v2 < 0) )
    return;

  // Crop to the source image boundary.
  // We know that we overlap the source image.
  // If we change h1 or v1, adjust the target location as well.

  if (h1 < 0)
  {
    hdest -= h1;
    h1 = 0;
  }

  if (v1 < 0)
  {
    vdest -= v1;
    v1 = 0;
  }

  if (h2 >= src.width) h2 = src.width - 1;
  if (v2 >= src.height) v2 = src.height - 1;



  // Compute the other corner of the destination rectangle.
  hdest2 = hdest + h2 - h1;
  vdest2 = vdest + v2 - v1;

  // Cull if the entire rectangle is outside the destination image.

  if ( (hdest >= width) || (hdest2 < 0)
    || (vdest >= height) || (vdest2 < 0) )
    return;

  // Crop to the destination image boundary.
  // If we change any coordinates, adjust the source location as well.
  // This will never swap the source location coordinates, since we have a
  // nonzero span on the destination image.

  if (hdest < 0)
  {
    h1 -= hdest;
    hdest = 0;
  }

  if (vdest < 0)
  {
    v1 -= vdest;
    vdest = 0;
  }

  if (hdest2 >= width)
  {
    h2 -= hdest2 - (width - 1);
    hdest2 = width - 1; // We don't actually use this, but keep it updated.
  }

  if (vdest2 >= height)
  {
    v2 -= vdest2 - (height - 1);
    vdest2 = height - 1; // We don't actually use this, but keep it updated.
  }


  // Cull if destination cropping moved the source outside the source image.

  if ( (h1 >= src.width) || (h2 < 0)
    || (v1 >= src.height) || (v2 < 0) )
    return;



  // Perform the copy, optionally blending.
  // NOTE - Counting on optimization to pull the "if" statement outside
  // the loop.

  srcpitch = src.pitch;

  srcrowstart = v1;
  srcrowstart *= srcpitch;
  scratch64 = h1;
  srcrowstart += scratch64;

  dstrowstart = vdest;
  dstrowstart *= pitch;
  scratch64 = hdest;
  dstrowstart += scratch64;

  for (vidx = v1; vidx <= v2; vidx++)
  {
    srcoset = srcrowstart;
    dstoset = dstrowstart;

    for (hidx = h1; hidx <= h2; hidx++)
    {
      srcpix = src.pixdata[srcoset];
      if (want_blend)
      {
        dstpix = pixdata[dstoset];
        dstpix = pix_alpha_blend(srcpix, dstpix, maxval);
        pixdata[dstoset] = dstpix;
      }
      else
        pixdata[dstoset] = srcpix;

      srcoset++;
      dstoset++;
    }

    srcrowstart += srcpitch;
    dstrowstart += pitch;
  }

  // Done.
}



//
// Pixel primitives.


void imgcjt::gfxplane::setpix(long h, long v, uint64_t pixval)
{
  uint64_t h64, v64, oset;

  if ( (h >= 0) && (h < width) && (v >= 0) && (v < height) )
  {
    h64 = h;
    v64 = v;
    oset = (v64 * pitch) + h64;

    pixdata[oset] = pixval;
  }
}



void imgcjt::gfxplane::setpix_argb(long h, long v,
    uint16_t a, uint16_t r, uint16_t g, uint16_t b)
{
  setpix( h, v, pix_from_argb_components(a, r, g, b) );
}



uint64_t imgcjt::gfxplane::getpix(long h, long v)
{
  uint64_t h64, v64, oset, pixval;

  pixval = defaultpixval;

  if ( (h >= 0) && (h < width) && (v >= 0) && (v < height) )
  {
    h64 = h;
    v64 = v;
    oset = (v64 * pitch) + h64;

    pixval = pixdata[oset];
  }

  return pixval;
}



void imgcjt::gfxplane::getpix_argb(long h, long v,
  uint16_t &a, uint16_t &r, uint16_t &g, uint16_t &b)
{
  uint64_t pixval;

  pixval = getpix(h, v);

  argb_components_from_pix(pixval, a, r, g, b);
}



void imgcjt::gfxplane::blendpix(long h, long v, uint64_t pixover)
{
  uint64_t h64, v64, oset;
  uint64_t pixval;

  if ( (h >= 0) && (h < width) && (v >= 0) && (v < height) )
  {
    h64 = h;
    v64 = v;
    oset = (v64 * pitch) + h64;

    pixval = pixdata[oset];
    pixval = pix_alpha_blend(pixover, pixval, maxval);
    pixdata[oset] = pixval;
  }
}


void imgcjt::gfxplane::blendpix_argb(long h, long v,
  uint16_t aover, uint16_t rover, uint16_t gover, uint16_t bover)
{
  blendpix( h, v, pix_from_argb_components(aover, rover, gover, bover) );
}



//
// Rectangle primitives.


void imgcjt::gfxplane::setrect(long h1, long v1, long h2, long v2,
  uint64_t pixval)
{
  helper_rect(h1, v1, h2, v2, pixval, BLEND_NO);
}



void imgcjt::gfxplane::setrect_argb(long h1, long v1, long h2, long v2,
  uint16_t a, uint16_t r, uint16_t g, uint16_t b)
{
  setrect( h1, v1, h2, v2, pix_from_argb_components(a, r, g, b) );
}



void imgcjt::gfxplane::blendrect(long h1, long v1, long h2, long v2,
  uint64_t pixover)
{
  helper_rect(h1, v1, h2, v2, pixover, BLEND_YES);
}



void imgcjt::gfxplane::blendrect_argb(long h1, long v1, long h2, long v2,
  uint16_t aover, uint16_t rover, uint16_t gover, uint16_t bover)
{
  blendrect( h1, v1, h2, v2,
    pix_from_argb_components(aover, rover, gover, bover) );
}



//
// Lines.


void imgcjt::gfxplane::setline(long h1, long v1, long h2, long v2,
  uint64_t pixval)
{
  helper_line(h1, v1, h2, v2, pixval, BLEND_NO, AA_NO);
}



void imgcjt::gfxplane::setline_argb(long h1, long v1, long h2, long v2,
  uint16_t a, uint16_t r, uint16_t g, uint16_t b)
{
  setline( h1, v1, h2, v2, pix_from_argb_components(a, r, g, b) );
}



void imgcjt::gfxplane::setline_aa(long h1, long v1, long h2, long v2,
  uint64_t pixval)
{
  helper_line(h1, v1, h2, v2, pixval, BLEND_NO, AA_YES);
}



void imgcjt::gfxplane::setline_aa_argb(long h1, long v1, long h2, long v2,
  uint16_t a, uint16_t r, uint16_t g, uint16_t b)
{
  setline_aa( h1, v1, h2, v2, pix_from_argb_components(a, r, g, b) );
}



void imgcjt::gfxplane::blendline(long h1, long v1, long h2, long v2,
  uint64_t pixover)
{
  helper_line(h1, v1, h2, v2, pixover, BLEND_YES, AA_NO);
}



void imgcjt::gfxplane::blendline_argb(long h1, long v1, long h2, long v2,
  uint16_t aover, uint16_t rover, uint16_t gover, uint16_t bover)
{
  blendline( h1, v1, h2, v2,
    pix_from_argb_components(aover, rover, gover, bover) );
}



void imgcjt::gfxplane::blendline_aa(long h1, long v1, long h2, long v2,
  uint64_t pixover)
{
  helper_line(h1, v1, h2, v2, pixover, BLEND_YES, AA_YES);
}



void imgcjt::gfxplane::blendline_aa_argb(long h1, long v1, long h2, long v2,
  uint16_t aover, uint16_t rover, uint16_t gover, uint16_t bover)
{
  blendline_aa( h1, v1, h2, v2,
    pix_from_argb_components(aover, rover, gover, bover) );
}



//
// Compositing (for sprites and such).


void imgcjt::gfxplane::copyfrom(gfxplane &src,
  long h1, long v1, long h2, long v2, long hdest, long vdest)
{
  helper_copy(src, h1, v1, h2, v2, hdest, vdest, BLEND_NO);
}



void imgcjt::gfxplane::blendfrom(gfxplane &src,
  long h1, long v1, long h2, long v2, long hdest, long vdest)
{
  helper_copy(src, h1, v1, h2, v2, hdest, vdest, BLEND_YES);
}



//
// This is the end of the file.
