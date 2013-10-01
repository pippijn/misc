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
// hnxGC - HnxGC header for application and API
//

/*
   Note: you can (un)define some of the following configuration macros:

   HNXGC_CONFIG_NODLL - define this to use the static linkage version of library (or on Linux)

   HNXGC_CONFIG_NOSHORTHAND - define this if you want CLockedPtr<T> style instead of lp<T>.

   undef gcnew - undefine the gcnew macro to use HNXGC_NEW instead.

   (HNXGC_EXPORTS - defined this to build HnxGC Library)

 */

#ifndef INCLUDE_HNXGC_D763F5D8EBB6
#define INCLUDE_HNXGC_D763F5D8EBB6

#define HNXGC_VERSION           "HnxGC 1.0 (Alpha)"
#define HNXGC_VERSIONSIG        "D763F5D8EBB6"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#if !defined(HNXGC_EXPORTS)
// For applications that use the HnxGC

//
//	Harnix Lite
//
#include "hnx/hnxLite.h"


//
//	Configuration of HnxGC
//
#if defined(_MSC_VER) && _MSC_VER >= 1300
// Visual C++ .NET 2002
#define HNX_CONFIG__IF_EXISTS
#endif

#if defined(__GNUC__)
#define HNXGC_CONFIG_NODLL
#endif

#else
// For implemenations of HnxGC

//
//	Harnix Base
//
#include "hnx/hnxBase.h"

#endif  // HNXGC_EXPORTS


//
//	API
//
#include "hnxGC/hnxGCAPI.hpp"


//
//	Runtime ClassInfo & gcnew
//
#include "hnxGC/hnxGCBase.hpp"


//
//	Smart Pointers
//
#include "hnxGC/hnxGCSmartPtr.hpp"
// note - gcnew is defined to HNXGC_NEW by default. If you don't like it and
// want to rollback to CLR semantics, you can uncomment the following line.
//
// #undef gcnew
//


//  Application Helpers
#if !defined(HNXGC_EXPORTS)

#pragma comment(lib, "HnxGC")

namespace harnix
{
  // invoke initialization of HnxGC before main()
  class _CHnxGCInit
  {
  public:
    _CHnxGCInit ()
    {
      _hnxgc_gc_init_t param = {
        _hnxgc_gc_Init,
        HNXGC_VERSION,
        HNXGC_VERSIONSIG,
        0
      };

      _hnxgc_garbage_collect (&param);
    }

    ~_CHnxGCInit ()
    {
      _hnxgc_gc_fini_t param = {
        _hnxgc_gc_Fini,
      };

      _hnxgc_garbage_collect (&param);
    }
  };

#if (defined(__GNUC__) && __GNUC__ > 4)
  template<typename T>
  class _CHnxGCCtor
  {
  public:
    static _CHnxGCInit _;
  };
  template<typename T>
  _CHnxGCInit _CHnxGCCtor<T>::_;

  static void *_hnxgc_initref = &_CHnxGCCtor<int>::_;
#else
  static _CHnxGCInit _hnxgc_init;
#endif
}

#endif


#ifdef _MANAGED
#pragma managed(pop)
#endif

#endif // INCLUDE_HNXGC_D763F5D8EBB6
