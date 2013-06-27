#include <stdio.h>
#include <stdlib.h>

int
main ()
{
  int max = 100000;
  int i;
  FILE *fh = fopen ("testsuite/generated.txt", "w");

  if (fh == NULL)
    return EXIT_FAILURE;

  for (i = 0; i < max; i++)
    fprintf (fh, "%d %d\n", rand () % max, rand () % max);
}
