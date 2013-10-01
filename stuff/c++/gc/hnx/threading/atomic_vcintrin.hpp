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



#if defined(_HNX_32BIT)
#include <intrin.h>
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedExchangeAdd)
#if defined(_M_IA64)
#pragma intrinsic (_InterlockedCompareExchange_acq)
#pragma intrinsic (_InterlockedCompareExchange_rel)
#pragma intrinsic (_InterlockedExchangeAdd_acq)
#pragma intrinsic (_InterlockedExchangeAdd_rel)
#else
#define _InterlockedCompareExchange_acq         _InterlockedCompareExchange
#define _InterlockedCompareExchange_rel         _InterlockedCompareExchange
#define _InterlockedExchangeAdd_acq                     _InterlockedExchangeAdd
#define _InterlockedExchangeAdd_rel                     _InterlockedExchangeAdd
#endif

namespace harnix
{
  static inline
  uint32
  atomicCompareExchange (uint32 volatile *addr, uint32 oldval, uint32 newval)
  {
    return (uint32)_InterlockedCompareExchange (
             (long volatile *)addr, (long)oldval, (long)newval);
  }

  static inline
  uint32
  atomicCompareExchangeAcquire (uint32 volatile *addr, uint32 oldval, uint32 newval)
  {
    return (uint32)_InterlockedCompareExchange_acq (
             (long volatile *)addr, (long)oldval, (long)newval);
  }

  static inline
  uint32
  atomicCompareExchangeRelease (uint32 volatile *addr, uint32 oldval, uint32 newval)
  {
    return (uint32)_InterlockedCompareExchange_rel (
             (long volatile *)addr, (long)oldval, (long)newval);
  }

  static inline
  uint32
  atomicExchangeAdd (uint32 volatile *addr, uint32 value)
  {
    return (uint32)_InterlockedExchangeAdd (
             (long volatile *)addr, (long)value);
  }

  static inline
  uint32
  atomicExchangeAddAcquire (uint32 volatile *addr, uint32 value)
  {
    return (uint32)_InterlockedExchangeAdd_acq (
             (long volatile *)addr, (long)value);
  }

  static inline
  uint32
  atomicExchangeAddRelease (uint32 volatile *addr, uint32 value)
  {
    return (uint32)_InterlockedExchangeAdd_rel (
             (long volatile *)addr, (long)value);
  }

  static inline
  uintptr_t
  atomicCompareExchangePtr (uintptr_t volatile *addr, uintptr_t oldval, uintptr_t newval)
  {
    return (uintptr_t)_InterlockedCompareExchange (
             (long volatile *)addr, (long)oldval, (long)newval);
  }

  static inline
  uintptr_t
  atomicCompareExchangePtrAcquire (uintptr_t volatile *addr, uintptr_t oldval, uintptr_t newval)
  {
    return (uintptr_t)_InterlockedCompareExchange_acq (
             (long volatile *)addr, (long)oldval, (long)newval);
  }

  static inline
  uintptr_t
  atomicCompareExchangePtrRelease (uintptr_t volatile *addr, uintptr_t oldval, uintptr_t newval)
  {
    return (uintptr_t)_InterlockedCompareExchange_rel (
             (long volatile *)addr, (long)oldval, (long)newval);
  }

  static inline
  uintptr_t
  atomicExchangeAddPtr (uintptr_t volatile *addr, uintptr_t value)
  {
    return (uintptr_t)_InterlockedExchangeAdd (
             (long volatile *)addr, (long)value);
  }

#define HNX_EXCHANGEADDPTR_DEFINED

  static inline
  uintptr_t
  atomicExchangeAddPtrAcquire (uintptr_t volatile *addr, uintptr_t value)
  {
    return (uintptr_t)_InterlockedExchangeAdd_acq (
             (long volatile *)addr, (long)value);
  }

  static inline
  uintptr_t
  atomicExchangeAddPtrRelease (uintptr_t volatile *addr, uintptr_t value)
  {
    return (uintptr_t)_InterlockedExchangeAdd_rel (
             (long volatile *)addr, (long)value);
  }
} // end of namespace harnix

#if !defined(_M_IA64)
#undef _InterlockedCompareExchange_acq
#undef _InterlockedCompareExchange_rel
#undef _InterlockedExchangeAdd_acq
#undef _InterlockedExchangeAdd_rel
#endif

#endif

