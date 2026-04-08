// Simple image rendering and I/O library - File I/O functions.
// Written by Christopher Thomas.
// This is a quick and dirty library for 2D graphics and PPM/PGM/PAM file I/O.


//
// Includes

#include "imgcjt.h"

// A few other headers we need.
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>

using namespace imgcjt;



//
// Private macros

// PAM header tokens are 8 or fewer characters.
// Tuple types don't have length limits. The longest defined one is
// BLACKANDWHITE_ALPHA (19 characters). The longest we care about is
// GRAYSCALE_ALPHA (15 characters).
#define PAMTOKENLEN 40
#define PAMTOKENPATTERN " %40s"



//
// I/O Functions


// Returns true on success, false on failure. Discards alpha. ASCII.
// Writes to cerr on failure.

bool imgcjt::write_ppm(gfxplane &src, std::string filename)
{
  // FIXME - Using C, not C++, file primitives because I prefer that syntax.

  bool is_ok;
  FILE *colfile;
  long h, v, width, height;
  uint16_t a, r, g, b, maxval;

  is_ok = true;

  width = src.get_width();
  height = src.get_height();
  maxval = src.get_maxval();

  // Force defaults to avoid compile-time warnings.
  a = 0;
  r = 0;
  g = 0;
  b = 0;

  colfile = fopen(filename.c_str(), "wb");

  if (NULL == colfile)
  {
    is_ok = false;
    std::cerr << "### Unable to write to \"" << filename << "\".\n";
  }
  else
  {
    // Using ASCII for ease of inspection and to avoid endianness issues.

    fprintf(colfile, "P3\n%ld %ld\n%u\n", width, height, maxval);

    for (v = 0; v < height; v++)
      for (h = 0; h < width; h++)
      {
        src.getpix_argb(h, v, a, r, g, b);
        fprintf(colfile, "%u %u %u\n", r, g, b);
      }

    fclose(colfile);
  }

  return is_ok;
}



// Returns true on success, false on failure.
// Saves colour to a PPM file and alpha to a PGM file. ASCII.
// Writes to cerr on failure.

bool imgcjt::write_ppm_pgm(gfxplane &src,
  std::string img_filename, std::string alpha_filename)
{
  // FIXME - Using C, not C++, file primitives because I prefer that syntax.

  bool is_ok;
  FILE *alphafile;
  long h, v, width, height;
  uint16_t a, r, g, b, maxval;

  is_ok = true;

  width = src.get_width();
  height = src.get_height();
  maxval = src.get_maxval();

  // Force defaults to avoid compile-time warnings.
  a = 0;
  r = 0;
  g = 0;
  b = 0;


  // Wrap the PPM function.
  is_ok = write_ppm(src, img_filename);


  // If that succeeded, write the alpha channel to a PGM file.

  if (is_ok)
  {
    alphafile = fopen(alpha_filename.c_str(), "wb");

    if (NULL == alphafile)
    {
      is_ok = false;
      std::cerr << "### Unable to write to \"" << alpha_filename << "\".\n";
    }
  }

  if (is_ok)
  {
    // Using ASCII for ease of inspection and to avoid endianness issues.

    fprintf(alphafile, "P2\n%ld %ld\n%u\n", width, height, maxval);

    for (v = 0; v < height; v++)
      for (h = 0; h < width; h++)
      {
        src.getpix_argb(h, v, a, r, g, b);
        fprintf(alphafile, "%u\n", a);
      }

    fclose(alphafile);
  }

  return is_ok;
}



// Returns true on success, false on failure. Saves alpha. Big-endian.
// Writes to cerr on failure.

