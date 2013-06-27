#include "xmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// catches bad (excessive)memory allocations
void *
xmalloc (size_t bytes)
{
  void *ptr = malloc (bytes);

  if (ptr == NULL)
    {
      (void)puts ("out of memory");
      exit (EXIT_FAILURE);
    }

  memset (ptr, 0, bytes);

  return ptr;
}

void
xfree (void *ptr)
{
  free (ptr);
}
