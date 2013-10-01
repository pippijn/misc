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

    rcint
    atomicCompareExchange (rcint oldval, rcint newval)
    {
      return ::harnix::atomicCompareExchangePtr (&m_nCount,
                                                 (uintptr_t)oldval, (uintptr_t)newval);
    }

  public:
    CLECount ()
      : m_nCount (LockAddend)
    {
    }

    // note - atomicXXX operations should have already guaranteed generated code ordering
    // for compiler. In current version, they also guarantee execution ordering in x86 and
    // IA64.
    void
    lock ()
    {
      atomicExchangeAdd (LockAddend);
    }

    bool
    unlock ()
    {
      // hold the environment unchanged if it is about to decrement to zero
      rcint oldval = 0, newval, retval;

      ASSERT (m_nCount != 0);
      while (1)
        {
          newval = oldval - LockAddend;
          if (newval == 0)
            {
              return true;
            }
          else
            {
              retval = atomicCompareExchange (oldval, newval);
              if (retval == oldval)
                {
                  return false;
                }
            }
          oldval = retval;
        }
    }

    void
    addRef ()
    {
      atomicExchangeAdd (RefAddend);
    }

    bool
    release ()
    {
      // hold the environment unchanged if it is about to decrement to zero
      rcint oldval = 0, newval, retval;

      ASSERT (m_nCount != 0);
      while (1)
        {
          newval = oldval - RefAddend;
          if (newval == 0)
            {
              return true;
            }
          else
            {
              retval = atomicCompareExchange (oldval, newval);
              if (retval == oldval)
                {
                  return false;
                }
            }
          oldval = retval;
        }
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
    uint_flags
    _getRaw ()
    {
      // return atomicCompareExchange(0, 0);
      return m_vFlags;
    }

    uint_flags
    atomicCompareExchange (uint_flags oldval, uint_flags newval)
    {
      return ::harnix::atomicCompareExchange (&m_vFlags, oldval, newval);
    }

  public:
    CAtomicFlags ()
      : m_vFlags (0)
    {
    }

    CAtomicFlags (uint_flags value)
      : m_vFlags (value)
    {
    }

    void
    bitwiseAND (uint_flags value)
    {
      RACINGDELAY ();

      uint_flags oldval;
      uint_flags newval;
      uint_flags rc = _getRaw ();

      do
        {
          oldval = rc;
          newval = rc & value;
          rc = atomicCompareExchange (oldval, newval);
        }
      while (rc != oldval);

      RACINGDELAY ();
    }

    void
    bitwiseOR (uint_flags value)
    {
      RACINGDELAY ();

      uint_flags oldval;
      uint_flags newval;
      uint_flags rc = _getRaw ();

      do
        {
          oldval = rc;
          newval = rc | value;
          rc = atomicCompareExchange (oldval, newval);
        }
      while (rc != oldval);

      RACINGDELAY ();
    }

    // mask: 0-unchanged, 1-change to corresponding bit value of value
    void
    setValue (uint_flags mask, uint_flags value)
    {
      RACINGDELAY ();

      uint_flags oldval;
      uint_flags newval;
      uint_flags rc = _getRaw ();

      do
        {
          oldval = rc;
          newval = (rc & ~mask) | (value & mask);
          rc = atomicCompareExchange (oldval, newval);
        }
      while (rc != oldval);

      RACINGDELAY ();
    }

    // mask: 0-unchanged, 1-change to corresponding bit value of value
    uint_flags
    bitwiseCompareExchange (uint_flags mask, uint_flags test, uint_flags value)
    {
      RACINGDELAY ();

      uint_flags oldval;
      uint_flags newval;
      uint_flags rc = _getRaw ();

      // note - test and value must fall into the mask shadow.
      // test = mask & test;
      // value = mask & value;
      do
        {
          oldval = rc;
          if (test != (rc & mask))
            {
              // critical portion is changed
              break;
            }
          newval = (rc & ~mask) | value;
          rc = atomicCompareExchange (oldval, newval);
        }
      while (rc != oldval);

      RACINGDELAY ();

      return oldval & mask;
    }

    // mask: 0-untested bits, 1-test bits
    uint_flags
    bitwiseTest (uint_flags mask)
    {
      RACINGDELAY ();

      uint_flags rc = atomicCompareExchange (0, 0);

      RACINGDELAY ();

      return rc & mask;
    }

    uint_flags
    constTest (uint_flags mask)
    {
      return _getRaw () & mask;
    }
  };

  //
  // CTraverseLock - allow application to pause and resume Traverse operation
  //
  class CTraverseLock
  {
  private:
    CCritSec m_vLock;                                                           // real lock
    CCritSec m_vHelper;                                                         // helper lock
    uintptr_t m_nCount;                                                         // Request Count

  public:
    CTraverseLock ()
      : m_nCount (0)
    {
    }

    void
    gcLock ()
    {
      m_vLock.lock ();
    }

    void
    gcUnlock ()
    {
      m_vLock.unlock ();
    }

    void
    gcYield ()
    {
      if (m_nCount)
        {
          m_vLock.unlock ();
          m_vHelper.lock ();                                                    // often waiting here
          m_vHelper.unlock ();
          m_vLock.lock ();
        }
    }

    void
    appLock ()
    {
      if (atomicExchangeAddPtr (&m_nCount, uintptr_t (1)) == 0)
        {
          // the first request
          m_vHelper.lock ();
        }
      m_vLock.lock ();
    }

    void
    appUnlock ()
    {
      if (atomicExchangeAddPtr (&m_nCount, uintptr_t (-1)) == 1)
        {
          // the last leaving
          m_vHelper.unlock ();
        }
      m_vLock.unlock ();
    }
  };


#if defined(HNXGC_CONFIG_UNALIGNACCESS)
  //
  // CRollbackLock - allow traverse rollback when racing with mutator
  //
  class CRollbackLock
  {
  private:
    volatile uint32 m_nCount;
    bool m_bRacing;

  private:
    // return true if conflict is detected
    bool
    interlockedDecrement_mb ()
    {
      return atomicDecrement (&m_nCount) != 1;
    }

    bool
    interlockedIncrement_acq ()
    {
      return atomicIncrementAcquire (&m_nCount) != 0;
    }

    bool
    interlockedDecrement_rel ()
    {
      return atomicDecrementRelease (&m_nCount) != 1;
    }

  public:
    CRollbackLock ()
      : m_nCount (0)
    {
    }

    bool
    enterTry ()
    {
      m_bRacing = 0;                    // ignore previous conflicts
      return interlockedIncrement_acq ();
    }

    bool
    leaveTry ()
    {
      if (interlockedDecrement_mb ())
        {
          return true;
        }
      else if (m_bRacing)
        {
          return true;
        }
      else
        {
          return false;
        }
    }

    void
    enterWork ()
    {
      if (interlockedIncrement_acq ())
        {
          // some others are in the section
          m_bRacing = true;
          // the setting is guaranteed to be done before leaveWork for
          // the reason of release semantic.
        }
    }

    void
    leaveWork ()
    {
      interlockedDecrement_rel ();
    }
  };
#endif
} // end of namespace harnix
