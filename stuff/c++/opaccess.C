#include <cstdio>

template<typename T>
struct call_wrapper
{
  call_wrapper (T *p)
  : ptr (p)
  {
    puts ("before call");
  }

  ~call_wrapper ()
  {
    puts ("after call");
  }

  T *operator -> ()
  {
    return ptr;
  }

private:
  T *ptr;
};

class functor
{
  struct functions
  {
    functions (functor &that)
    : self (that)
    {
    }

    void fun ()
    {
      puts ("functor::functions::fun ()");
    }

  private:
    functor &self;
  };

public:
  functor ()
  : funcs (*this)
  {
  }

  call_wrapper<functions> operator -> ()
  {
    return &funcs;
  }

private:
  functions funcs;
};

int
main ()
{
  functor f;
  f->fun ();
}
