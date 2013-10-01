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
  //	Multi-Processors
  //
#if defined(__SMP__)
  //
  // CompilerReadWriteBarrier - prevent compiler generating re-ordered code
  //
#if (defined(_MSC_VER) && (_MSC_VER >= 1400)) && (defined(_M_IX86) || defined(_M_IA64))
#include <intrin.h>
#pragma intrinsic(_ReadWriteBarrier)
  static inline
  void
  CompilerReadWriteBarrier ()
  {
    // this does not generate any processor instruction
    _ReadWriteBarrier ();
  }

#elif defined(__GNUC__)
  static inline
  void
  CompilerReadWriteBarrier ()
  {
    __asm volatile ("" ::: "memory");
  }

#else
  // generic but do introduce a little cost of function call/ret
  extern void _dummy_empty ();
  static inline
  void
  CompilerReadWriteBarrier ()
  {
    _dummy_empty ();
  }
#endif


  //
  //	Uni-processor
  //
#else
  //
  // CompilerReadWriteBarrier - prevent compiler generating re-ordered code
  //
  static inline
  void
  CompilerReadWriteBarrier ()
  {
  }
#endif


  // Memory Fence
#if defined(__SMP__)
  extern char g_dummyData[];
  static inline
  void
  MemoryFence ()
  {
    atomicExchangePtr ((uintptr_t *)g_dummyData, (uintptr_t)0);
  }

#else
  static inline void
  MemoryFence ()
  {
  }
#endif


  //
  // Global Memory Fence
  //
#if defined(__SMP__) && !(defined(__i386__) || defined(_M_IX86))
  void _GlobalMemoryFence ();
  static inline void
  GlobalMemoryFence ()
  {
    _GlobalMemoryFence ();
  }

#else
  static inline void
  GlobalMemoryFence ()
  {
  }
#endif

} // end of namespace harnix
