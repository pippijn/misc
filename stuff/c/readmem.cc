#define _XOPEN_SOURCE 500
#define _FILE_OFFSET_BITS 64
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <unistd.h>

static void
error_unless (bool cond, char const *file, int line, char const *c)
{
  if (!cond)
    {
      printf ("error at %s:%d: %s -> %m\n", file, line, c);
      throw 0;
    }
}

static FILE *
checked (FILE *fh, char const *file, int line, char const *c)
{
  error_unless (fh != NULL, file, line, c);
  return fh;
}

static int
checked (int v, char const *file, int line, char const *c)
{
  error_unless (v != -1, file, line, c);
  return v;
}

#define checked(v) checked (v, __FILE__, __LINE__, #v)

int
main (int argc, char *argv[])
{
  if (argc < 2)
    return EXIT_FAILURE;

  assert (argv[1] != NULL);
  int pid = atoi (argv[1]);
  assert (pid != 0);
  char mempath[] = "/proc/65535/mem";

  snprintf (mempath, sizeof mempath, "/proc/%d/mem", pid);

  checked (ptrace (PTRACE_ATTACH, pid, NULL, NULL));
  try
  {
    int fd = checked (open (mempath, O_RDONLY));
    try
    {
      char buf[BUFSIZ];
      ssize_t nmemb;
      while (checked (nmemb = read (fd, buf, sizeof buf)))
        printf ("read %zd bytes -> %m\n", nmemb);
    }
    catch (...)
    {
    }
    close (fd);
  }
  catch (...)
  {
  }
  checked (ptrace (PTRACE_DETACH, pid, NULL, NULL));

  return EXIT_SUCCESS;
}
