#include <iostream>
#include <boost/static_assert.hpp>

namespace types
{
  template<typename T, T t>
  struct integral_constant
  {
  };

  enum name
  {
    type_nothing,
    type_bool,
    type_char,
    type_int,
    type_charptr
  };

  typedef integral_constant<name, type_nothing> nothing_type;
  template<typename>
  struct get
  : public nothing_type
  {
  };

#define ADD_TYPE(real_type, type_name)                                                  \
  typedef integral_constant<name, type_ ## type_name> type_name ## _type;     \
  template<>                                                                            \
  struct get<real_type>                                                                 \
  : public type_name ## _type                                                           \
  {                                                                                     \
  }

  ADD_TYPE (bool, bool);
  ADD_TYPE (char, char);
  ADD_TYPE (int, int);
  ADD_TYPE (char *, charptr);
}

struct S
{
  template<typename T>
  T retrieve ()
  {
    return retrieve (types::get<T> ());
  }

private:
  int retrieve (types::int_type)
  {
    return 42;
  }

  char retrieve (types::char_type)
  {
    return 42;
  }

  char *retrieve (types::charptr_type)
  {
    return "fourtytwo";
  }
};

namespace impl
{
  template<bool>
  struct static_assert;
  
  template<>
  struct static_assert<true>
  {
  };
}

#define static_assert(cond, msg) impl::static_assert<(cond)> JOIN4 (sa, __LINE__, _, msg)

#define JOIN(A, B)              DOJOIN (A, B)
#define DOJOIN(A, B)            DODOJOIN (A, B)
#define DODOJOIN(A, B)          A ## B

#define JOIN3(A, B, C)          JOIN (JOIN (A, B), C)
#define JOIN4(A, B, C, D)       JOIN (JOIN3 (A, B, C), D)
#define JOIN5(A, B, C, D, E)    JOIN (JOIN4 (A, B, C, D), E)
#define JOIN6(A, B, C, D, E, F) JOIN (JOIN5 (A, B, C, D, E), F)


int
main ()
{
  S s;

  static_assert (0, Cant_do_this);
  std::cout << s.retrieve<char *> () << "\n";

  return 0;
}
