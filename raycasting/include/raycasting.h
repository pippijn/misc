#ifndef RAYCASTING_H
#define RAYCASTING_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#define MULTIPLIER 1000000
#define COORD_MAX (LONG_MAX / MULTIPLIER)

typedef struct raycasting_ctx rc_ctx;

// initialises a new raytracing context,
// intv must be an array of intc longs less than or equal to COORD_MAX.
// intc must be even.
// the resulting context may be used for operations defined below.
/*@only@*/ rc_ctx *rc_new (size_t intc, long const *intv);

// frees all internal data structures associated with the
// passed raycasting context.
void rc_delete (/*@only@*/ /*@out@*/ rc_ctx *self);

// determines whether a point is inside a given polygon (returns true if so).
// px and py must also be less than or equal to COORD_MAX.
bool rc_contains (rc_ctx const *self, long px, long py);

#endif /* RAYCASTING_H */
