#include <cstdio>

struct blah
{
  struct foo
  {
    void do_something ()
    {
      puts (existing ? "Hello from foo" : "Hello from a nonexisting foo");
    }

    foo (bool exists)
    : nr (ctr++), existing (exists)
    {
    }

  private:
    static int ctr;
    int nr;
    bool existing;
  };

  blah ()
  : null_foo (false)
  {
  }

  foo &get_foo ()
  {
    return f ? *f : null_foo;
  }

  foo *get_fooptr ()
  {
    return f;
  }

  void create_foo ()
  {
    f = new foo (true);
  }

  foo *operator -> ()
  {
    return f ? f : &null_foo;
  }

private:
  foo *f;
  foo null_foo;
};

int blah::foo::ctr;

int
main ()
{
  blah b;
  b->do_something ();
  b.create_foo ();
  b->do_something ();

  return 0;
}
