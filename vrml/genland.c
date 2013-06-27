#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "land.h"

static float const*
elev2colour (float elev)
{
  static float colour[3];
  static int const gradient[] = {
    0x0000dd,
    0x0000ff,
    0x51301c,
    0x516637,
    0x4e9c55,
    0x247030,
    0x0f591d,
    0xffffff,
    0xffffff,
    0xffffff,
  };

  int const index = elev * (sizeof gradient / sizeof gradient[0]);
  colour[0] = (float)((gradient[index] & 0xff0000) >> 8 * 2) / 0xff;
  colour[1] = (float)((gradient[index] & 0x00ff00) >> 8 * 1) / 0xff;
  colour[2] = (float)((gradient[index] & 0x0000ff) >> 8 * 0) / 0xff;
  return colour;
}

int
main (void)
{
  unsigned int max_elev;
  char const* data;

  printf ("#VRML V2.0 utf8\n");
  printf ("Shape {\n");
  printf ("   appearance Appearance {\n");
  printf ("      material Material { }\n");
  printf ("   }\n");
  printf ("   geometry ElevationGrid {\n");
  printf ("      xDimension %d\n", width);
  printf ("      xSpacing 8\n");
  printf ("      zDimension %d\n", height);
  printf ("      zSpacing 8\n");
  printf ("      height [ ");
  data = header_data;
  while (*data)
    {
      unsigned int elev;
      unsigned char pixel[4];
      HEADER_PIXEL (data, pixel);
      elev = pixel[0] + pixel[1] + pixel[2] + pixel[3];
      printf ("%u, ", elev / 2);

      if (elev > max_elev)
        max_elev = elev;
    }
  printf ("      ]\n");
  printf ("      color Color {\n");
  printf ("         color [\n");
  data = header_data;
  while (*data)
    {
      float const* colour;
      float elev;
      unsigned char pixel[4];
      HEADER_PIXEL (data, pixel);
      elev = pixel[0] + pixel[1] + pixel[2] + pixel[3];
      colour = elev2colour (elev / max_elev);
      assert (colour[0] <= 1);
      assert (colour[1] <= 1);
      assert (colour[2] <= 1);
      printf ("%f %f %f, ", colour[0], colour[1], colour[2]);
    }
  printf ("         ]\n");
  printf ("      }\n");
  printf ("      colorPerVertex TRUE\n");
  /*printf ("      normal NULL\n");*/
  /*printf ("      normalPerVertex TRUE\n");*/
  /*printf ("      texCoord NULL\n");*/
  /*printf ("      ccw TRUE\n");*/
  /*printf ("      convex TRUE\n");*/
  printf ("      solid TRUE\n");
  /*printf ("      creaseAngle 0.0\n");*/
  printf ("   }\n");
  printf ("}\n");

  return 0;
}
