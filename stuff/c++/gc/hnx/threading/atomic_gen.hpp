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


//
// Default template functions based on atomicCompareExchange(Acquire/Relase)
//
namespace harnix
{
  //
  // ExchangeAdd
  //
  template<typename T>
  static inline
  T
  atomicExchangeAdd (T volatile *pVar, T value)
  {
    T retval = *pVar;
    T oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchange (pVar, oldval, oldval + value);
      }
    while (retval != oldval);
    return retval;
  }

#if !defined(HNX_EXCHANGEADDACQUIRE_DEFINED)
#define HNX_EXCHANGEADDACQUIRE_DEFINED
  template<typename T>
  static inline
  T
  atomicExchangeAddAcquire (T volatile *pVar, T value)
  {
    T retval = *pVar;
    T oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangeAcquire (pVar, oldval, oldval + value);
      }
    while (retval != oldval);
    return retval;
  }
#endif


#if !defined(HNX_EXCHANGEADDRELEASE_DEFINED)
#define HNX_EXCHANGEADDRELEASE_DEFINED
  template<typename T>
  static inline
  T
  atomicExchangeAddRelease (T volatile *pVar, T value)
  {
    T retval = *pVar;
    T oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangeRelease (pVar, oldval, oldval + value);
      }
    while (retval != oldval);
    return retval;
  }
#endif


  //
  // Exchange
  //
  template<typename T>
  static inline
  T
  atomicExchange (T volatile *pVar, T value)
  {
    T retval = *pVar;
    T oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchange (pVar, oldval, value);
      }
    while (retval != oldval);
    return retval;
  }

  template<typename T>
  static inline
  T
  atomicExchangeAcquire (T volatile *pVar, T value)
  {
    T retval = *pVar;
    T oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangeAcquire (pVar, oldval, value);
      }
    while (retval != oldval);
    return retval;
  }

  template<typename T>
  static inline
  T
  atomicExchangeRelease (T volatile *pVar, T value)
  {
    T retval = *pVar;
    T oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangeRelease (pVar, oldval, value);
      }
    while (retval != oldval);
    return retval;
  }

  //
  // Increment
  //
  template<typename T>
  static inline
  T
  atomicIncrement (T volatile *pVar)
  {
    return atomicExchangeAdd (pVar, T (1));
  }

  template<typename T>
  static inline
  T
  atomicIncrementAcquire (T volatile *pVar)
  {
    return atomicExchangeAddAcquire (pVar, T (1));
  }

  template<typename T>
  static inline
  T
  atomicIncrementRelease (T volatile *pVar)
  {
    return atomicExchangeAddRelease (pVar, T (1));
  }

  //
  // Decrement
  //
  template<typename T>
  static inline
  T
  atomicDecrement (T volatile *pVar)
  {
    return atomicExchangeAdd (pVar, T (-1));
  }

  template<typename T>
  static inline
  T
  atomicDecrementAcquire (T volatile *pVar)
  {
    return atomicExchangeAddAcquire (pVar, T (-1));
  }

  template<typename T>
  static inline
  T
  atomicDecrementRelease (T volatile *pVar)
  {
    return atomicExchangeAddRelease (pVar, T (-1));
  }
} // end of namespace harnix

namespace harnix
{
#if !defined(HNX_EXCHANGEADDPTR_DEFINED)
#define HNX_EXCHANGEADDPTR_DEFINED
  //
  // ExchangeAddPtr
  //
  static inline
  uintptr_t
  atomicExchangeAddPtr (uintptr_t volatile *pVar, uintptr_t value)
  {
    uintptr_t retval = *pVar;
    uintptr_t oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangePtr (pVar, oldval, oldval + value);
      }
    while (retval != oldval);
    return retval;
  }

  static inline
  uintptr_t
  atomicExchangeAddPtrAcquire (uintptr_t volatile *pVar, uintptr_t value)
  {
    uintptr_t retval = *pVar;
    uintptr_t oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangePtrAcquire (pVar, oldval, oldval + value);
      }
    while (retval != oldval);
    return retval;
  }

  static inline
  uintptr_t
  atomicExchangeAddPtrRelease (uintptr_t volatile *pVar, uintptr_t value)
  {
    uintptr_t retval = *pVar;
    uintptr_t oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangePtrRelease (pVar, oldval, oldval + value);
      }
    while (retval != oldval);
    return retval;
  }
