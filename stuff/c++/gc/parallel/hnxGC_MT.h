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


// Configuration Macros:
//
// HNXGC_CONFIG_UNALIGNACCESS - define this to support unaligned pointer access (SMP)
//

#define HNXGC_EXPORTS

#include "hnxGC/hnxGC.h"

#include "hnx/DoubleList.hpp"
#include "hnx/SingleList.hpp"

//
//	Check some configurations
//
#if !defined(__SMP__)
#undef HNXGC_CONFIG_UNALIGNACCESS
#endif

#if defined(_DEBUG)
#define RACINGDELAY()           do { for (int i = 0; i < 1000; i++) ;} while (0)
#else
#define RACINGDELAY()           do { } while (0)
#endif

#define DPRINTF                         HNX_DPRINTF

#define HNXGC_BUILDSIG          "HnxGC-1.0(Alpha 07613) "

#include "hnxGC_MT/hnxGCFence.hpp"
#include "hnxGC_MT/hnxGCSync.hpp"

namespace harnix
{
  class CPouch;

  // CPouchBody - the place for application-level object data
  class CPouchBody
  {
  public:
    byte m_aData[0x1234];
  };

  // CSysData - object data that is basically used by collector
  class CSysData
  {
  public:
    CPouch *m_pPouch;                                                                   // point back the obj or the array of objs
    DListEntry m_vLnkObjects;                                                   // link objects of the same color
    IRuntimeClassInfo *m_pIRtClassInfo;                                 // containing entries of Traverse, Destructor, etc.

  public:
    CSysData (CPouch *pPouch, IRuntimeClassInfo *pIRtClassInfo)
      : m_pPouch (pPouch)
      , m_pIRtClassInfo (pIRtClassInfo)
    {
    }
  };

  class CPouchHeader            // for a single object
  {
  public:
    SListEntry m_lnkGCBarrier;                                          // Single List Entry for GCBarrier
    CAtomicFlags m_nFlags;                                              // Pouch state
    CLECount m_vLECount;                                                        // L-Counter & E-Counter
    CSysData *m_pSysData;                                                       // point to associated system data
    CPouchBody m_vUserData;                                             // app see this address
  };

  class CPouchHeaderArray               // for array of objects
  {
  public:
    CSysData *m_pSysData;                                                       // point to associated system data
    size_t m_nObjSize;                                                          // size of each element
    SListEntry m_lnkGCBarrier;                                          // Single List Entry for GCBarrier
    CAtomicFlags m_nFlags;                                              // Pouch state
    CLECount m_vLECount;                                                        // L-Counter & E-Counter
    size_t m_nNumOfElem;                                                // operator new return this address for C++ compiler
    CPouchBody m_vUserData;                                             // app see this address
  };

  class CPouchHeaderCommon              // for common RC & GCBarrier access
  {
  public:
    SListEntry m_lnkGCBarrier;                                          // Single List Entry for GCBarrier
    CAtomicFlags m_nFlags;                                              // Pouch state
    CLECount m_vLECount;                                                        // L-Counter & E-Counter
    size_t reserved;
    CPouchBody m_vUserData;                                             // app see this address
  };

  // `m_nFlags' bit fields:
  // color:1;				0000-000x		0/1 - white / black
  // F_INGCBL:1;			0000-000x		0/4 - none / in GCBL
  // F_ISARRAY:1;			0000-000x		0/8 - single / array
  // res1:5;
  // F_DESTRUCTING:1		0000-0100		0/0x100 - normal / destructing
  // F_UNREACHABLE:1;		0000-0200		0/0x200 - normal / unreachable
  // F_ONRECLAIM:1;		0000-0400		0/0x400 - normal / onreclaim called
  // F_DEPENDENCE:1		0000-0800		0/0x800 - Q-Destructing/Q-Standby
  // res2:21

  static const uint_flags F_COLORMASK = 0x1;
  static const uint_flags F_INGCBL = 0x4;
  static const uint_flags F_ISARRAY = 0x8;
  static const uint_flags F_DESTRUCTING = 0x100;
  static const uint_flags F_UNREACHABLE = 0x200;
  static const uint_flags F_ONRECLAIM = 0x400;
  static const uint_flags F_DEPENDENCE = 0x800;

