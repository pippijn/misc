#include <gmpxx.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

static mpz_t
newfactgmpy (mpz_t n, bool v = false)
{
  assert (n >= 100);

  mpz_t logn;
  mpz_init_set_d (logn, pow ((round (floor (log10 (mpz_get_d (n))))), 6));
  std::vector<mpz_t> multitotals;
  for (int i = 0; i < logn; i++)
    {
      mpz_t m;
      mpz_init_set_ui (m, 1);
      multitotals.push_back (m);
    }
  mpz_t lf;
  mpz_init_set (lf, n); // (logn - 1)
  std::vector<mpz_t> frs;
  mpz_t m;
  mpz_init_set (m, 1); // (logn - 1)
  frs.push_back (m);
  for (int i = 0; i < logn - 2; i++)
    {
      mpz_t a;
      mpz_init_set (a, lf);

      frs.push_back (lf * (i + 1));
    }
  frs.push_back (n);
  if (v)
    {
      std::cout << "beginning initial calculation\n"
                << frs.size () << '\n';
    }
  for (int i = 1; i < frs.size (); i++)
    {
      if (v)
        {
          std::cout << i << '\n'
                    << frs.at (i) << '\n';
        }
      for (mpz_class j = frs.at (i - 1); j < frs.at (i); j++)
        {
          multitotals.at (i) *= j;
        }
    }
  if (v)
    std::cout << multitotals.size () << '\n';
  std::vector<mpz_class> totals;
  mpz_class total = 1;
  while (multitotals.size () > 1)
    {
      for (int i = 1; i < multitotals.size (); i += 2)
        totals.push_back (multitotals.at (i - 1) * multitotals.at (i));
      if (v)
        std::cout << multitotals.size ();
      multitotals = totals;
      totals.clear ();
    }
  return multitotals.at (0);
}

int
main ()
{
  std::cout << newfactgmpy (100000) << '\n';
}