#endif


  //
  // ExchangePtr
  //
  static inline
  uintptr_t
  atomicExchangePtr (uintptr_t volatile *pVar, uintptr_t value)
  {
    uintptr_t retval = *pVar;
    uintptr_t oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangePtr (pVar, oldval, value);
      }
    while (retval != oldval);
    return retval;
  }

  static inline
  uintptr_t
  atomicExchangePtrAcquire (uintptr_t volatile *pVar, uintptr_t value)
  {
    uintptr_t retval = *pVar;
    uintptr_t oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangePtrAcquire (pVar, oldval, value);
      }
    while (retval != oldval);
    return retval;
  }

  static inline
  uintptr_t
  atomicExchangePtrRelease (uintptr_t volatile *pVar, uintptr_t value)
  {
    uintptr_t retval = *pVar;
    uintptr_t oldval;

    do
      {
        oldval = retval;
        retval = atomicCompareExchangePtrRelease (pVar, oldval, value);
      }
    while (retval != oldval);
    return retval;
  }

  //
  // IncrementPtr
  //
  static inline
  uintptr_t
  atomicIncrementPtr (uintptr_t volatile *pVar)
  {
    return atomicExchangeAddPtr (pVar, uintptr_t (1));
  }

  static inline
  uintptr_t
  atomicIncrementPtrAcquire (uintptr_t volatile *pVar)
  {
    return atomicExchangeAddPtrAcquire (pVar, uintptr_t (1));
  }

  static inline
  uintptr_t
  atomicIncrementPtrRelease (uintptr_t volatile *pVar)
  {
    return atomicExchangeAddPtrRelease (pVar, uintptr_t (1));
  }

  //
  // DecrementPtr
  //
  static inline
  uintptr_t
  atomicDecrementPtr (uintptr_t volatile *pVar)
  {
    return atomicExchangeAddPtr (pVar, uintptr_t (-1));
  }

  static inline
  uintptr_t
  atomicDecrementPtrAcquire (uintptr_t volatile *pVar)
  {
    return atomicExchangeAddPtrAcquire (pVar, uintptr_t (-1));
  }

  static inline
  uintptr_t
  atomicDecrementPtrRelease (uintptr_t volatile *pVar)
  {
    return atomicExchangeAddPtrRelease (pVar, uintptr_t (-1));
  }

  //
  // CompareExchangePtr
  //
  static inline
  void *
  atomicCompareExchangePtr (void *volatile *addr, void *oldval, void *newval)
  {
    return (void *)atomicCompareExchangePtr ((uintptr_t volatile *)addr,
                                             (uintptr_t)oldval, (uintptr_t)newval);
  }

  static inline
  void *
  atomicCompareExchangePtrAcquire (void *volatile *addr, void *oldval, void *newval)
  {
    return (void *)atomicCompareExchangePtrAcquire (
             (uintptr_t volatile *)addr, (uintptr_t)oldval, (uintptr_t)newval);
  }

  static inline
  void *
  atomicCompareExchangePtrRelease (void *volatile *addr, void *oldval, void *newval)
  {
    return (void *)atomicCompareExchangePtrRelease (
             (uintptr_t volatile *)addr, (uintptr_t)oldval, (uintptr_t)newval);
  }

  //
  // ExchangeAddPtr
  //
  static inline
  void *
  atomicExchangeAddPtr (void *volatile *pVar, void *value)
  {
    return (void *)atomicExchangeAddPtr (
             (uintptr_t volatile *)pVar, (uintptr_t)value);
  }

  static inline
  void *
  atomicExchangeAddPtrAcquire (void *volatile *pVar, void *value)
  {
    return (void *)atomicExchangeAddPtrAcquire (
             (uintptr_t volatile *)pVar, (uintptr_t)value);
  }

  static inline
  void *
  atomicExchangeAddPtrRelease (void *volatile *pVar, void *value)
  {
    return (void *)atomicExchangeAddPtrRelease (
             (uintptr_t volatile *)pVar, (uintptr_t)value);
  }

  //
  // ExchangePtr
  //
  static inline
  void *
  atomicExchangePtr (void *volatile *pVar, void *value)
  {
    return (void *)atomicExchangePtr (
             (uintptr_t volatile *)pVar, (uintptr_t)value);
  }

  static inline
  void *
  atomicExchangePtrAcquire (void *volatile *pVar, void *value)
  {
    return (void *)atomicExchangePtrAcquire (
             (uintptr_t volatile *)pVar, (uintptr_t)value);
  }

  static inline
  void *
  atomicExchangePtrRelease (void *volatile *pVar, void *value)
  {
    return (void *)atomicExchangePtrRelease (
             (uintptr_t volatile *)pVar, (uintptr_t)value);
  }

  //
  // IncrementPtr
  //
  static inline
  void *
  atomicIncrementPtr (void *volatile *pVar)
  {
    return (void *)atomicIncrementPtr ((uintptr_t volatile *)pVar);
  }

  static inline
  void *
  atomicIncrementPtrAcquire (void *volatile *pVar)
  {
    return (void *)atomicIncrementPtrAcquire ((uintptr_t volatile *)pVar);
  }

  static inline
  void *
  atomicIncrementPtrRelease (void *volatile *pVar)
  {
    return (void *)atomicIncrementPtrRelease ((uintptr_t volatile *)pVar);
  }

  //
  // DecrementPtr
  //
  static inline
  void *
  atomicDecrementPtr (void *volatile *pVar)
  {
    return (void *)atomicDecrementPtr ((uintptr_t volatile *)pVar);
  }

  static inline
  void *
  atomicDecrementPtrAcquire (void *volatile *pVar)
  {
    return (void *)atomicDecrementPtrAcquire ((uintptr_t volatile *)pVar);
  }

  static inline
  void *
  atomicDecrementPtrRelease (void *volatile *pVar)
  {
    return (void *)atomicDecrementPtrRelease ((uintptr_t volatile *)pVar);
  }
} // end of namespace harnix
