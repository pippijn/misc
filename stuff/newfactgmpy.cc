#include <gmpxx.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

static mpz_class
newfactgmpy (mpz_class n, bool v = false)
{
  assert (n >= 100);

  mpz_class logn = pow ((round (floor (log10 (n.get_d ())))), 6);
  std::cout << logn << '\n';
  if (v)
    std::cout << logn << '\n';
  std::vector<mpz_class> multitotals;
  for (int i = 0; i < logn; i++)
    multitotals.push_back (1);
  mpz_class lf = n; // (logn - 1)
  std::vector<mpz_class> frs;
  frs.push_back (1);
  for (int i = 0; i < logn - 2; i++)
    frs.push_back (lf * (i + 1));
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
