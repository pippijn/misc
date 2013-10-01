/**
 * Use memset to initialise the booleans and bool instead of the
 * boolean<bool> template for booleans.
 *
 * This makes things go a little faster, but only on initialisation
 * of the sieve. It does not change runtime speed.
 *
 * Note that the boolean class is not slower than pure booleans. The only
 * thing that booleans are faster at is being initialised.
 */
#define USE_MEMSET 1

#include <cmath>
#include <cstring>

#include <iostream>
#include <iomanip>

template<bool B>
struct boolean
{
  boolean ()
  : self (B)
  {
  }

  void operator = (bool b)
  {
    self = b;
  }

  operator bool () const
  {
    return self;
  }

private:
  bool self;
};

template<size_t MAX>
struct sieve
{
  sieve ()
  : sqrtn (sqrt (MAX))
  {
    memset (prime, 1, sizeof prime);
  }

  void operator () ()
  {
    for (size_t i = 2; i <= sqrtn; i++)
      if (prime[i])
        for (size_t s = 2; s <= (MAX / i); s++) // gcc optimises the constant (MAX / i) to a single division
          prime[s * i] = 0;
  }

  friend std::ostream &operator << (std::ostream &os, sieve const &s)
  {
    float beta  = 1.3;
    float eta   = 1.0 / 16.0;
    float p1    = 1.0;
    float p2    = 60000.0;
    float gf_p1 = 3.0;
    float gf_p2 = 1.2;
    float delta = (gf_p2 - gf_p1) / (p2 - p1);
    float q     = (p2 * gf_p1 - p1 * gf_p2) / (p2 - p1);
    size_t prevdiff = 0;
    size_t prevprime = 1;
    size_t prevratio = 0;
    int nprimes = 0;

    os << "/**\n"
          " * Prime number table. Used for Rona hash table expansion.\n"
          " */\n"
          "\n"
          "#ifndef PRIMES_H\n"
          "#define PRIMES_H\n"
          "\n"
          "static const long primes[] = {\n";

    for (size_t prime = 2; prime < MAX; prime++)
      if (s.prime[prime])
        {
          float gfl = delta * prime + q;
          float gf  = beta + eta * exp (gfl);

          if (prime / prevprime < gf)
            continue;

          prevdiff  = prime - prevprime;
          prevratio = prime / prevprime;

          os << std::setw (10) << prime << ",\n";

          ++nprimes;
          prevprime = prime;
        }

    os << "};\n"
          "\n"
          "static int nprimes = " << nprimes << ";\n"
          "\n"
          "#endif /* PRIMES_H */\n";

    return os;
  }
  
  bool *list ()
  {
    return &prime;
  }

private:
  bool prime[MAX];
  size_t sqrtn;
};

typedef sieve<1000000000> prime_sieve;
prime_sieve p;

int
main ()
{
  p ();
  std::cout << p;
}
