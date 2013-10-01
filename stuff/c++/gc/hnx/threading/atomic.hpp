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
//  atomic_CompareExchange / atomic_ExchangeAdd on uint_ptr
//

#if defined(HNX_SINGLE_THREADED)
// single threading version

#define volatile

#include "atomic_st.hpp"


#else // multi-threading on multi-processors / uni-processor

//
//	Visual C++
//
#if defined(_MSC_VER)
#if defined(_M_IX86) && defined(_WIN32)
// x86 32-bit inline asm
#include "atomic_x86asm.hpp"

#else
// based on MS intrinsic functions 16/32/64-bit
#include "atomic_vcintrin.hpp"

#endif // _MSC_VER


//
//  GNU C++
//
#elif defined(__GNUC__)
#if defined(__i386__)
#include "atomic_gnux86.hpp"
#else
#error You must implement the atomic_* functions for your CPU
#endif

#else
#error You must implement the atomic_* functions for your compiler
#endif  // _MSC_VER, __GNUC__

#endif // HNX_SINGLE_THREADED

/*
   atomicCompareExchange, atomicCompareExchangeAcquire, atomicCompareExchangeRelease
   atomicCompareExchangePtr, atomicCompareExchangePtrAcquire, atomicCompareExchangePtrRelease

   atomicExchangeAdd, atomicExchangeAddAcquire, atomicExchangeAddRelease
   atomicExchangeAddPtr, atomicExchangeAddPtrAcquire, atomicExchangeAddPtrRelease

   atomicExchange, atomicExchangeAcquire
   atomicExchangePtr, atomicExchangePtrAcquire

   atomicIncrement, atomicIncrementAcquire, atomicIncrementRelease, atomicIncrementNone
   atomicIncrementPtr, atomicIncrementPtrAcquire, atomicIncrementPtrRelease, atomicIncrementPtrNone

   atomicDecrement, atomicDecrementAcquire, atomicDecrementRelease, atomicDecrementNone
   atomicDecrementPtr, atomicDecrementPtrAcquire, atomicDecrementPtrRelease, atomicDecrementPtrNone

 */
