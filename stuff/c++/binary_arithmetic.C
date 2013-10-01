#ifndef BINARY_ARITHMETIC_HPP
#define BINARY_ARITHMETIC_HPP

#include <stdexcept>
#include <cassert>
#include <limits>

namespace cdiggins
{
  template<class T>
  T
  logical_xor (const T &x, const T &y)
  {
    return (x || y) && !(x && y);
  }

  bool
  full_adder (bool b1, bool b2, bool &carry)
  {
    bool sum = (b1 ^ b2) ^ carry;
    carry = (b1 && b2) || (b1 && carry) || (b2 && carry);
    return sum;
  }

  bool
  full_subtractor (bool b1, bool b2, bool &borrow)
  {
    bool diff;
    if (borrow)
      {
        diff = !logical_xor (b1, b2);
        borrow = !b1 || (b1 && b2);
      }
    else
      {
        diff = logical_xor (b1, b2);
        borrow = !b1 && b2;
      }
    return diff;
  }

  template<typename T>
  unsigned int
  count_significant_digits (T x)
  {
    unsigned int n = 0;
    while (x > 0)
      {
        ++n;
        x >>= 1;
      }
    return n;
  }

  template<typename T>
  T
  multiply (T x, T y)
  {
    // this multiply accepts only non-negative integer values
    assert (x >= 0);
    assert (y >= 0);
    T ret = 0;
    while (y > 0)
      {
        if ((y & 1) == 1)
          {
            ret += x;
          }
        y >>= 1;
        x <<= 1;
      }
    return ret;
  }

  template<typename T>
  void
  divide (T x, T y, T &q, T &r)
  {
    assert (x >= 0);
    assert (y >= 0);
    if (y == 0)
      {
        throw std::domain_error ("division by zero undefined");
      }
    if (x == 0)
      {
        q = 0;
        r = 0;
        return;
      }
    if (y == x)
      {
        q = 1;
        r = 0;
        return;
      }
    
    q = 0;
    r = x;
    
    if (y > x)
      {
        return;
      }
    
    unsigned int sig_x = count_significant_digits (x);
    unsigned int sig_y = count_significant_digits (y);
    
    assert (sig_x >= sig_y);

    unsigned int n = (sig_x - sig_y);
    y <<= n;
    
    ++n;
    
    // long division algorithm, shift and subtract
    while (n--)
      {
        q <<= 1; // shift the quotient to the left
        if (y <= r)
          {
            q |= 1; // add a new digit to quotient
            r = r - y; // subtract from the remained
          }
        y >>= 1; // shift the divisor to the right
      }
  }

  template<typename T>
  T
  divide_quotient (const T &x, const T &y)
  {
    T q;
    T r;
    divide (x, y, q, r);
    return q;
  }

  template<typename T>
  T
  divide_remainder (const T &x, const T &y)
  {
    T q;
    T r;
    divide (x, y, q, r);
    return r;
  }
}

namespace binary_arithmetic_test
{
  using namespace cdiggins;

  void test(bool b) {
    if (!b) {
      throw std::runtime_error("test failed");
    }
  }

  void test_divide() {
    for (unsigned char i=0; i < 0xFF; i++) {
      for (unsigned char j=1; j < 0xFF; j++) {
        test(divide_quotient(i, j) == i / j);
        test(divide_remainder(i, j) == i % j);
      }
    }
  }

  void test_multiply() {
    for (int i=0; i < 0xFF; i++) {
      for (int j=0; j < 0xFF; j++) {
        test(multiply(i, j) == i * j);
      }
    }
  }

  void test_full_adder() {
    bool carry = true;
    test(full_adder(true, true, carry) == true);
    test(carry == true);
    test(full_adder(true, false, carry) == false);
    test(carry == true);
    test(full_adder(false, true, carry) == false);
    test(carry == true);
    test(full_adder(false, false, carry) == true);
    test(carry == false);
    test(full_adder(true, false, carry) == true);
    test(carry == false);
    test(full_adder(false, true, carry) == true);
    test(carry == false);
    test(full_adder(true, true, carry) == false);
    test(carry == true);
  }

  void test_full_subtractor() {
    bool borrow = false;
    test(full_subtractor(true, true, borrow) == false);
    test(borrow == false);
    test(full_subtractor(true, false, borrow) == true);
    test(borrow == false);
    test(full_subtractor(false, false, borrow) == false);
    test(borrow == false);
    test(full_subtractor(false, true, borrow) == true);
    test(borrow == true);
    test(full_subtractor(true, true, borrow) == true);
    test(borrow == true);
    test(full_subtractor(false, true, borrow) == false);
    test(borrow == true);
    test(full_subtractor(false, false, borrow) == true);
    test(borrow == true);
    test(full_subtractor(true, false, borrow) == false);
    test(borrow == false);
  }

  void test_main()
    {
      test_divide();
      test_multiply();
      test_full_adder();
      test_full_subtractor();
    }
}

#endif
