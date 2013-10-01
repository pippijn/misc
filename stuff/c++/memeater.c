#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int
main()
{
  nice (20);
  while (1)
    {
      void *p = malloc (67295);
      memset (p, 0, 67295);
    }
}
