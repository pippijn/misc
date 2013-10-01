#include <iostream>

template<bool>
struct overflow
{
  long c : INT_MAX;
};

template<>
struct overflow<false>
{
};

template<int step, int base,long res = 1>
struct power
{
  static long const check = sizeof (overflow<(res > (LONG_MAX / base))>);
  static long const result = power <step - 1, base, res * base>::result;
};

template<int base, long res>
struct power<0, base, res>
{
  static long const result = res;
};

int
main ()
{
  std::cout << power<32, 2>::result << "\n";
}