bool imgcjt::write_pam(gfxplane &src, std::string filename)
{
  // FIXME - Using C, not C++, file primitives because I prefer that syntax.

  bool is_ok;
  FILE *pamfile;
  long h, v, width, height;
  uint16_t a, r, g, b, maxval;
  bool twobytes;

  is_ok = true;

  width = src.get_width();
  height = src.get_height();
  maxval = src.get_maxval();

  twobytes = false;
  if (maxval > 0xff)
    twobytes = true;

  // Force defaults to avoid compile-time warnings.
  a = 0;
  r = 0;
  g = 0;
  b = 0;

  pamfile = fopen(filename.c_str(), "wb");

  if (NULL == pamfile)
  {
    is_ok = false;
    std::cerr << "### Unable to write to \"" << filename << "\".\n";
  }
  else
  {
    // No ASCII option.
    // Using RGB_ALPHA format.

    fprintf(pamfile,
      "P7\nWIDTH %ld\nHEIGHT %ld\nDEPTH 4\nMAXVAL %u\n"
      "TUPLTYPE RGB_ALPHA\nENDHDR\n", width, height, maxval);

    for (v = 0; v < height; v++)
      for (h = 0; h < width; h++)
      {
        src.getpix_argb(h, v, a, r, g, b);

        // This is slow, but simple.

        if (twobytes)
        {
          // Big-endian format.
          fputc((r >> 8) & 0xff, pamfile);
          fputc(r & 0xff, pamfile);
          fputc((g >> 8) & 0xff, pamfile);
          fputc(g & 0xff, pamfile);
          fputc((b >> 8) & 0xff, pamfile);
          fputc(b & 0xff, pamfile);
          fputc((a >> 8) & 0xff, pamfile);
          fputc(a & 0xff, pamfile);
        }
        else
        {
          fputc(r & 0xff, pamfile);
          fputc(g & 0xff, pamfile);
          fputc(b & 0xff, pamfile);
          fputc(a & 0xff, pamfile);
        }
      }

    fclose(pamfile);
  }

  return is_ok;
}



// Returns an object on success, NULL on failure.
// Writes to cerr on failure.
// This reads P2 or P5 PGM, P3 or P6 PPM, or P7 PAM (with formats GRAYSCALE,
// GRAYSCALE_ALPHA, RGB, or RGB_ALPHA).
// Generates an opaque alpha channel if none is provided.
// For binary PGM, PPM, or PAM, assumes big-endian format.
// Writes to cerr on failure.

