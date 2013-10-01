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


#define HNXGC_EXPORTS

#include "hnxGC/hnxGC.h"

#include "hnx/DoubleList.hpp"
#include "hnx/SingleList.hpp"

#define DPRINTF                         HNX_DPRINTF

#define HNXGC_BUILDSIG          "HnxGC-1.0(Alpha 07613) "

#include "hnxGC_ST/hnxGCSync.hpp"

namespace harnix
{
  class CPouch;

  // CPouchBody - the place for application-level object data
  class CPouchBody
  {
  public:
    byte m_aData[0x1234];
  };

  class CPouchHeader                            // for a single object or
  {
  public:
    DListEntry m_vLnkObjects;                                           // link objects of the same color
    IRuntimeClassInfo *m_pIRtClassInfo;                         // containing entries of Traverse, Destructor, etc.
    CAtomicFlags m_nFlags;                                              // Pouch state
    CLECount m_vLECount;                                                        // L-Counter & E-Counter
    size_t m_nNumOfElem;                                                // operator new [] return this address
    CPouchBody m_vUserData;                                             // app see this address
  };

  // `m_nFlags' bit fields:
  // color:2;				0000-000x		0/1 - white/black
  // res1:6;
  // F_DESTRUCTED:1;		0000-0100
  // F_UNREACHABLE:1;		0000-0200
  // F_DEPENDENCE:1		0000-0400		0/1 - Q-Destructing/Q-Standby
  // res2:21

  static const uint_flags F_COLORMASK = 0x1;
  static const uint_flags F_DESTRUCTED = 0x100;
  static const uint_flags F_UNREACHABLE = 0x200;
  static const uint_flags F_DEPENDENCE = 0x400;

  class CPouch
    : public CPouchBody
  {
  public:
    static void *CreateInstance (IRuntimeClassInfo *pRuntimeClassInfo, bool bHasSize, size_t nNumOfElem, size_t nTotalSize);
    void traverse (void *caller = 0);
    void onreclaim (void *caller);
    void destruct ();
    void destroy ();

    CPouchHeader *
    getPouchHeader ()
    {
      return ContainingRecord ((CPouchBody *)this, &CPouchHeader::m_vUserData);
    }

    CLECount *
    getLECount ()
    {
      return &getPouchHeader ()->m_vLECount;
    }

    CAtomicFlags *
    getFlags ()
    {
      return &getPouchHeader ()->m_nFlags;
    }

    bool
    isLocked ()
    {
      return getLECount ()->isLocked ();
    }

    bool
    isDestructed ()
    {
      return getFlags ()->bitwiseTest (F_DESTRUCTED) != 0;
    }

    CPouch *
    RealPtr ()
    {
      return PointerAndMask (this, ~3);
      // Pouch is always aligned to 4 byte boundary at least
    }

    static DListEntry *
    GetObjLink (CPouch *pPouch)
    {
      return &pPouch->getPouchHeader ()->m_vLnkObjects;
    }
  };

  // CDeclarator - internal struct of the `caller' argument of OnReclaim routine
  class CDeclarator
  {
  public:
    bool *m_pResurr;                                                                    // bool for resurrection request
    CPouch *m_pOwner;                                                                   // `this' of OnReclaim method
    CPouch *m_pDepender;                                                        // `depender' (default to `this')
    CDeclarator (bool *pResurr, CPouch *pOwner, CPouch *pDepender)
      : m_pResurr (pResurr)
      , m_pOwner (pOwner)
      , m_pDepender (pDepender)
    {
    }
  };

  // CDependence - Record of dependence declaration
  class CDependence
  {
  public:
    SListEntry m_vLink;
    CPouch *m_pDepender;
    CPouch *m_pDependee;
    CDependence (CPouch *pDepender, CPouch *pDependee)
      : m_pDepender (pDepender)
      , m_pDependee (pDependee)
    {
    }
  };

  typedef DoubleLinkedList<CPouch, &CPouch::GetObjLink> ObjectList;
  typedef SingleLinkedList<CDependence,
                           SListEntryGetFromPtr<CDependence, &CDependence::m_vLink> >      DRecList;

