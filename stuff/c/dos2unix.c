#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int
main (int argc, char **argv)
{
  char src[1024 * 1024];
  char dst[1024 * 1024];

  while (--argc)
    {
      char const *file = *++argv;
      int fd = open (file, O_RDWR);

      off_t read_pos = 0;
      off_t write_pos = 0;

      // while there are more bytes to read
      while (1)
        {
          ssize_t const bytes = read (fd, src, sizeof src);

          ssize_t src_pos, dst_pos = 0;
          // advance read position by number of bytes read
          read_pos += bytes;

          // go through the whole buffer
          for (src_pos = 0; src_pos < bytes; src_pos++)
            if (src[src_pos] != '\r')
              dst[dst_pos++] = src[src_pos];
          // seek back to the beginning of the current block
          lseek (fd, write_pos, SEEK_SET);
          // write back to file
          write_pos += write (fd, dst, dst_pos);
          // seek to new read position
          lseek (fd, read_pos, SEEK_SET);

          if (bytes != sizeof src)
            break;
        }

      if (ftruncate (fd, write_pos) == -1)
        perror ("ftruncate");

      close (fd);
    }
  return 0;
}
