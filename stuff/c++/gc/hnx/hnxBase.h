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
// hnxBase.h -- Basis Definitions
//
#ifndef INCLUDE_HNXBASE_6A0820E0B69C
#define INCLUDE_HNXBASE_6A0820E0B69C

// this header file covers all in the hnxLite.h
#define INCLUDE_HNXLITE_72002902738F

//
//	Configuration
//
#include "hnx/hnxConfig.h"

//
//	Platform Dependent Definitions
//
#if defined(_MSC_VER)
#include "hnx/hnxMSVC.hpp"
#endif

#if defined(__GNUC__)
#include "hnx/hnxGNU.hpp"
#endif

//
//  Debug
//
#include "hnx/hnxDebug.hpp"


//
//  Base Helper
//
#include "hnx/hnxBaseHlp.hpp"


//
//  Multi-threading
//
#include "hnx/hnxMT.hpp"


#endif // INCLUDE_HNXBASE_6A0820E0B69C
