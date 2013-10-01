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
// hnxLite - A light-weight version of harnix types
//
#ifndef INCLUDE_HNXLITE_72002902738F
#define INCLUDE_HNXLITE_72002902738F

#if defined(_MSC_VER)
#define HNX_CONFIG_FASTCALL

#if defined(_WIN64)
#define     __cdecl
#define     __stdcall
#define     __fastcall
#undef HNX_CONFIG_FASTCALL
#endif

#if defined(_MANAGED)
#define     __fastcall
#undef HNX_CONFIG_FASTCALL
#endif

#define HNX_DECLSPEC_NOINLINE           __declspec (noinline)
#endif

#if defined(__GNUC__)
#define __cdecl
#define __stdcall
#define __fastcall

#define HNX_DECLSPEC_NOINLINE           __attribute__ ((noinline))

#endif

//
// placement new
//
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void *__cdecl
operator new (size_t, void *place) throw()
{
  return place;
}

inline void __cdecl
operator delete (void *, void *) throw()
{
}

#endif /* __PLACEMENT_NEW_INLINE */

#ifndef __PLACEMENT_VEC_NEW_INLINE
#define __PLACEMENT_VEC_NEW_INLINE
inline void *__cdecl operator new
[] (size_t, void *place) throw()
{
  return place;
}

inline void __cdecl operator delete
[] (void *, void *) throw()
{
}

#endif /* __PLACEMENT_VEC_NEW_INLINE */

namespace harnix
{
  //
  // Null Pointer
  //
#if defined(_MANAGED)
  // using visual C++ nullptr keyword
#else
  class nullptr_t
  {
  public:
    template<typename T>
    operator T
    * () const
    {
      return 0;
    }

    template<typename T, typename U>
    operator T
    U ::* () const
    {
      return 0;
    }

    void *
    operator & () const
    {
      return 0;
    }

  private:
    char m_aBadData[0x1234];
  };
  static const nullptr_t &nullptr = *(nullptr_t *)0;
#endif
}

#endif // INCLUDE_HNXLITE_72002902738F
