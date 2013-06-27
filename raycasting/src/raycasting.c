#include "raycasting.h"
#include "xmalloc.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0

#if DEBUG
#  define DISASSEMBLE(n) (n / MULTIPLIER), (n % MULTIPLIER)
#  define PRIdPNT "%ld.%06ld"
#  define dprintf(fmt, args...) fprintf (stderr, fmt, args)
#else
#  define dprintf(fmt, args...)
#endif

typedef long coord;

struct point
{
  coord x;
  coord y;
};

struct raycasting_ctx
{
  size_t pointc;
  struct point pointv[];
};


rc_ctx *
rc_new (size_t intc, long const *intv)
{
  size_t i;
  struct raycasting_ctx *self = xmalloc (sizeof *self + sizeof *self->pointv * (intc / 2) + 1);

  assert (intv != NULL);
  assert (intc % 2 == 0);

  // every point consists of 2 coordinates
  self->pointc = intc / 2;

  // converts the raw integers into point structs
  for (i = 0; i < intc / 2; i++)
    {
      assert (intv[i * 2 + 0] < COORD_MAX);
      assert (intv[i * 2 + 1] < COORD_MAX);
      self->pointv[i].x = intv[i * 2 + 0] * MULTIPLIER;
      self->pointv[i].y = intv[i * 2 + 1] * MULTIPLIER;
    }

  // the last point of the polygon is equal to the first (to close the polygon)
  self->pointv[i] = self->pointv[0];

  return self;
}

void
rc_delete (rc_ctx *self)
{
  xfree (self);
}


bool
rc_contains (rc_ctx const *self, long const px, long const py)
{
  size_t i;
  struct point const *const pointv = self->pointv;
  size_t const pointc = self->pointc;
  bool inside = false;

  struct point const P = { px * MULTIPLIER, py * MULTIPLIER };

#if DEBUG
  dprintf ("P = ["PRIdPNT","PRIdPNT"]\n", DISASSEMBLE (P.x), DISASSEMBLE (P.y));
#endif

  for (i = 0; i < pointc; i++)
    {
      struct point const P1 = pointv[i + 0];
      struct point const P2 = pointv[i + 1];

#if DEBUG
      dprintf ("["PRIdPNT","PRIdPNT"] -> ["PRIdPNT","PRIdPNT"]\n",
               DISASSEMBLE (P1.x), DISASSEMBLE (P1.y),
               DISASSEMBLE (P2.x), DISASSEMBLE (P2.y));
#endif

      // point P is exactly on a polygon point
      if (P.x == P1.x && P.y == P1.y)
        return true;

      if (P.x < P1.x && P.x < P2.x)
        {
          // both x coordinates are right of P's => definitely no intersection
#if DEBUG
          dprintf ("both x coordinates ("PRIdPNT"; "PRIdPNT") are right of P's ("PRIdPNT") => definitely no intersection\n",
                   DISASSEMBLE (P1.x), DISASSEMBLE (P2.x), DISASSEMBLE (P.x));
#endif
        }
      else if (P.y == P1.y && P.y == P2.y &&
               P1.x > P.x != P2.x > P.x)
        {
          // P is on horizontal edge
#if DEBUG
          dprintf ("P is on horizontal edge\n");
#endif
          return true;
        }
      else if (P.y > P1.y == P.y > P2.y)
        {
          // both y coordinates are above or below P's => definitely no intersection
#if DEBUG
          dprintf ("both y coordinates ("PRIdPNT"; "PRIdPNT") are above or below P's ("PRIdPNT") => definitely no intersection\n",
                   DISASSEMBLE (P1.y), DISASSEMBLE (P2.y), DISASSEMBLE (P.y));
#endif
        }
      else
        {
          // the point of the supposed intersection
          struct point const Q = { P1.x, P.y };
          // X distance of the current edge
          coord const dx_P = P2.x - P1.x;
          // Y distance of the current edge
          coord const dy_P = P2.y - P1.y;
          // Y distance between the supposed intersection and P1 (first point of the edge) 
          coord const dy_Q = Q.y - P1.y;
          // the ratio of delta_y Q and delta_y P
          double const ratio = (double)dy_Q / (double)dy_P;
          // we use the ratio to construct the point on which the two lines may intersect
          struct point const R = { P1.x + ratio * dx_P, Q.y };

          if (R.x == P.x)
            return true;

#if DEBUG
          dprintf ("R = ["PRIdPNT","PRIdPNT"] => %sintersection\n", DISASSEMBLE (R.x), DISASSEMBLE (R.y), R.x < P.x ? "" : "no ");
#endif
          // if the point of interest is on the left of the supposed intersection
          // there *is* no intersection as we've already established the P.O.I.
          // has to be on the left of it (see P.x < P1.x && P.x < P2.x) 
          if (R.x < P.x)
            inside = !inside;
        }
    }

  return inside;
}
