#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

struct char_multiset
{
  unsigned char chars[256];

  char_multiset (char const *it, char const *et)
    : chars { }
  {
    assert (it != et);
    while (it != et)
      ++chars[*it++];
  }

  unsigned char const *end () const
  {
    return &chars[sizeof chars];
  }

  unsigned char const *find (unsigned char c) const
  {
    if (chars[c])
      return &chars[c];
    return end ();
  }

  void erase (unsigned char const *it)
  {
    assert (it != end ());
    assert (*it != 0);

    --chars[it - chars];
  }
};

static bool
longer (std::string const &a, std::string const &b)
{
  return a.size () > b.size ();
}

int
main (int argc, char *argv[])
{
  assert (argc == 4);

  std::ifstream in (argv[1]);
  assert (in);

  char_multiset const chars (argv[2], argv[2] + strlen (argv[2]));
  char const must = argv[3][0];

  std::vector<std::string> matches;
next:
  while (in)
    {
      std::string line;
      std::getline (in, line);

      if (line.find (must) == -1)
        continue;

      char_multiset chars_left = chars;
      for (char c : line)
        {
          auto found = chars_left.find (c);
          if (found == chars_left.end ())
            goto next;
          chars_left.erase (found);
        }

      matches.emplace_back (move (line));
    }

  std::sort (matches.begin (), matches.end (), longer);
  for (std::string const &word : matches)
    std::cout << word << "\n";

  return 0;
}
