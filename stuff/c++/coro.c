#include <stdio.h>
int coro ()
{
  static int i = 0;
  switch (i) {
  case 0:
    puts ("hello");
    i++;
    return;
  case 1:
    puts ("else");
    i++;
    return;
  default:
    return;
  }
}

int main ()
{
  coro ();
  coro ();
  coro ();
  coro ();
}
