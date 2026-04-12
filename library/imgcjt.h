// Simple image rendering and I/O library.
// Written by Christopher Thomas.
// This is a quick and dirty library for 2D graphics and PPM/PGM/PAM file I/O.


// Wrapper
#ifndef IMGCJTINCLUDE
#define IMGCJTINCLUDE


//
// Required Includes

// These have wrappers, so we're okay if the user already included them.
#include <string>
#include <vector>


//
// Namespace
namespace imgcjt
{


//
// Constants

// This is "transparent black" for any bit depth.
const uint64_t defaultpixval = 0;


//
// Pixel Functions


// ARGB pixel manipulation functions.

// The "right" way to do this is to make a class, but that will slow
// things down a lot.

uint64_t pix_from_argb_components(
  uint16_t a, uint16_t r, uint16_t g, uint16_t b);
void argb_components_from_pix(uint64_t pixval,
  uint16_t &a, uint16_t &r, uint16_t &g, uint16_t &b);

// NOTE - By convention, alpha 0 is transparent and alpha 1 is opaque.
// We need to explicitly specify the maximum value for alpha.
uint64_t pix_alpha_blend(uint64_t pixover, uint64_t pixunder, uint16_t maxval);



//
// Classes


// ARGB graphics plane with 16-bit components.
// Adequate for just about all of my use-cases.

class gfxplane
{
protected:
  // Geometry.
  long width, height;
  uint64_t pitch;
  uint16_t maxval;

  // Frame buffer.
  // Do this the C++ way and hope it's adequately fast.
  std::vector<uint64_t> pixdata;

  // Helper functions for drawing primitives.
  // The idea is to avoid having to duplicate nontrivial rendering logic.
  void helper_rect(long h1, long v1, long h2, long v2,
    uint64_t pixval, bool want_blend);
  // FIXME - Add a "want anti-alias" flag here at some point.
  void helper_line(long h1, long v1, long h2, long v2,
    uint64_t pixval, bool want_blend);
  void helper_copy(gfxplane &src, long h1, long v1, long h2, long v2,
    long hdest, long vdest, bool want_blend);

public:
  // Constructor and destructor.
  gfxplane(long new_width, long new_height, uint16_t new_maxval);
  // Default destructor is fine.

  // Accessors.
  long get_width(void);
  long get_height(void);
  uint16_t get_maxval(void);


  // Miscellaneous.
  void rescale_maxval(uint16_t new_maxval);


  // Drawing primitives.

  // Pixels.

  void setpix(long h, long v, uint64_t pixval);
  void setpix_argb(long h, long v,
    uint16_t a, uint16_t r, uint16_t g, uint16_t b);

  uint64_t getpix(long h, long v);
  void getpix_argb(long h, long v,
    uint16_t &a, uint16_t &r, uint16_t &g, uint16_t &b);

  void blendpix(long h, long v, uint64_t pixover);
  void blendpix_argb(long h, long v,
    uint16_t aover, uint16_t rover, uint16_t gover, uint16_t bover);

  // Rectangles.

  void setrect(long h1, long v1, long h2, long v2, uint64_t pixval);
  void setrect_argb(long h1, long v1, long h2, long v2,
    uint16_t a, uint16_t r, uint16_t g, uint16_t b);

  void blendrect(long h1, long v1, long h2, long v2, uint64_t pixover);
  void blendrect_argb(long h1, long v1, long h2, long v2,
    uint16_t aover, uint16_t rover, uint16_t gover, uint16_t bover);

  // Lines.
  // FIXME - Add AA line variants at some point.

  void setline(long h1, long v1, long h2, long v2, uint64_t pixval);
  void setline_argb(long h1, long v1, long h2, long v2,
    uint16_t a, uint16_t r, uint16_t g, uint16_t b);

  void blendline(long h1, long v1, long h2, long v2, uint64_t pixover);
  void blendline_argb(long h1, long v1, long h2, long v2,
    uint16_t aover, uint16_t rover, uint16_t gover, uint16_t bover);

  // Compositing (for sprites and such).

  void copyfrom(gfxplane &src,
    long h1, long v1, long h2, long v2, long hdest, long vdest);
  void blendfrom(gfxplane &src,
    long h1, long v1, long h2, long v2, long hdest, long vdest);
};



//
// I/O Functions


// Returns true on success, false on failure. Discards alpha. ASCII.
// Writes to cerr on failure.
bool write_ppm(gfxplane &src, std::string filename);

// Returns true on success, false on failure.
// Saves colour to a PPM file and alpha to a PGM file. ASCII.
// Writes to cerr on failure.
bool write_ppm_pgm(gfxplane &src,
  std::string img_filename, std::string alpha_filename);

// Returns true on success, false on failure. Saves alpha. Big-endian.
// Writes to cerr on failure.
bool write_pam(gfxplane &src, std::string filename);

// Returns an object on success, NULL on failure.
// Writes to cerr on failure.
// This reads P2 or P5 PGM, P3 or P6 PPM, or P7 PAM (with formats GRAYSCALE,
// GRAYSCALE_ALPHA, RGB, or RGB_ALPHA).
// Generates an opaque alpha channel if none is provided.
// For binary PGM, PPM, or PAM, assumes big-endian format.
// Writes to cerr on failure.
gfxplane *read_ppm(std::string filename);

// As above but takes colour from the first file and alpha from the second.
// Intended to be used with PPM as the first file and PGM as the second.
gfxplane *read_ppm(std::string img_filename, std::string alpha_filename);



// Namespace
}

// Wrapper
#endif


//
// This is the end of the file.
