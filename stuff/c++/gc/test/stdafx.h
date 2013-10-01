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


// stdafx.h : system header files for testsuits

// Features
#if defined(__GNUC__)
#define _XOPEN_SOURCE           500
#endif

#define _CRT_SECURE_NO_DEPRECATE
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_MSC_VER) && !defined(UNDER_CE)
#define _CRTDBG_MAP_ALLOC       // enable source filename and line number in memory leak dump
#include <crtdbg.h>                     // enable memory leak dump
#endif // UNDER_CE


#if defined(_MSC_VER)

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <new>
#include <windows.h>

#elif defined(__GNUC__)
#include <new>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define __PLACEMENT_NEW_INLINE
#define __PLACEMENT_VEC_NEW_INLINE

#endif
