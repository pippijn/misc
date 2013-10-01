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


// GNU C++

#if !defined(_W64)
#define _W64
#endif

#ifndef _SIZE_T_DEFINED
#ifdef  _HNX_64BIT
typedef unsigned long long size_t;
#else
typedef _W64 unsigned int size_t;
#endif
#define _SIZE_T_DEFINED
#endif

#ifndef _INTPTR_T_DEFINED
#ifdef  _HNX_64BIT
typedef long long intptr_t;
#else
typedef _W64 int intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif

#ifndef _UINTPTR_T_DEFINED
#ifdef  _HNX_64BIT
typedef unsigned long long uintptr_t;
#else
typedef _W64 unsigned int uintptr_t;
#endif
#define _UINTPTR_T_DEFINED
#endif

#ifndef _PTRDIFF_T_DEFINED
#ifdef  _HNX_64BIT
typedef long long ptrdiff_t;
#else
typedef _W64 int ptrdiff_t;
#endif
#define _PTRDIFF_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


//
// define types that have the same size on different platforms
//
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed long long int64;


//
//  define types that may have different size on various platforms
//
typedef unsigned int uint;
// small unsigned integer, fast, only guarantee <= 32 bits storage
// even on 64-bit platform


// <system types>
//	-- size_t, intptr_t, uintptr_t, ptrdiff_t
//
// integer of the same size as a pointer type. i.e. 32 bits with
// 32-bit compiler and 64 bits with 64-bit compiler


//
// __cdecl
//
#define __cdecl
#define __stdcall
#define __fastcall

//
// noinline
//
#define HNX_DECLSPEC_NOINLINE           __attribute__ ((noinline))


//
//  C++ header file <new> has been included
//
#if defined(__NEW__)
#define __PLACEMENT_NEW_INLINE
#define __PLACEMENT_VEC_NEW_INLINE
#endif
