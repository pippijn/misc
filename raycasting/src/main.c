#include "raycasting.h"
#include "xmalloc.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK 1
#if CHECK
// generic function to check return values passsed to it
static void
checked (int ret, char const *act, char const *file, int line)
{
  if (ret == -1)
    {
      printf ("%s:%d: Failed `%s' because: %s\n", file, line, act, strerror (errno));
      exit (EXIT_FAILURE);
    }
}

#  define checked(action) checked (action, #action, __FILE__, __LINE__)
#else
#  define checked(action) (void)action
#endif

// prints usage information
static int
usage (void)
{
  (void)puts ("Usage: px py points.txt");
  return EXIT_SUCCESS;
}

// convert a given string into long
static long
require_number (char const *arg)
{
  char *endp;
  long number = strtol (arg, &endp, 10);

  if (number > COORD_MAX)
    {
      printf ("number too large: %s maximum allowed: %ld\n", arg, COORD_MAX);
      exit (EXIT_FAILURE);
    }
  if (*arg == '\0')
    {
      printf ("invalid number: %s\n", arg);
      exit (EXIT_FAILURE);
    }
  if (*endp != '\0')
    {
      perror ("parse_ints");
      exit (EXIT_FAILURE);
    }

  return number;
}


// Counts the lines in the data file by iterating over all of it and
// counting all the newline characters
static size_t
count_lines (FILE *fh)
{
  int ch;
  size_t count = 0;

  while ((ch = fgetc (fh)) != EOF)
    if ((char)ch == '\n')
      count++;

  // rewind the file handle
  rewind (fh);

  return count;
}


// Intelligently turns the contents of a file
// into an array of longs
static /*@only@*/ /*@null@*/ long *
read_numbers (char const *filename, /*@out@*/ size_t *count)
{
  int nch;
  long *intv; // base pointer
  long *intp; // pointer to last read integer
  size_t intc = 0; // count
  size_t linecount;

  FILE *fh = fopen (filename, "r");

  if (fh == NULL)
    {
      *count = 0;
      return NULL;
    }

  linecount = count_lines (fh);

  // we allocate twice the amount of memory as suggested by
  // the line count, since we expect every line to contain two numbers
  intp = intv = xmalloc (sizeof *intv * linecount * 2);

  {
    long cur = 0;
    enum { NUMBER, SPACE } state = SPACE;
    while ((nch = fgetc (fh)) != EOF)
      {
        // we distinguish between two states:
        // in the NUMBER state we read characters as atol() does and assume to deal with a base 10 number;
        // in SPACE we skip irrelevant characters until we find a number, if so, we seek back
        // once and change the state.
        char ch = (char)nch;
        switch (state)
          {
          case NUMBER:
            if (ch >= '0' && ch <= '9')
              {
                cur *= 10;
                cur += (long)(ch - '0');
                if (cur > COORD_MAX)
                  {
                    printf ("number too large: %ld maximum allowed: %ld\n", cur, COORD_MAX);
                    exit (EXIT_FAILURE);
                  }
              }
            else
              { // bail out if the file seems to be in the wrong format
                if (intc == linecount * 2)
                  {
                    (void)puts ("read_numbers: too many numbers in file (more than 2 numbers per line?)");
                    exit (EXIT_FAILURE);
                  }
                *intp++ = cur;
                intc++;
                cur = 0;
                state = SPACE;
              }
            break;
          case SPACE:
            if (ch >= '0' && ch <= '9')
              {
                state = NUMBER;
                checked (ungetc (ch, fh));
              }
            break;
          }
      }
  }

  checked (fclose (fh));

  *count = intc;
  return intv;
}

int
main (int argc, char *argv[])
{
  long px, py;
  size_t intc;
  long *intv;

  if (argc != 4)
    return usage ();

  // the first to arguments are interpreted respectively as X and Y coordinates of the point of interest
  px = require_number (argv[1]);
  py = require_number (argv[2]);

  // the third argument is interpreted as the filename
  // from which we are reading
  intv = read_numbers (argv[3], &intc);

  if (intv == NULL)
    {
      (void)puts ("main: error parsing points file");
      return EXIT_FAILURE;
    }

  if (intc % 2 != 0)
    {
      printf ("main: got %lu numbers, but expected an even number (pairs)\n", (unsigned long)intc);
      xfree (intv);
      return EXIT_FAILURE;
    }

  // call the algorithm that calculates wheter the point is inside the polyon
  {
    rc_ctx *ctx = rc_new (intc, intv);
    printf ("%s\n", rc_contains (ctx, px, py) ? "inside" : "outside");
    rc_delete (ctx);
  }

  xfree (intv);

  return EXIT_SUCCESS;
}