gfxplane *imgcjt::read_ppm(std::string filename)
{
  // FIXME - Using C, not C++, file primitives because I prefer that syntax.

  gfxplane *newimg;
  bool is_ok;
  FILE *pfile;
  char c1, c2;
  bool is_ascii, is_16bit, is_grey, has_alpha, is_pam;
  long new_width, new_height, new_maxval, new_depth;
  int numread;
  char headertoken[PAMTOKENLEN+1], firstchar, tupletype[PAMTOKENLEN+1];
  bool headerdone;
  long h, v;
  int r1, g1, b1, a1, r2, g2, b2, a2;

  newimg = NULL;
  is_ok = true;


  // Try to open the file.

  pfile = fopen(filename.c_str(), "rb");
  if (NULL == pfile)
  {
    is_ok = false;
    std::cerr << "### Unable to read from \"" << filename << "\".\n";
  }


  // Look for the magic identifier.

  is_ascii = false;
  is_grey = false;
  is_pam = false;

  if (is_ok)
  {
    c1 = fgetc(pfile);
    c2 = fgetc(pfile);

    if (c1 != 'P')
      is_ok = false;
    else
    {
      switch (c2)
      {
        case '2':
          is_ascii = true;
          is_grey = true;
          break;

        case '3':
          is_ascii = true;
          is_grey = false;
          break;

        case '5':
          is_ascii = false;
          is_grey = true;
          break;

        case '6':
          is_ascii = false;
          is_grey = false;
          break;

        case '7':
          is_pam = true;
          is_ascii = false;
          // Greyscale and alpha are determined by TUPLTYPE.
          break;

        default:
          is_ok = false;
          break;
      }
    }

    if (!is_ok)
      std::cerr << "### Unable to parse magic identifier in \""
        << filename << "\".\n";
  }


  // Try to read the header metadata.
  // FIXME - Not handling comments properly!

  new_width = 0;
  new_height = 0;
  new_maxval = 0;
  has_alpha = false;
  is_16bit = false;

  if (is_ok)
  {
    if (!is_pam)
    {
      // Start of PGM/PPM header read.

      numread = fscanf(pfile, " %ld %ld %ld",
        &new_width, &new_height, &new_maxval);

      if (3 != numread)
      {
        is_ok = false;
        std::cerr << "### Unable to read PGM/PPM header metadata from \""
          << filename << "\".\n";
      }

      // Skip the trailing newline character in binary mode.
      if (is_ok && (!is_ascii))
      {
        c1 = fgetc(pfile);

        if (EOF == c1)
        {
          is_ok = false;
          std::cerr << "### Unexpected end-of-file after PGM/PPM header in \""
            << filename << "\".\n";
        }
      }

      // End of PGM/PPM header read.
    }
    else
    {
      // Start of PAM header read.

      new_depth = 0;
      // Remember that we have PAMTOKENLEN+1 characters in the array.
      for (h = 0; h <= PAMTOKENLEN; h++)
        tupletype[h] = 0;

      headerdone = false;
      while ( is_ok && (!headerdone) )
      {
        // FIXME - The right thing to do is to use getline() and then
        // parse each line. Instead, parse tokens.

        // Remember that we have PAMTOKENLEN+1 characters in the array.
        for (h = 0; h <= PAMTOKENLEN; h++)
          headertoken[h] = 0;

        numread = fscanf(pfile, PAMTOKENPATTERN, headertoken);
        // Should already be handled but do it anyways.
        headertoken[PAMTOKENLEN] = 0;

        if (numread != 1)
        {
          std::cerr << "### Unexpected end of file reading token in \""
            << filename << "\".\n";
          is_ok = false;
        }
        else
        {
          firstchar = headertoken[0];
          switch (firstchar)
          {
            case 'W': // Assume WIDTH.
              numread = fscanf(pfile, " %ld", &new_width);
              if (numread != 1)
              {
                std::cerr << "### Unexpected end of file reading width in \""
                  << filename << "\".\n";
                is_ok = false;
              }
              break;

            case 'H': // Assume HEIGHT.
              numread = fscanf(pfile, " %ld", &new_height);
              if (numread != 1)
              {
                std::cerr << "### Unexpected end of file reading height in \""
                  << filename << "\".\n";
                is_ok = false;
              }
              break;

            case 'D': // Assume DEPTH.
              numread = fscanf(pfile, " %ld", &new_depth);
              if (numread != 1)
              {
                std::cerr << "### Unexpected end of file reading depth in \""
                  << filename << "\".\n";
                is_ok = false;
              }
              break;

            case 'M': // Assume MAXVAL.
              numread = fscanf(pfile, " %ld", &new_maxval);
              if (numread != 1)
              {
                std::cerr << "### Unexpected end of file reading maxval in \""
                  << filename << "\".\n";
                is_ok = false;
              }
              break;

            case 'T': // Assume TUPLTYPE.
              // Remember that we have PAMTOKENLEN+1 characters in the array.
              for (h = 0; h <= PAMTOKENLEN; h++)
                tupletype[h] = 0;

              numread = fscanf(pfile, PAMTOKENPATTERN, tupletype);
              // Should already be handled but do it anyways.
              tupletype[PAMTOKENLEN] = 0;

              if (numread != 1)
              {
                std::cerr << "### Unexpected end of file reading"
                  " tuple type in \"" << filename << "\".\n";
                is_ok = false;
              }
              break;

            case 'E': // Assume ENDHDR.
              headerdone = true;
              break;

            default:
              std::cerr << "### Unknown header token \"" << headertoken
                << "\" in \"" << filename << "\".\n";
              is_ok = false;
              break;
          }
        }
      }

      // In theory, we have all of the PAM metadata now.

      // Convert tuple type to flags.
      // Allow "GREY" as a variant of "GRAY".

      if (is_ok)
      {
        if (0 == strcmp(tupletype, "RGB"))
        {
          is_grey = false;
          has_alpha = false;
        }
        else if (0 == strcmp(tupletype, "RGB_ALPHA"))
        {
          is_grey = false;
          has_alpha = true;
        }
        else if ( (0 == strcmp(tupletype, "GRAYSCALE"))
          || (0 == strcmp(tupletype, "GREYSCALE")) )
        {
          is_grey = true;
          has_alpha = false;
        }
        else if ( (0 == strcmp(tupletype, "GRAYSCALE_ALPHA"))
          || (0 == strcmp(tupletype, "GREYSCALE_ALPHA")) )
        {
          is_grey = true;
          has_alpha = true;
        }
        else
        {
          is_ok = false;
          std::cerr << "### Unknown tuple type \"" << tupletype
            << "\" in PAM file \"" << filename << "\".\n";
        }
      }

      // Check depth against flags.
      if (is_ok)
      {
        if ( (is_grey && (!has_alpha) && (new_depth != 1))
          || (is_grey && has_alpha && (new_depth != 2))
          || ((!is_grey) && (!has_alpha) && (new_depth != 3))
          || ((!is_grey) && (has_alpha) && (new_depth != 4)) )
        {
          is_ok = false;
          std::cerr << "### Tuple type \"" << tupletype
            << "\" had unexpected channel count (" << new_depth
            << ") in PAM file \"" << filename << "\".\n";
        }
      }

      // Skip the trailing newline character.

      if (is_ok)
      {
        c1 = fgetc(pfile);

        if (EOF == c1)
        {
          is_ok = false;
          std::cerr << "### Unexpected end-of-file after PAM header in \""
            << filename << "\".\n";
        }
      }

      // End of PAM header read.
    }
  }


  // Sanity-check the metadata.

  if (is_ok)
  {
    if ( (1 > new_width) || (1 > new_height)
      || (1 > new_maxval) || (0xffff < new_maxval) )
    {
      is_ok = false;
      std::cerr << "### Bad image metadata specified in \"" << filename
        << "\" (" << new_width << " by " << new_height
        << ", max " << new_maxval << ").\n";
    }
  }


  // Finish processing the metadata and allocate an image.

  if (is_ok)
  {
    // Metadata looks okay.
    // Finish processing it and allocate an image.

    // This only matters for binary-formatted data, but always compute it.
    is_16bit = false;
    if (new_maxval > 0xff)
      is_16bit = true;

    newimg = new gfxplane(new_width, new_height, (uint16_t) new_maxval);
    if (NULL == newimg)
    {
      is_ok = false;
      std::cerr << "### Unable to allocate memory for image while reading \""
        << filename << "\".\n";
    }
  }


  // Read the raster data.

  if (is_ok)
  {
    for (v = 0; is_ok && (v < new_height); v++)
      for (h = 0; is_ok && (h < new_width); h++)
      {
        r2 = 0;
        r1 = 0;
        g2 = 0;
        g1 = 0;
        b2 = 0;
        b1 = 0;
        a2 = 0;
        a1 = 0;


        // Read components for this pixel.

        if (is_ascii)
        {
          numread = fscanf(pfile, " %d", &r1);
          if ( (numread != 1) || (r1 < 0) || (r1 > new_maxval) )
            is_ok = false;

          if (is_ok && (!is_grey))
          {
            numread = fscanf(pfile, " %d %d", &g1, &b1);
            if ( (numread != 2) || (g1 < 0) || (g1 > new_maxval)
              || (b1 < 0) || (b1 > new_maxval) )
              is_ok = false;
          }

          // Shouldn't happen, since PAM is never ASCII, but do it anyways.
          if (is_ok && has_alpha)
          {
            numread = fscanf(pfile, " %d", &a1);
            if ( (numread != 1) || (a1 < 0) || (a1 > new_maxval) )
              is_ok = false;
          }
        }
        else
        {
          // The NetPBM standard specifies big-endian format.

          if (is_16bit)
            r2 = fgetc(pfile);
          r1 = fgetc(pfile);

          if (!is_grey)
          {
            if (is_16bit)
              g2 = fgetc(pfile);
            g1 = fgetc(pfile);
            if (is_16bit)
              b2 = fgetc(pfile);
            b1 = fgetc(pfile);
          }

          if (has_alpha)
          {
            if (is_16bit)
              a2 = fgetc(pfile);
            a1 = fgetc(pfile);
          }

          // Anything we didn't read was initialized to 0 earlier.
          if ( (r1 < 0) || (r1 > 0xff) || (r2 < 0) || (r2 > 0xff)
            || (g1 < 0) || (g1 > 0xff) || (g2 < 0) || (g2 > 0xff)
            || (b1 < 0) || (b1 > 0xff) || (b2 < 0) || (b2 > 0xff)
            || (a1 < 0) || (a1 > 0xff) || (a2 < 0) || (a2 > 0xff) )
            is_ok = false;

          if (is_ok)
          {
            // Compute 16-bit values.
            r1 += (r2 << 8);
            g1 += (g2 << 8);
            b1 += (b2 << 8);
            a1 += (a2 << 8);
          }
        }


        // Fill in missing components.

        if (is_ok && is_grey)
        {
          g1 = r1;
          b1 = r1;
        }

        if (is_ok && (!has_alpha))
          // Maxval means "opaque".
          a1 = new_maxval;


        // Store this pixel.

        if (is_ok)
          newimg->setpix_argb(h, v,
            (uint16_t) a1, (uint16_t) r1, (uint16_t) g1, (uint16_t) b1);


        // Complain if we had errors.
        if (!is_ok)
          std::cerr << "### Unable to read pixel data for \""
            << filename << "\".\n";
      }
  }


  // Clean up: Close the file if it's open.
  if (NULL != pfile)
    fclose(pfile);

  // Clean up: Discard the image if we ran into problems.
  if ((NULL != newimg) && (!is_ok))
  {
    delete newimg;
    newimg = NULL;
  }


  // Done.
  return newimg;
}



