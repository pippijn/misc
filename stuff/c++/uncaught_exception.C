#include <iostream>

struct A
{
  A ()
  {
    std::cout << "Constructing A (at " << this << ")\n";
  }

  ~A ()
  {
    if (std::uncaught_exception ())
      std::cout << "Destructing A (at " << this << ") after throwing an exception\n";
    else
      std::cout << "Destructing A (at " << this << ")\n";
  }
};

int
main ()
{
  A a;

  try
  {
    A a;
    throw 3;
  }
  catch (int i)
  {
    std::cout << "Integer " << i << " thrown\n";
  }

  return 0;
}