  class CCollector
  {
  public:
    uint_flags m_nWhiteColor;                                                   // color value for `white'
    ObjectList m_aLists[2];                                                     // white & black object list
    ObjectList m_lstGray;                                                               // Gray object list (LG)
    DRecList m_lstDependences;                                                  // single list of dependence records
    bool m_bF1;                                                                         // GC is in processing
    bool m_bSkipFinalGC;                                                        // skip garbage collection at exit
    uintptr_t m_nInitCount;                                                     // # of initialization calls
    bool m_bIsFinalGC;                                                          // the final collection
    static CCollector *s_pInstance;

  public:
    CCollector ()
      : m_nWhiteColor (0)
      , m_bF1 (false)
      , m_bSkipFinalGC (false)
      , m_nInitCount (1)
      , m_bIsFinalGC (false)
    {
    }

    void
    skipFinalGC ()
    {
      m_bSkipFinalGC = true;
    }

    static void
    Initialization ()
    {
      if (s_pInstance)
        {
          s_pInstance->m_nInitCount++;
        }
      else
        {
          static char place[sizeof (CCollector) + 16];
          s_pInstance = new (place)CCollector;
        }
    }

    static void
    Termination ()
    {
      if (s_pInstance)
        {
          s_pInstance->m_nInitCount--;
          if (s_pInstance->m_nInitCount == 0)
            {
              if (!s_pInstance->m_bSkipFinalGC)
                {
                  // collect all objects as garbage
                  s_pInstance->m_bF1 = true;
                  s_pInstance->m_bIsFinalGC = true;
                  s_pInstance->m_nWhiteColor = !s_pInstance->m_nWhiteColor;
                  s_pInstance->Reclamation ();
                }
              // note - s_pInstance is deliberately not destructed for the
              // case that the final collection are skipped.
            }
        }
    }

    static CCollector &
    GetInstance ()
    {
      return *s_pInstance;
    }

    void
    setColorBlack (CPouch *pPouch)
    {
      pPouch->getFlags ()->setValue (F_COLORMASK, !m_nWhiteColor);
    }

    bool
    convertWhiteToBlack (CPouch *pPouch)
    {
      return pPouch->getFlags ()->bitwiseCompareExchange (
               F_COLORMASK, m_nWhiteColor, !m_nWhiteColor
               ) == m_nWhiteColor;
    }

    ObjectList &
    getWhiteList ()
    {
      return m_aLists[m_nWhiteColor];
    }

    ObjectList &
    getBlackList ()
    {
      return m_aLists[!m_nWhiteColor];
    }

    ObjectList &
    getGrayList ()
    {
      return m_lstGray;
    }

  public:
    void
    TraverseReport (void *, size_t n, CPouch **ppObj)
    {
      while (n--)
        {
          if (ppObj[n])
            {
              CPouch *pPouch = ppObj[n]->RealPtr ();
              if (convertWhiteToBlack (pPouch))
                {
                  getWhiteList ().remove (pPouch);
                  getGrayList ().pushTail (pPouch);
                }
            }
        }
    }

    void *
    OnReclaimChangeDepender (void *caller, CPouch *pObj)
    {
      CDeclarator *p = (CDeclarator *)caller;
      void *retval = p->m_pDepender;

      p->m_pDepender = pObj ? pObj : p->m_pOwner;
      return retval;
      // note - ChangeDepender emulates declaration of other object.
    }

    void
    OnReclaimDeclare (void *caller, size_t n, CPouch **ppObj)
    {
      CDeclarator *pDecl = reinterpret_cast<CDeclarator *> (caller);

      if (pDecl->m_pOwner == 0)
        {
          return;
        }
      CPouch *pDepender = pDecl->m_pDepender;
      for (size_t i = 0; i < n; i++)
        {
          if (ppObj[n])
            {
              CPouch *pPouch = ppObj[i]->RealPtr ();
              if (pPouch->getFlags ()->bitwiseTest (F_UNREACHABLE))
                {
                  m_lstDependences.pushNode (new CDependence (pDepender, pPouch));
                }
            }
        }
    }

    void
    OnReclaimResurrect (void *caller)
    {
      bool *pFlag = reinterpret_cast<CDeclarator *> (caller)->m_pResurr;

      if (pFlag && !m_bIsFinalGC)
        {
          *pFlag = true;
        }
    }

    void NewObject (CPouch *);
    void HandleZRC (CPouch *);
    int GarbageCollect ();
    int     Reclamation ();
    void
    DestructObject (CPouch *pPouch)
    {
      pPouch->destruct ();                              // note - it is safe to call multiple times
    }
  };
} // end of namespace harnix

using namespace harnix;
