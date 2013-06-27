#define _GNU_SOURCE
#include <dlfcn.h>

#include <stdio.h>
#include <string.h>

static void init (void) __attribute__ ((__constructor__));
static void *(*real_dlopen)(const char *, int);

static void
init (void)
{
  real_dlopen = dlsym (RTLD_NEXT, "dlopen");
}

static char const *const noload[] = {
  "libpulse.so.0",
  NULL,
};

void *
dlopen (char const *filename, int flag)
{
  char const *const *it = noload;
  for (it = noload; *it; ++it)
    if (strcmp (filename, *it) == 0)
      {
        fprintf (stderr, "Not loading %s\n", *it);
        return NULL;
      }
#if 0
  fprintf (stderr, "Loading %s\n", filename);
#endif
  return real_dlopen (filename, flag);
}
