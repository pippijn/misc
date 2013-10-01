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


#include "stdafx.h"

#include "hnxGC_MT.h"

#if defined(__SMP__)

#if defined(_WINBASE_)

namespace harnix
{
  class CGlobalMemoryFence
  {
  public:
    DWORD m_nNumberOfProcessors;
    DWORD_PTR m_nAffinityMask;
    HANDLE             *m_aThreadHandle;
    HANDLE m_hEventD1;
    HANDLE m_hEventD2;
    HANDLE m_hEventGMF;
    volatile LONG m_nPendingAMF;
    CRITICAL_SECTION m_CriticalSection;
    bool m_bQuit;

    CGlobalMemoryFence ();
    ~CGlobalMemoryFence ();
    void CallGMF ();
    static DWORD WINAPI DThreadProc (LPVOID);
  };

  static CGlobalMemoryFence s_gmf;

  void
  _GlobalMemoryFence ()
  {
    s_gmf.CallGMF ();
  }

  CGlobalMemoryFence::CGlobalMemoryFence ()
  {
    {
      // get affinity mask of the current process
      DWORD_PTR nProcessAffinityMask, nSystemAffinityMask;
      GetProcessAffinityMask (GetCurrentProcess (), &nProcessAffinityMask, &nSystemAffinityMask);

      m_nAffinityMask = nProcessAffinityMask;
    }

    {
      // calculate the number of processors from the affinity mask
      DWORD nProcessorMask;
      for (m_nNumberOfProcessors = 0, nProcessorMask = 1;
           nProcessorMask <= m_nAffinityMask;
           nProcessorMask <<= 1)
        {
          if ((nProcessorMask & m_nAffinityMask) != 0)
            {
              m_nNumberOfProcessors++;
            }
        }
    }

    {
      // setup Event Objects
      m_hEventD1 = CreateEvent (NULL, TRUE, FALSE, NULL);
      m_hEventD2 = CreateEvent (NULL, TRUE, FALSE, NULL);
      m_hEventGMF = CreateEvent (NULL, TRUE, FALSE, NULL);
      InitializeCriticalSection (&m_CriticalSection);

      m_nPendingAMF = -1;
      m_bQuit = false;
    }

    {
      // create D threads
      m_aThreadHandle = new HANDLE[m_nNumberOfProcessors];
      DWORD dwThreadAffinityMask = 1;
      for (DWORD i = 0; i < m_nNumberOfProcessors; ++i)
        {
          HANDLE hThread;
          DWORD nThreadId;
          hThread = ::CreateThread (NULL, 0, DThreadProc, this, 0, &nThreadId);
          m_aThreadHandle[i] = hThread;
          while ((dwThreadAffinityMask & m_nAffinityMask) == 0)
            {
              ASSERT ((dwThreadAffinityMask > m_nAffinityMask));
              dwThreadAffinityMask <<= 1;
            }
          SetThreadAffinityMask (hThread, dwThreadAffinityMask);
        }
    }
  }

  CGlobalMemoryFence::~CGlobalMemoryFence ()
  {
    // wait for all D threads to quit
    m_bQuit = true;
    SetEvent (this->m_hEventD1);

    while (1)
      {
        DWORD rc = WaitForMultipleObjects (
          m_nNumberOfProcessors,
          m_aThreadHandle,
          TRUE,
          INFINITE);
        if (                    /* WAIT_OBJECT_0 <= rc && */
          rc < WAIT_OBJECT_0 + m_nNumberOfProcessors)
          {
            break;
          }
      }

    // release all resources
    CloseHandle (m_hEventD1);
    CloseHandle (m_hEventD2);
    CloseHandle (m_hEventGMF);
    DeleteCriticalSection (&m_CriticalSection);
    for (DWORD i = 0; i < m_nNumberOfProcessors; i++)
      {
        CloseHandle (m_aThreadHandle[i]);
        delete[] m_aThreadHandle;
      }
  }

  DWORD WINAPI
  CGlobalMemoryFence::DThreadProc (LPVOID lpParameter)
  // DThread - D Threads' main routine
  {
    CGlobalMemoryFence *pGMF = (CGlobalMemoryFence *)lpParameter;

    while (1)
      {
        if (WaitForSingleObject (pGMF->m_hEventD1, INFINITE) == WAIT_OBJECT_0)
          {
            if (pGMF->m_bQuit)
              {
                break;
              }
            if (InterlockedDecrement (&pGMF->m_nPendingAMF) == 0)
              {
                SetEvent (pGMF->m_hEventGMF);
              }
            // wait for the ready of environment for next AMF
            while (WaitForSingleObject (pGMF->m_hEventD2, INFINITE) != WAIT_OBJECT_0)
              {
                ;                         // do nothing
              }
          }
      }
    return 0;
  }

  void
  CGlobalMemoryFence::CallGMF ()
  // CallGMF - GMF service routine
  {
    EnterCriticalSection (&m_CriticalSection);
    ResetEvent (m_hEventD2);                                // prevent racing for D1
    ResetEvent (m_hEventGMF);                               // prepare to wait
    m_nPendingAMF = m_nNumberOfProcessors;
    MemoryFence ();
    SetEvent (m_hEventD1);                                  // wake up D thread for AMF
    while (WaitForSingleObject (m_hEventGMF, INFINITE) != WAIT_OBJECT_0)
      {
        ;
      }
    ResetEvent (m_hEventD1);                                // prepare for next AMF
    MemoryFence ();
    SetEvent (m_hEventD2);                                  // allow D thread go next AMF
    LeaveCriticalSection (&m_CriticalSection);
  }
} // end of namespace harnix


#elif (defined(__i386__) || defined(_M_IX86))
// no need for GMF

#else // _WINBASE_

#if defined(_MSC_VER)
#pragma message(__FILE__ ": hnxGMF: need external program of GlobalMemoryFence support.")
#else
#warning "hnxGMF: need external program of GlobalMemoryFence support."
#endif

namespace harnix
{
  void
  _GlobalMemoryFence ()
  {
    // invoke external global memory fence support
    system ("hnxgmf");
  }
} // end of namespace harnix
#endif

#endif // __SMP__
