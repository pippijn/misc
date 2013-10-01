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

//
// Macros
//
#define HNX_NUMELE(a)                           (sizeof (a) / sizeof ((a)[0]))

namespace harnix
{
  //
  // type aliases
  //
  typedef int8 sbyte;
  typedef uint8 byte;
  typedef uint16 ushort;


  //
  // Null Pointer
  //
#if defined(_MANAGED)
  // using visual C++ nullptr keyword
#else
  class nullptr_t;
  static const nullptr_t &nullptr = *(nullptr_t *)0;
  class nullptr_t
  {
  public:
    template<typename T>
    operator T
    * () const
    {
      return (T *)&nullptr;
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
#endif


  //
  // ValueInRange(a, x, b) - return true, if x is in range [ a, b ]
  //
  template<typename T>
  static inline
  bool
  ValueInRange (T a, T x, T b)
  {
    return x >= a && x <= b;
  }

  //
  // ValueInside(x, start, n) - return true, if x is in range [ start, start+n )
  //
  template<typename T>
  static inline
  bool
  ValueInside (T x, T start, size_t size)
  {
    return x >= start && x < start + size;
  }

  //
  // OffsetOf - return the offset of a member from the beginning of its structure
  //
  template<typename Y, typename T>
  static inline
  size_t
  OffsetOf (Y T::*field)
  {
    return reinterpret_cast<size_t> (&(reinterpret_cast<T *> (0)->*field));
  }

  //
  // ContainingRecord - return structure pointer from a pointer to its field
  //
  template<typename Y, typename T>
  static inline
  T *
  ContainingRecord (Y *ptr, Y T::*field)
  {
    return reinterpret_cast<T *> (reinterpret_cast<char *> (ptr) - OffsetOf (field));
  }

  // example:
  //  CFoo *pFoo = ContainingRecord(ptr, &CFoo::m_nCount);
  //  (ptr - points to the m_nCount field of an instance of CFoo)


  template<typename T, typename U, U T::*Field>
  class ContainingRecorderConverter
  {
  public:
    static T *
    GetContainer (U *p)
    {
      return ContainingRecord (p, Field);
    }

    static U *
    GetField (T *p)
    {
      return &(p->*Field);
    }
  };


  //
  //  PointerAdjust - adjust the value of pointer without changing the type
  //
  template<typename T>
  static inline
  T *
  PointerAdjust (T *ptr, size_t adj)
  {
    if (ptr)
      {
        return reinterpret_cast<T *> (reinterpret_cast<char *> (ptr) + adj);
      }
    else
      {
        return ptr;
      }
  }

  //
  //  PointerDiff - calculate the distance in bytes between two address
  //
  static inline
  size_t
  PointerDiff (void *ptr1, void *ptr2)
  {
    return reinterpret_cast<char *> (ptr1) - reinterpret_cast<char *> (ptr2);
  }

  //
  //  PointerInside - determine whether the pointer is inside the ranage
  //
  static inline
  bool
  PointerInside (void *ptr, void *start, size_t size)
  {
    return start <= ptr && ptr < (char *)start + size;
  }

  //
  //  PointerAlignHigh - adjust `ptr' to upper boundary of `align'
  //
  template<typename T>
  static inline
  T *
  PointerAlignLow (T *ptr, size_t align)
  {
    size_t addr = (size_t)ptr;

    return (T *)(addr & ~(align - 1));
  }

  //
  //  PointerAlignHigh - adjust `ptr' to upper boundary of `align'
  //
  template<typename T>
  static inline
  T *
  PointerAlignHigh (T *ptr, size_t align)
  {
    size_t addr = (size_t)ptr;

    return (T *)((addr + align - 1) & ~(align - 1));
  }

  //
  //  PointerAndMask - bitwise `AND' an integer with the pointer
  //
  template<typename T>
  static inline
  T *
  PointerAndMask (T *ptr, uintptr_t mask)
  {
    uintptr_t addr = (uintptr_t)ptr;

    addr &= mask;
    return (T *)addr;
  }

  //
  //  PointerOrMask - bitwise `OR' an integer with the pointer
  //
  template<typename T>
  static inline
  T *
  PointerOrMask (T *ptr, uintptr_t mask)
  {
    uintptr_t addr = (uintptr_t)ptr;

    addr |= mask;
    return (T *)addr;
  }
} // end of namespace harnix