  class CPouch
    : public CPouchBody
  {
  public:
    static void *CreateInstance (IRuntimeClassInfo *pRuntimeClassInfo, bool bHasSize, size_t nObjSize, size_t nNumOfElem, size_t nTotalSize);
    void traverse (void *caller = 0);
    void onreclaim (void *caller);
    void destruct ();
    void destroy ();

    CPouchHeaderCommon *
    getCommonHeader ()
    {
      return ContainingRecord ((CPouchBody *)this, &CPouchHeaderCommon::m_vUserData);
    }

    bool
    isArray ()
    {
      return getCommonHeader ()->m_nFlags.constTest (F_ISARRAY) != 0;
    }

    CSysData *
    getSysData ()
    {
      if (isArray ())
        {
          CPouchHeaderArray *pPouchHeader =
            ContainingRecord ((CPouchBody *)this, &CPouchHeaderArray::m_vUserData);
          return pPouchHeader->m_pSysData;
        }
      else
        {
          CPouchHeader *pPouchHeader =
            ContainingRecord ((CPouchBody *)this, &CPouchHeader::m_vUserData);
          return pPouchHeader->m_pSysData;
        }
    }

    CLECount *
    getLECount ()
    {
      return &getCommonHeader ()->m_vLECount;
    }

    CAtomicFlags *
    getFlags ()
    {
      return &getCommonHeader ()->m_nFlags;
    }

    bool
    isLocked ()
    {
      return getLECount ()->isLocked ();
    }

    bool
    isDestructing ()
    {
      return getFlags ()->bitwiseTest (F_DESTRUCTING) != 0;
    }

    bool
    isInGCBL ()
    {
      return getFlags ()->bitwiseTest (F_INGCBL) != 0;
    }

    CPouch *
    RealPtr ()
    {
      return PointerAndMask (this, ~3);                                 // Pouch is always aligned to 4 byte boundary at least
    }

    static DListEntry *
    GetObjLink (CPouch *pPouch)
    {
      return &pPouch->getSysData ()->m_vLnkObjects;
    }

    static SListEntry *
    GetBarrierLink (CPouch *pPouch)
    {
      return &pPouch->getCommonHeader ()->m_lnkGCBarrier;
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
  typedef SingleLinkedList<CPouch, &CPouch::GetBarrierLink> GCBarrierList;
  typedef SingleLinkedList<CDependence,
                           SListEntryGetFromPtr<CDependence, &CDependence::m_vLink> >      DRecList;

  class CCollector
  {
  private:
    uint_flags m_nWhiteColor;                                                   // color value for `white'
    ObjectList m_aLists[2];                                                     // white & black object list
    ObjectList m_lstGray;                                                               // Gray object list (LG)
    ObjectList m_lstNewObj;                                                     // newly created object (SB)
    GCBarrierList m_lstBarrier;                                         // GC Barrier
    DRecList m_lstDependences;                                                  // single list of dependence records

  public:
    CCritSec m_lckL1;                                                                   // synchronize mutators with collector
    uint_flags m_bF1;                                                                   // indicate the marking-phase in progress
    CCritSec m_lckL2;                                                                   // allow only one collector
    uintptr_t m_nGCCount;                                                               // # of garbage collection operation
    CTraverseLock m_lckTraverse;                                        // allow pause/resume traverse operation
    bool m_bSkipFinalGC;                                                        // skip garbage collection at exit
    uintptr_t m_nInitCount;                                                     // # of initialization calls
    bool m_bIsFinalGC;                                                          // the final collection
#if defined(HNXGC_CONFIG_UNALIGNACCESS)
    CRollbackLock m_lckRollback;                                        // allow rollback of unaligned traverse
#endif
    // note - (1) currently the `m_lckL1' need *not* be a recursive lock
    // to support nesting HandleZRC in destructing unreachable objects as
    // the system does not hold the lock for GC processing. (2) the
    // `m_lckL2' need to be recursive to support invoking garbage collection
    // within destruction of unreachable objects, etc.

    static CCollector *s_pInstance;

  public:
    CCollector ()
      : m_nWhiteColor (0)
      , m_bF1 (0)
      , m_nGCCount (0)
      , m_bSkipFinalGC (false)
      , m_nInitCount (1)
      , m_bIsFinalGC (false)
    {
    }

    ~CCollector ()
    {
      if (!m_bSkipFinalGC)
        {
          _hnxgc_garbage_collect (_hnxgc_gc_WaitCollect);
        }
    }

    void
    skipFinalGC ()
    {
      m_bSkipFinalGC = true;
    }

    // Note - the first invocation of `Initialization' is guaranteed be
    // happenned before starting up multiple threads, so there is no
    // racing problem herein.
    static void
    Initialization ()
    {
      if (s_pInstance)
        {
          atomicIncrementPtr (&s_pInstance->m_nInitCount);
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
          atomicDecrementPtr (&s_pInstance->m_nInitCount);
          if (s_pInstance->m_nInitCount == 0)
            {
              // all threads should have been terminated before reach here.
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

    bool
    checkF1 ()
    {
      return m_bF1 != 0;
    }

#if defined(_DEBUG)
#define TRACECOLOR(pPouch)                      do {                                                                                                            \
      uint_flags f = (pPouch)->getFlags ()->                                                                   \
                     bitwiseTest (F_INGCBL | F_COLORMASK);            \
      uint_flags color = f & F_COLORMASK;                                                                             \
      DPRINTF ("%p %s%s\n", (pPouch), f & F_INGCBL ? "Silver " : "",                   \
               color == m_nWhiteColor ? "White" : "Black");                                                    \
  } while (0)
#else
#define TRACECOLOR(pPouch)                      do { } while (0)
#endif

    void
    setColorBlack (CPouch *pPouch)
    {
      pPouch->getFlags ()->setValue (F_COLORMASK, !m_nWhiteColor);
    }

    void
    resetInGCBL (CPouch *pPouch)
    {
      pPouch->getFlags ()->setValue (F_INGCBL, 0);
    }

    // return: (called by Collector when performing traversal/GCBL)
    //	true - the color was White, changed to Black
    //	false - the color was not White, not changed
    int
    convertWhiteToBlack (CPouch *pPouch)
    {
      return pPouch->getFlags ()->bitwiseCompareExchange (
               F_COLORMASK, m_nWhiteColor, !m_nWhiteColor
               ) == m_nWhiteColor;
    }

    // return: (called by Mutator CCollector::GCBarrier())
    //	true - the color was White or Black, add F_INGCBL
    //	false - the color was not White or safe to skip GCBarrier
    bool
    convertWhiteToSilver (CPouch *pPouch)
    {
      uint_flags nWhiteColor = m_nWhiteColor;

      // note - if `m_nWhiteColor' is changed by Collector before
      // Mutator check the object color, then it means at least one
      // GC is started between these two operations. The SaveRef
      // operation that precedes GCBarrier will guarantee to be seen
      // by Collector, so skipping GCBarrier is no problem.

      return pPouch->getFlags ()->bitwiseCompareExchange (
               F_COLORMASK | F_INGCBL, nWhiteColor,
               F_INGCBL | nWhiteColor) == nWhiteColor;
    }

    void
    setDestructing (CPouch *pPouch)
    {
      pPouch->getFlags ()->bitwiseOR (F_DESTRUCTING);
    }

    bool
    convertToDestructing (CPouch *pPouch)
    {
      // note - once set, the object will retain in F_DESTRUCTING
      // state forever except in the case of resurrection.
      return pPouch->getFlags ()->bitwiseCompareExchange (
               F_DESTRUCTING, 0, F_DESTRUCTING) == 0;
    }

    void
    convertBackLive (CPouch *pPouch)
    {
      pPouch->getFlags ()->bitwiseAND (~F_DESTRUCTING);
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

    ObjectList &
    getNewList ()
    {
      return m_lstNewObj;
    }

    GCBarrierList &
    getBarrierList ()
    {
      return m_lstBarrier;
    }

  public:
#if defined(HNXGC_CONFIG_UNALIGNACCESS)
    void
    TraverseReport (void *caller, size_t n, CPouch **ppObj)
    {
      bool &bConflict = *(bool *)caller;

      if (bConflict)
        {
          return;
        }
      if (m_lckRollback.leaveTry ())
        {
          bConflict = true;
        }
      if (!bConflict)
        {
          // the ppObj[] contains correct addresses
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
      if (m_lckRollback.enterTry ())
        {
          bConflict = true;
        }
    }

#else
    void
    TraverseReport (void * /*caller*/, size_t n, CPouch **ppObj)
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
#endif

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
          // HandleZRC will set `owner' to zero as a flag.
          return;
        }
      CPouch *pDepender = pDecl->m_pDepender;
      if (pDepender->getFlags ()->bitwiseTest (F_UNREACHABLE))
        {
          for (size_t i = 0; i < n; i++)
            {
              if (ppObj[i])
                {
                  CPouch *pPouch = ppObj[i]->RealPtr ();
                  if (pPouch->getFlags ()->bitwiseTest (F_UNREACHABLE))
                    {
                      m_lstDependences.pushNode (
                        new CDependence (pDepender, pPouch));
                    }
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

    void
    GCBarrier (CPouch *pPouch)
    {
      if (convertWhiteToSilver (pPouch))
        {
          // it is now safely writing barrier for this Pouch

          // note - collector may finish traversal here, and convert the
          // object to Black.

          // atomic single-list access (with other mutators and Pouches)
          getBarrierList ().atomicPushNode (pPouch);
        }
    }

    void NewObject (CPouch *);
    void HandleZRC (CPouch *, void **, void *);
    int GarbageCollect ();
    int Reclamation ();
    void Traverse (CPouch *);
#if defined(HNXGC_CONFIG_UNALIGNACCESS)
    void
    UpdatePtr (void **pRef, void *value)
    {
      uintptr_t addr = (uintptr_t)pRef;

      if ((addr & (sizeof (void *) - 1)) == 0)
        {
          // naturally-aligned (most cases)
          *pRef = value;
        }
      else
        {
          // unaligned pointer
          m_lckRollback.enterWork ();
          *pRef = value;
          m_lckRollback.leaveWork ();
        }
    }

    void
    UpdatePtrRelease (void **pRef, void *value)
    {
      uintptr_t addr = (uintptr_t)pRef;

      if ((addr & (sizeof (void *) - 1)) == 0)
        {
          // naturally-aligned (most cases)
          atomicExchangePtrRelease (pRef, value);
        }
      else
        {
          // unaligned pointer
          m_lckRollback.enterWork ();
          atomicExchangePtrRelease (pRef, value);
          m_lckRollback.leaveWork ();
        }
    }

#else
    void
    UpdatePtr (void **pRef, void *value)
    {
      *pRef = value;
    }

    void
    UpdatePtrRelease (void **pRef, void *value)
    {
      atomicExchangePtrRelease (pRef, value);
    }
#endif


    void
    DestructObject (CPouch *pPouch)
    {
      // note - This function should only be called by HNXGC_DELETE(x) on live
      // object. Therefore, a destructing object will not be ZRC or unreachable
      // except destructing unreachable from destructor.

      if (convertToDestructing (pPouch))
        {
          if (pPouch->getFlags ()->bitwiseTest (F_UNREACHABLE))
            {
              // if the object is unreachable, then the caller must be
              // in a destructor which is the only way for application
              // accessing unreachable objects. The OnReclaim routine
              // must has been invoked before, so skip onreclaim.

              // note - allow it become ZRC in itself destruction, since
              // HandleZRC will not remove it from the system because of
              // nesting in a GC operation.
              pPouch->destruct ();
            }
          else
            {
              bool bResurr = false;
              CDeclarator dcl (&bResurr, 0, 0);
              // to discard declaration of dependence relationship

              pPouch->onreclaim (&dcl);

              if (bResurr)
                {
                  convertBackLive (pPouch);
                  return;
                }

              // ensure that the object is live during Destruction,
              // even all references to the object are dropped in
              // its destructor.
              pPouch->getLECount ()->lock ();                                           // IncrLRC

              // note - IncrLRC must precede ChkF1 in programming order, execution
              // order is not required under the Global Memory Fence mechanism.
              CompilerReadWriteBarrier ();

              if (checkF1 ())                                                                           // ChkF1
                {
                  GCBarrier (pPouch);                                                                   // GCBarrier
                }

              pPouch->destruct ();

              if (pPouch->getLECount ()->unlock ())                             // DecrLRC
                {
                  // note - the LRC remains non-zero herein to avoid simultaneously
                  // discover the object unreachable by garbage collection.

                  HandleZRC (pPouch, 0, 0);
                  // note - the second argument must be 0
                }
            }
        }
    }
  };
} // end of namespace harnix