#if defined(_HNX_64BIT)
#include <intrin.h>
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedExchangeAdd)
#pragma intrinsic (_InterlockedCompareExchange64)
#pragma intrinsic (_InterlockedExchangeAdd64)
#if defined(_M_IA64)
#pragma intrinsic (_InterlockedCompareExchange_acq)
#pragma intrinsic (_InterlockedCompareExchange_rel)
#pragma intrinsic (_InterlockedExchangeAdd_acq)
#pragma intrinsic (_InterlockedExchangeAdd_rel)
#pragma intrinsic (_InterlockedCompareExchange64_acq)
#pragma intrinsic (_InterlockedCompareExchange64_rel)
#pragma intrinsic (_InterlockedExchangeAdd64_acq)
#pragma intrinsic (_InterlockedExchangeAdd64_rel)
#else
#define _InterlockedCompareExchange_acq                         _InterlockedCompareExchange
#define _InterlockedCompareExchange_rel                         _InterlockedCompareExchange
#define _InterlockedExchangeAdd_acq                                     _InterlockedExchangeAdd
#define _InterlockedExchangeAdd_rel                                     _InterlockedExchangeAdd
#define _InterlockedCompareExchange64_acq                       _InterlockedCompareExchange64
#define _InterlockedCompareExchange64_rel                       _InterlockedCompareExchange64
#define _InterlockedExchangeAdd64_acq                           _InterlockedExchangeAdd64
#define _InterlockedExchangeAdd64_rel                           _InterlockedExchangeAdd64
#endif

namespace harnix
{
  static inline
  uint32
  atomicCompareExchange (uint32 volatile *addr, uint32 oldval, uint32 newval)
  {
    return (uint32)_InterlockedCompareExchange (
             (long volatile *)addr, (long)oldval, (long)newval);
  }

  static inline
  uint32
  atomicCompareExchangeAcquire (uint32 volatile *addr, uint32 oldval, uint32 newval)
  {
    return (uint32)_InterlockedCompareExchange_acq (
             (long volatile *)addr, (long)oldval, (long)newval);
  }

  static inline
  uint32
  atomicCompareExchangeRelease (uint32 volatile *addr, uint32 oldval, uint32 newval)
  {
    return (uint32)_InterlockedCompareExchange_rel (
             (long volatile *)addr, (long)oldval, (long)newval);
  }

  static inline
  uint32
  atomicExchangeAdd (uint32 volatile *addr, uint32 value)
  {
    return (uint32)_InterlockedExchangeAdd (
             (long volatile *)addr, (long)value);
  }

  static inline
  uint32
  atomicExchangeAddAcquire (uint32 volatile *addr, uint32 value)
  {
    return (uint32)_InterlockedExchangeAdd_acq (
             (long volatile *)addr, (long)value);
  }

  static inline
  uint32
  atomicExchangeAddRelease (uint32 volatile *addr, uint32 value)
  {
    return (uint32)_InterlockedExchangeAdd_rel (
             (long volatile *)addr, (long)value);
  }

  static inline
  uintptr_t
  atomicCompareExchangePtr (uintptr_t volatile *addr, uintptr_t oldval, uintptr_t newval)
  {
    return (uintptr_t)_InterlockedCompareExchange64 (
             (__int64 volatile *)addr, (__int64)oldval, (__int64)newval);
  }

  static inline
  uintptr_t
  atomicCompareExchangePtrAcquire (uintptr_t volatile *addr, uintptr_t oldval, uintptr_t newval)
  {
    return (uintptr_t)_InterlockedCompareExchange64_acq (
             (__int64 volatile *)addr, (__int64)oldval, (__int64)newval);
  }

  static inline
  uintptr_t
  atomicCompareExchangePtrRelease (uintptr_t volatile *addr, uintptr_t oldval, uintptr_t newval)
  {
    return (uintptr_t)_InterlockedCompareExchange64_rel (
             (__int64 volatile *)addr, (__int64)oldval, (__int64)newval);
  }

  static inline
  uintptr_t
  atomicExchangeAddPtr (uintptr_t volatile *addr, uintptr_t value)
  {
    return (uintptr_t)_InterlockedExchangeAdd64 (
             (__int64 volatile *)addr, (__int64)value);
  }

#define HNX_EXCHANGEADDPTR_DEFINED

  static inline
  uintptr_t
  atomicExchangeAddPtrAcquire (uintptr_t volatile *addr, uintptr_t value)
  {
    return (uintptr_t)_InterlockedExchangeAdd64_acq (
             (__int64 volatile *)addr, (__int64)value);
  }

  static inline
  uintptr_t
  atomicExchangeAddPtrRelease (uintptr_t volatile *addr, uintptr_t value)
  {
    return (uintptr_t)_InterlockedExchangeAdd64_rel (
             (__int64 volatile *)addr, (__int64)value);
  }
} // end of namespace harnix
#endif

#include "atomic_gen.hpp"
