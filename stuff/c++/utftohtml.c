#include <stdio.h>
#include <setjmp.h>
#include <stdint.h> /* Replace by <sys/types.h> if no stdint.h */

jmp_buf eof_exn;
jmp_buf invalid_utf_exn;
jmp_buf bad_data_exn;

int32_t
fgetutf (FILE *f)
{
  int ch;
  int32_t val;

  ch = getc (f);
  if ( ch == EOF )
    longjmp (eof_exn, 1);
  if ( ch >= 0 && ch < 0x80 )
    return ch;
  if ( ch >= 0xc0 && ch < 0xe0 )
    {
      val = ((int32_t)(ch-0xc0))<<6;
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= (int32_t)(ch-0x80);
      else
	longjmp (invalid_utf_exn, 1);
      return val;
    }
  if ( ch >= 0xe0 && ch < 0xf0 )
    {
      val = ((int32_t)(ch-0xe0))<<12;
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<6;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= (int32_t)(ch-0x80);
      else
	longjmp (invalid_utf_exn, 1);
      return val;
    }
  if ( ch >= 0xf0 && ch < 0xf8 )
    {
      val = ((int32_t)(ch-0xf0))<<18;
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<12;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<6;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= (int32_t)(ch-0x80);
      else
	longjmp (invalid_utf_exn, 1);
      return val;
    }
  if ( ch >= 0xf8 && ch < 0xfc )
    {
      val = ((int32_t)(ch-0xf8))<<24;
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<18;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<12;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<6;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= (int32_t)(ch-0x80);
      else
	longjmp (invalid_utf_exn, 1);
      return val;
    }
  if ( ch >= 0xfc && ch < 0xfe )
    {
      val = ((int32_t)(ch-0xf8))<<30;
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<24;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<18;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<12;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= ((int32_t)(ch-0x80))<<6;
      else
	longjmp (invalid_utf_exn, 1);
      ch = getc (f);
      if ( ch >= 0x80 && ch < 0xc0 )
	val |= (int32_t)(ch-0x80);
      else
	longjmp (invalid_utf_exn, 1);
      return val;
    }
  longjmp (invalid_utf_exn, 1);
}

void
fputhtml (FILE *f, int32_t ch)
{
  if ( ch == '&' )
    fprintf (f, "&amp;");
  else if ( ch == '<' )
    fprintf (f, "&lt;");
  else if ( ch == '>' )
    fprintf (f, "&gt;");
  else if ( ch == '\n' || ch == '\t' || ch == '\r' ||
	    ( ch >= ' ' && ch < 0x7f ) )
    putc (ch, f);
  else if ( ( ch >= 160 && ch < 55296 ) ||
	    ( ch >= 55344 && ch < 1114112 ))
    fprintf (f, "&#%ld;", (long int)ch);
  else {
    fprintf (f, "<!-- Invalid char %ld -->", (long int)ch);
    longjmp (bad_data_exn, 1);
  }
}

void
fhtmlheader (FILE *f)
{
  fprintf (f,
	   "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
	   "\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n");
  fprintf (f,
	   "<html>\n<head>\n<title>\nConverted UTF-8 document\n</title>\n"
	   "</head>\n<body>\n<pre>\n");
}

void
fhtmlfooter (FILE *f)
{
  fprintf (f,
	   "</pre>\n</body>\n</html>\n");
}

int
main (int argc, char *argv[])
{
  static FILE *f;

  if ( argc > 1 )
    {
      f = fopen (argv[1], "r");
      if ( f == NULL )
	{
	  perror ("Opening input file");
	  exit (1);
	}
    }
  else
    f = stdin;
  fhtmlheader (stdout);
  if ( setjmp (eof_exn) )
    {
      fhtmlfooter (stdout);
      fflush (stdout);
      exit (0);
    }
  if ( setjmp (invalid_utf_exn) )
    {
      fprintf (stderr, "Error: invalid UTF data.\n");
      exit (1);
    }
  if ( setjmp (bad_data_exn) )
    {
      fprintf (stderr, "Warning: bad data for HTML file.\n");
      /* Continue */
    }
  while (1) /* Exit from this loop by calling an exception */
    fputhtml (stdout, fgetutf (f));
}
