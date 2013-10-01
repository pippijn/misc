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


#if defined(HNX_SINGLE_THREADED)

namespace harnix
{
  // note - keyword `volatile' is redefined to nothing

  //
  // CompareExchange
  //
  template<typename T>
  static inline
  T
  atomicCompareExchange (T volatile *pVar, T oldval, T newval)
  {
    if (*pVar == oldval)
      {
        *pVar = newval;
        return oldval;
      }
    else
      {
        return *pVar;
      }
  }

  template<typename T>
  static inline
  T
  atomicCompareExchangeAcquire (T volatile *addr, T oldval, T newval)
  {
    return atomicCompareExchange (addr, oldval, newval);
  }

  template<typename T>
  static inline
  T
  atomicCompareExchangeRelease (T volatile *addr, T oldval, T newval)
  {
    return atomicCompareExchange (addr, oldval, newval);
  }
} // end of namespace harnix

namespace harnix
{
  //
  // CompareExchangePtr
  //
  static inline
  uintptr_t
  atomicCompareExchangePtr (uintptr_t volatile *addr, uintptr_t oldval, uintptr_t newval)
  {
    return (uintptr_t)atomicCompareExchange ((uint32 volatile *)addr,
                                             (uint32)oldval, (uint32)newval);
  }

  template<typename T>
  static inline
  T
  atomicCompareExchangePtrAcquire (T volatile *addr, T oldval, T newval)
  {
    return atomicCompareExchangePtr (addr, oldval, newval);
  }

  template<typename T>
  static inline
  T
  atomicCompareExchangePtrRelease (T volatile *addr, T oldval, T newval)
  {
    return atomicCompareExchangePtr (addr, oldval, newval);
  }
} // end of namespace harnix

#include "atomic_gen.hpp"

#endif  // HNX_SINGLE_THREADED
