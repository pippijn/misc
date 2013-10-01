#include <cassert>
#include <vector>
#include <iostream>
#include <tr1/type_traits>

template<typename T>
class superpos_t
{
  typedef std::vector<T> data_type;

  data_type data;

public:
  superpos_t (T const &v0, T const &v1, T const &v2)
  : data (3), all (data), any (data)
  {
    data[0] = v0;
    data[1] = v1;
    data[2] = v2;
  }

  struct all_t
  {
    explicit all_t (data_type const &d)
    : data (d)
    {
    }

    bool operator == (T const &v) const
    {
      typename data_type::const_iterator it = data.begin ();
      typename data_type::const_iterator et = data.end ();
      while (it != et)
        {
          if (*it != v)
            return false;
          ++it;
        }
      return true;
    }

    std::string substr (std::string::size_type a)
    {
      std::string ret;
      typename data_type::const_iterator it = data.begin ();
      typename data_type::const_iterator et = data.end ();
      while (it != et - 1)
        {
          ret += it->substr (a) + ", ";
          ++it;
        }
      return ret + it->substr (a);
    }

  private:
    data_type const &data;
  };

  all_t all;

  template<typename U>
  struct any_t
  {
    explicit any_t (data_type const &d)
    : data (d)
    {
    }

    any_t<data_type> operator * (T const &v) const
    {
      data_type ret = data;
      typename data_type::iterator it = ret.begin ();
      typename data_type::iterator et = ret.end ();
      while (it != et)
        {
          *it *= v;
          ++it;
        }
      return any_t<data_type> (ret);
    }

    bool operator == (T const &v) const
    {
      typename data_type::const_iterator it = data.begin ();
      typename data_type::const_iterator et = data.end ();
      while (it != et)
        {
          if (*it == v)
            return true;
          ++it;
        }
      return false;
    }

    friend std::ostream &operator << (std::ostream &os, any_t const &any)
    {
      typename data_type::const_iterator it = any.data.begin ();
      typename data_type::const_iterator et = any.data.end ();
      while (it != et - 1)
        {
          os << *it << ", ";
          ++it;
        }
      os << *it;
      return os;
    }

  private:
    U data;
  };

  any_t<data_type const &> any;
};

template<typename T, typename U>
bool
operator == (T const &lhs, U const &rhs)
{
  return rhs.operator == (lhs);
}

template<typename T, typename U>
bool
operator != (T const &lhs, U const &rhs)
{
  return !(lhs == rhs);
}

/*
 * Program output:
 *
 * 4!
 * 1, 2, 3
 * 2, 4, 6
 * meow found in set!
 * the entire set is not meow. good.
 * g, t, ow
 */
int
main ()
{
  superpos_t<int> p (1, 2, 3);
  if (4 == p.any * 2)
    {
      std::cout << "4!" << std::endl;
      std::cout << p.any << std::endl;
      std::cout << p.any * 2 << std::endl;
    }

  superpos_t<std::string> p2 ("dog", "cat", "meow");
  if ("meow" == p2.any)
    std::cout << "meow found in set!" << std::endl;

  assert ("meow" != p2.all);
  if ("meow" != p2.all)
    std::cout << "the entire set is not meow. good." << std::endl;

  std::cout << p2.all.substr (2) << std::endl;
}
