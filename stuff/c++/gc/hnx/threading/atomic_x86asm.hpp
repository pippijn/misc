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
// define CompareExchange[Ptr][Acquire/Release] & ExchangeAdd[Ptr][Acquire/Release]
//
#if defined(__SMP__)
namespace harnix
{
#pragma warning (push)
#pragma warning (disable: 4793)
  static inline
  uint32
  atomicCompareExchange (uint32 volatile *addr, uint32 oldval, uint32 newval)
  {
    __asm {
      mov eax, oldval
      mov edx, newval
      mov ecx, addr
      lock cmpxchg dword ptr[ecx], edx
    }
  }

  static inline
  uint32
  atomicExchangeAdd (uint32 volatile *addr, uint32 incr)
  {
    __asm {
      mov eax, incr
      mov ecx, addr
      lock xadd dword ptr[ecx], eax
    }
  }

#pragma warning (pop)
} // end of namespace harnix
#else
namespace harnix
{
#pragma warning (push)
#pragma warning (disable: 4793)
#pragma warning (disable: 4035)
  static inline
  uint32
  atomicCompareExchange (uint32 volatile *addr, uint32 oldval, uint32 newval)
  {
    __asm {
      mov eax, oldval
      mov edx, newval
      mov ecx, addr
      cmpxchg dword ptr[ecx], edx
    }
  }

  static inline
  uint32
  atomicExchangeAdd (uint32 volatile *addr, uint32 incr)
  {
    __asm {
      mov eax, incr
      mov ecx, addr
      xadd dword ptr[ecx], eax
    }
  }

#pragma warning (pop)
} // end of namespace harnix
#endif

namespace harnix
{
  static inline
  uintptr_t
  atomicCompareExchangePtr (uintptr_t volatile *addr, uintptr_t oldval, uintptr_t newval)
  {
    return (uintptr_t)atomicCompareExchange (
             (uint32 volatile *)addr, (uint32)oldval, (uint32)newval);
  }

  static inline
  uintptr_t
  atomicExchangeAddPtr (uintptr_t volatile *addr, uintptr_t value)
  {
    return (uintptr_t)atomicExchangeAdd (
             (uint32 volatile *)addr, (uint32)value);
  }

#define HNX_EXCHANGEADDPTR_DEFINED
} // end of namespace harnix

namespace harnix
{
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

  template<typename T>
  static inline
  T
  atomicExchangeAddAcquire (T volatile *addr, T value)
  {
    return atomicExchangeAdd (addr, value);
  }

#define HNX_EXCHANGEADDACQUIRE_DEFINED

  template<typename T>
  static inline
  T
  atomicExchangeAddRelease (T volatile *addr, T value)
  {
    return atomicExchangeAdd (addr, value);
  }

#define HNX_EXCHANGEADDRELEASE_DEFINED

  template<typename T>
  static inline
  T
  atomicExchangeAddPtrAcquire (T volatile *addr, T value)
  {
    return atomicExchangeAddPtr (addr, value);
  }

#define HNX_EXCHANGEADDACQUIRE_DEFINED

  template<typename T>
  static inline
  T
  atomicExchangeAddPtrRelease (T volatile *addr, T value)
  {
    return atomicExchangeAddPtr (addr, value);
  }

#define HNX_EXCHANGEADDRELEASE_DEFINED
} // end of namespace harnix

#include "atomic_gen.hpp"
