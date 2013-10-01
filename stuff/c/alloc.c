#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void *
mymalloc (size_t bytes)
{
  void *ptr;

  printf ("Allocating %d bytes\n", bytes);
  ptr = malloc (bytes + sizeof (size_t));
  memcpy (ptr, &bytes, sizeof (size_t));

  return (char *) ptr + sizeof (size_t);
}

void
myfree (void *ptr)
{
  size_t bytes;
  char *ptr_p;
  
  ptr_p = (char *) ptr - sizeof (size_t);
  memcpy (&bytes, ptr_p, sizeof (size_t));
  printf ("Freeing %d bytes\n", bytes);

  free (ptr_p);
}

int
main ()
{
  char *buf = mymalloc (5);
  myfree (buf);

  return 0;
}
