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


#ifndef INCLUDE_HNXCONFIG_0268368BDCBC
#define INCLUDE_HNXCONFIG_0268368BDCBC

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Basis Configuration:
//
//	Single-Threading					- define HNX_SINGLE_THREADED
//  Uni-processor Multi-Threading		- <none>
//  Multi-processors Multi-Threading	- define __SMP__
//
// Predefined Configurations:			- define PLATFORM_PRECONFIG=<pre_config_name>
//		e.g. cl -DPLATFORM_PRECONFIG=PLATFORM_PRECONFIG_WIN32
//
// User-defined configuration:			- define PLATFORM_CUSTOM_CONFIG="<filename>"
//		e.g. cl -DPLATFORM_CUSTOM_CONFIG="hnxConfig_mobile.hpp"

#define PLATFORM_PRECONFIG_WIN32                1
#define PLATFORM_PRECONFIG_UNIX                 2
#define PLATFORM_PRECONFIG_STDALONE             3

#if defined(PLATFORM_PRECONFIG)
#include "hnxConfig_predefined.h"
#endif  // PLATFORM_PRECONFIG


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// automatic definitions of some configurations
//

#if !defined(_HNX_32BIT) && !defined(_HNX_64BIT)
#if defined(__ia64) || defined(__ia64__) || defined(_M_IA64) || \
  defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || \
  defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || \
  defined(__alpha__) || defined(_M_ALPHA)
#define _HNX_64BIT
#elif defined(__i386__) || defined(_M_IX86) || \
  defined(_M_ARM)
#define _HNX_32BIT
#endif
#endif

//
// GNU C++ Compiler (__GNUC__)
//

//
// Visual C++ Compiler (_MSC_VER)
//	1300 - Visual C++ .NET 2002 (supports: __if_exists)
//	1310 - Visual C++ .NET 2003
//  1400 - Visual C++ .NET 2005
#if defined(_MSC_VER) && _MSC_VER >= 1300
// Visual C++ .NET 2002
#define HNX_CONFIG__IF_EXISTS
#endif

#if defined(_MSC_VER) && (!defined(_WIN64) && !defined(_MANAGED))
#define HNX_CONFIG_FASTCALL
#endif


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// include user-defined configuration
//
#if defined(PLATFORM_CUSTOM_CONFIG)
#include PLATFORM_CUSTOM_CONFIG
#endif


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//
// Configuration checking
//
#if !defined(_HNX_32BIT) && !defined(_HNX_64BIT)
#error at least one of following must be defined: _HNX_32BIT or _HNX_64BIT
#endif

#if !defined(__SMP__)
// no need to handle no-naturally-aligned pointer access
#undef HNXGC_CONFIG_UNALIGNACCESS
#endif

#if defined(_MSC_VER) && (defined(_MANAGED) || defined(_WIN64))
// MS CLR does not allow __fastcall
#undef HNX_CONFIG_FASTCALL
#endif

#endif // INCLUDE_HNXCONFIG_0268368BDCBC
