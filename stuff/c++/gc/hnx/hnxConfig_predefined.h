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


#if PLATFORM_PRECONFIG == PLATFORM_PRECONFIG_WIN32
// #define	HNXGC_CONFIG_UNALIGNACCESS
#define _CRT_SECURE_NO_DEPRECATE
#include <assert.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#elif PLATFORM_PRECONFIG == PLATFORM_PRECONFIG_UNIX
// #define	HNXGC_CONFIG_UNALIGNACCESS
#include <new>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HNXGC_CONFIG_NODLL

#elif PLATFORM_PRECONFIG == PLATFORM_PRECONFIG_STDALONE
#define _CRT_SECURE_NO_DEPRECATE
#include <memory.h>
#include <stdio.h>
#define HNX_SINGLE_THREADED

#endif
