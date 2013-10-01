// This is a part of the HnxGC Library.
// Copyright (C) Mingnan Guo (author).
// All rights reserved.
//
// This program(s) uses several patented inventions of the author in the United
// States and/or other countries. Whoever without authority makes, uses, offers
// to sell, or sells any of these patented invention, within the United States
// and/or other countries infringes the patent(s).
//
// The copyright to the computer program(s) herein is the property of author.
// The program(s) may be used and/or copied only with the written permission of
// Mingnan Guo or in accordance with the terms and conditions stipulated in the
// agreement/contract under which the program(s) have been supplied. The
// copyright notice must not be removed.
//
// Permission to use, copy and distribute this software and its documentation
// for a non-commercial educational purpose can be granted free by electronic
// registration via hnxgc.harnixworld.com, provided that this copyright notice
// appears in all copies.
//
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, EXPRESS,
// IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//
// IN NO EVENT SHALL MINGNAN GUO LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT
// OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER RESULTING
// FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE POSSIBILITY
// OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION
// WITH THE USE OR PERFORMANCE  OF THIS SOFTWARE.
//


namespace harnix
{
  //
  // CLECount - atomic operation on Lock-Count and Extra-Count
  //
  class CLECount
  {
    uintptr_t m_nCount;                                                         // Lock-Count & Extra-Count
    typedef intptr_t rcint;
    static const rcint LockShift = sizeof (void *) * 5;                         // # of bits shift for Lock-Counter
    // 20 for 32-bit and 40 for 64-bit
    static const rcint LockAddend = rcint (1) << LockShift;
    static const rcint RefAddend = rcint (1);
    static const rcint LockMask = rcint (-1) << LockShift;                      // mask for Lock-Count

    rcint
    atomicExchangeAdd (rcint value)
    {
      return ::harnix::atomicExchangeAddPtr (&m_nCount, (uintptr_t)value);
    }

  public:
    CLECount ()
      : m_nCount (LockAddend)
    {
    }

    void
    lock ()
    {
      atomicExchangeAdd (LockAddend);
    }

    bool
    unlock ()
    {
      return atomicExchangeAdd (-LockAddend) - LockAddend == 0;
    }

    void
    addRef ()
    {
      atomicExchangeAdd (RefAddend);
    }

    bool
    release ()
    {
      return atomicExchangeAdd (-RefAddend) - RefAddend == 0;
    }

    bool
    isLocked ()
    {
      return (m_nCount & LockMask) != 0;
    }

    bool
    isZero ()
    {
      return m_nCount == 0;
    }

    void
    clear ()
    {
      m_nCount = 0;
    }
  };

  //
  // CAtomicFlags - atomic operation on bit-field flags
  //
  typedef uint32 uint_flags;
  class CAtomicFlags
  {
    uint_flags m_vFlags;

  public:
    CAtomicFlags ()
      : m_vFlags (0)
    {
    }

    CAtomicFlags (uint_flags value)
      : m_vFlags (value)
    {
    }

    uint_flags
    _getRaw ()
    {
      return m_vFlags;
    }

    uint_flags
    atomicCompareExchange (uint_flags oldval, uint_flags newval)
    {
      return (uint_flags) ::harnix::atomicCompareExchange (&m_vFlags, oldval, newval);
    }

    void
    bitwiseAND (uint_flags value)
    {
      uint_flags oldval;
      uint_flags newval;
      uint_flags rc;

      do
        {
          oldval = _getRaw ();
          newval = oldval & value;
          rc = atomicCompareExchange (oldval, newval);
        }
      while (rc != oldval);
    }

    void
    bitwiseOR (uint_flags value)
    {
      uint_flags oldval;
      uint_flags newval;
      uint_flags rc;

      do
        {
          oldval = _getRaw ();
          newval = oldval | value;
          rc = atomicCompareExchange (oldval, newval);
        }
      while (rc != oldval);
    }

    // mask: 0-unchanged, 1-change to corresponding bit value of value
    void
    setValue (uint_flags mask, uint_flags value)
    {
      uint_flags oldval;
      uint_flags newval;
      uint_flags rc;

      do
        {
          oldval = _getRaw ();
          newval = (oldval & ~mask) | (value & mask);
          rc = atomicCompareExchange (oldval, newval);
        }
      while (rc != oldval);
    }

    // mask: 0-unchanged, 1-change to corresponding bit value of value
    uint_flags
    bitwiseCompareExchange (uint_flags mask, uint_flags test, uint_flags value)
    {
      uint_flags oldval;
      uint_flags newval;
      uint_flags rc;

      // test = mask & test;
      // value = mask & value;
      do
        {
          oldval = _getRaw ();
          if (test != (oldval & mask))
            {
              // critical portion is changed
              break;
            }
          newval = (oldval & ~mask) | value;
          rc = atomicCompareExchange (oldval, newval);
        }
      while (rc != oldval);
      return oldval & mask;
    }

    // mask: 0-untested bits, 1-test bits
    uint_flags
    bitwiseTest (uint_flags mask)
    {
      return _getRaw () & mask;
    }
  };
} // end of namespace harnix
