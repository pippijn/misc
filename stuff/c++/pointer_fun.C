#include <iostream>

// forward declaration of B, so a pointer to it can be used within A
struct B;

struct A
{
  A ()
  {
    std::cout << "In A::A ()\n";
  }

  void do_something ()
  {
    std::cout << "Doing something in A\n";
  }

  B *b;
};

struct B
{
  B ()
  {
    std::cout << "In B::B ()\n";
  }

  void do_something ()
  {
    std::cout << "Doing something in B\n";
  }

  A *a;
};

int
main ()
{
  A a;
  B b;

  a.b = &b;
  b.a = &a;

  a.b->a->b->a->b->a->do_something ();

  return 0;
}
