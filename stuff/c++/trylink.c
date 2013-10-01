#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

int
main (int argc, char *argv[])
{
  int i;
  void *handle;

  if (argc < 2)
    {
      printf ("Usage: %s <library> [<library> ...]\n", argv[0]);
      return EXIT_FAILURE;
    }
  else
    {
      for (i = 1; i < argc; i++)
        {
          printf ("Trying to link with \033[1;34m%s\033[0m...", argv[i]);
          handle = dlopen (argv[i], RTLD_NOW);
          if (handle == NULL)
            printf ("\033[1;31mfailed:\033[0m %s\n", dlerror());
          else
            {
              printf ("\033[1;32msuccess\033[0m\n");
#if 0
              if (dlclose (handle) != 0)
                {
                  printf ("\033[1;31mError\033[0m while unloading library %s\n", argv[i]);
                  return EXIT_FAILURE;
                }
#endif
            }
        }
    }

  return EXIT_SUCCESS;
}
