#define _GNU_SOURCE 1
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int
main (int argc, char **argv)
{
  while (--argc)
    {
      char const *file = *++argv;

      int fd = open (file, O_RDWR);
      if (fd == -1)
        {
          perror ("open");
          return 0;
        }

      struct stat file_stat;
      if (fstat (fd, &file_stat) == -1)
        {
          perror ("fstat");
          return 0;
        }

      char *const ptr = mmap (0, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
      if (ptr == (void *)-1)
        {
          perror ("mmap");
          return 0;
        }

      char *end = ptr + file_stat.st_size;

      // copy loop
      char *src_it;
      if ((src_it = memchr (ptr, '\r', file_stat.st_size)))
        {
          char *dst_it = src_it;

          while (src_it != end)
            {
#define USE_LIBC 0
#if USE_LIBC
              char *eol = memchr (src_it, '\r', end - src_it);
              if (eol != NULL)
                {
                  size_t length = eol - src_it - 1;
                  memmove (dst_it, src_it, length);
                  dst_it += length;
                  src_it = eol + 1;
                }
              else
                {
                  // no more \r in the file
                  size_t length = end - src_it;
                  memmove (dst_it, src_it, length);
                  dst_it += length;
                  src_it = end;
                }
#else
              if (*src_it != '\r')
                *dst_it++ = *src_it;
              src_it++;
#endif
            }
          // end of copy loop

          // truncate
          if (ftruncate (fd, dst_it - ptr) == -1)
            {
              perror ("ftruncate");
              return 0;
            }
        }

      if (munmap (ptr, file_stat.st_size) == -1)
        {
          perror ("munmap");
          return 0;
        }

      if (close (fd) == -1)
        {
          perror ("close");
          return 0;
        }
    }
  return 0;
}
