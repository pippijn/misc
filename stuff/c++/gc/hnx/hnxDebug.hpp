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
//  ASSERT
//
#include <assert.h>

#ifndef ASSERT
#define ASSERT                      assert
#endif

#if defined(_DEBUG)
namespace harnix
{
  class _CDebugPrint
  {
    char *m_file;
    int m_line;

  public:
    _CDebugPrint (char *file, int line)
      : m_file (file)
      , m_line (line)
    {
    }

    void
    format (const char *fmt, ...)
    {
      va_list ap;
      static bool s_bInited = false;
      static bool s_bNoLF = false;
      static bool s_bRaw = false;
      bool bRaw;
      bool bAddLF;
      char buf[512];
      int len = 0;

      char *p = strrchr (m_file, '\\');

      if (!p) p = strrchr (m_file, '/');

      if (fmt[0] == '^')
        {
          bRaw = true;
          fmt = fmt + 1;
        }
      else
        {
          bRaw = false;
        }

      if (bRaw && s_bRaw)
        {
          bAddLF = false;
        }
      else if (bRaw && !s_bRaw)
        {
          bAddLF = s_bNoLF;
        }
      else if (!bRaw && s_bRaw)
        {
          bAddLF = true;
        }
      else
        {
          bAddLF = s_bNoLF;
        }

      if (!s_bInited)
        {
          s_bInited = true;
          bAddLF = false;
        }

      if (bAddLF)
        {
          len += sprintf (buf + len, "\n");
        }

      if (!bRaw)
        {
          len += sprintf (buf + len, "%s:%u - ", p ? p + 1 : m_file, m_line);
        }

      va_start (ap, fmt);
      len += vsprintf (buf + len, fmt, ap);
      va_end (ap);

      if (buf[len - 1] == '\n')
        {
          s_bNoLF = false;
        }
      else
        {
          s_bNoLF = true;
        }
      s_bRaw = bRaw;

      fputs (buf, stdout);
    }
  };
} // namespace harnix

#define HNX_DPRINTF                                     _CDebugPrint (__FILE__, __LINE__).format
// Example: DPRINTF(fmt, ...);

#else

#if defined(_MSC_VER) && _MSC_VER >= 1400
#define HNX_DPRINTF(...)

#elif defined(__GNUC__)
#define HNX_DPRINTF(x...)

#else
static inline void
dummyprintf (const char *fmt, ...)
{
}

#define HNX_DPRINTF             dummyprintf

#endif


#endif