// As above but takes colour from the first file and alpha from the second.
// Intended to be used with PPM as the first file and PGM as the second.

gfxplane *imgcjt::read_ppm(std::string img_filename,
  std::string alpha_filename)
{
  gfxplane *colimg, *alphaimg;
  bool is_ok;
  long h, v, width, height;
  uint16_t a, r, g, b, atemp;

  // Try to read both images, and check that they're compatible.

  is_ok = false;

  colimg = read_ppm(img_filename);
  alphaimg = NULL;

  if (NULL != colimg)
  {
    alphaimg = read_ppm(alpha_filename);

    if (NULL != alphaimg)
    {
      if ( ( colimg->get_width() == alphaimg->get_width() )
        && ( colimg->get_height() == alphaimg->get_height() )
        && ( colimg->get_maxval() == alphaimg->get_maxval() ) )
        is_ok = true;
    }
  }

  // If we failed, release any allocated memory.

  if (!is_ok)
  {
    if (NULL != colimg)
      delete colimg;
    colimg = NULL;

    if (NULL != alphaimg)
      delete alphaimg;
    alphaimg = NULL;
  }

  // If we succeeded, copy the green channel from the second image to the
  // alpha channel of the first image.

  if (is_ok)
  {
    // Set defaults to avoid compile-time warnings.
    a = 0;
    r = 0;
    g = 0;
    b = 0;

    // Copy the alpha channel.
    // Using accessors is slow, but keeps implementation opaque and
    // avoids "oops" potential with bare pointers and pixel bit-fiddling.
    width = colimg->get_width();
    height = colimg->get_height();
    for (v = 0; v < height; v++)
      for (h = 0; h < width; h++)
      {
        alphaimg->getpix_argb(h, v, a, r, g, b);
        atemp = g; // Alpha from green.
        colimg->getpix_argb(h, v, a, r, g, b);
        // Overwrite the old alpha value.
        colimg->setpix_argb(h, v, atemp, r, g, b);
      }

    // Release the alpha image.
    // We're only returning the colour image (with added alpha).
    delete alphaimg;
    alphaimg = NULL;
  }

  return colimg;
}



//
// This is the end of the file.
