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

namespace harnix
{
  CCollector *CCollector::s_pInstance = NULL;

  char g_dummyData[64];                 // place for trash writting


  //
  //	CCollector::NewObject - add a newly-created Pouch to the system
  //
  void
  CCollector::NewObject (CPouch *pPouch)
  {
    m_lckL1.lock ();
    if (m_bF1)
      {
        getNewList ().pushTail (pPouch);
      }
    else
      {
        getBlackList ().pushTail (pPouch);
      }
    setColorBlack (pPouch);
    m_lckL1.unlock ();
  }

  //
  //	CCollector::HandleZRC - the Pouch is lost all references from mutator
  //
  void
  CCollector::HandleZRC (CPouch *pPouch, void **pRef, void *newRef)
  {
    // note - when execution reaches here, the LECount is retained its
    // original value (not updated to zero), and the content of the
    // pointer is also unchanged. In another word, the object is still
    // alive from both the views of RC and GC.

    // note - HNXGC_DELETE(x) require the object be kept alive, so if
    // destructing in HNXGC_DELETE(x) has not finished, the RC cannot drop
    // to zero. Therefore, if the F_DESTRUCTING is set by HNXGC_DELETE(x),
    // it is guaranteed that the destruction is done.
    if (convertToDestructing (pPouch))
      {
        if (pPouch->getFlags ()->bitwiseTest (F_UNREACHABLE))
          {
            // Unreachable objects have completed calling the OnReclaim
            // routine before executing destruction in which HandleZRC
            // are nested.
            ;
          }
        else
          {
            bool bResurr = false;
            CDeclarator dcl (&bResurr, 0, 0);
            // to discard declaration of dependence relationship

            pPouch->onreclaim (&dcl);

            if (bResurr)
              {
                if (pRef == 0)
                  {
                    pPouch->getLECount ()->unlock ();                                   // should not reach zero
                  }
                else
                  {
                    pPouch->getLECount ()->release ();                                  // should not reach zero
                  }
                convertBackLive (pPouch);
                if (pRef)
                  {
                    // ensure convertBackLive is global visible
                    CompilerReadWriteBarrier ();

                    UpdatePtrRelease (pRef, newRef);
                  }
                return;
                // note - if an object is resurrected but does not add a
                // new reference to itself at OnReclaim routine, it will
                // not be reclaimed until next garbage collection. (orphan
                // object)
              }
          }

        pPouch->destruct ();                            // destructing an object
      }

    // note - An object in progress of destructing will not become
    // unreachable until the completion of destruction. So, if collector
    // see a F_DESTRUCTING unreachable object, the destruction has
    // completed.

    m_lckL1.lock ();
    if (m_bF1)
      {
        // concurrent garbage collection is running

        CompilerReadWriteBarrier ();

        // to ensure the completion of destruction is global visible.
        UpdatePtrRelease ((void **)pPouch->getLECount (), 0);
        if (pRef)
          {
            UpdatePtrRelease (pRef, newRef);
          }
      }
    else
      {
        // no concurrent collector, and will not start a collection until
        // we unlock the `m_lckL1'.

        *(uintptr_t *)pPouch->getLECount () = 0;
        // must done pPouch manipulation before removal

        if (!pPouch->isInGCBL ())
          {
            // all objects are always in BlackList when GC is finished.
            getBlackList ().remove (pPouch);
            pPouch->destroy ();
          }
        else
          {
            // the object is in GCBL, need Collector to remove from it.
            ;                     // skip destroy
          }

        if (pRef)
          {
            *pRef = newRef;
          }
      }
    m_lckL1.unlock ();
  }

  //
  //	CCollector::GarbageCollect - perform garbage collection
  //
  int
  CCollector::GarbageCollect ()
  {
#if defined(_DEBUG)
    // The F_INGCBL of objects in GCBL are always set. The objects should
    // be Black and discardable from GCBL.
    do
      {
        GCBarrierList lstBarrier (getBarrierList ().atomicFlush ());
        if (!lstBarrier.isEmpty ())
          {
            CPouch *pPouch;
            while ((pPouch = lstBarrier.popNode ()) != 0)
              {
                if (pPouch->getFlags ()->bitwiseTest (F_INGCBL) == 0)
                  {
                    TRACECOLOR (pPouch);
                    __debugbreak ();
                  }
                if (pPouch->getFlags ()->bitwiseTest (F_COLORMASK) == m_nWhiteColor)
                  {
                    TRACECOLOR (pPouch);
                    __debugbreak ();
                  }
                pPouch->getFlags ()->bitwiseAND (~F_INGCBL);
              }
          }
      }
    while (0);

    // Black List should contains all and only Black (w/wo Silver) objects.
    m_lckL1.lock ();                    // accessing BlackList may race with NewObject
    do for (ObjectList::Iterator iter (getBlackList ()); iter; iter.next ())
        {
          if (iter->getFlags ()->bitwiseTest (F_COLORMASK) == m_nWhiteColor)
            {
              DPRINTF ("BlackList contains other colors.\n");
              TRACECOLOR ((CPouch *)iter);
              __debugbreak ();
            }
        }
    while (0);
    m_lckL1.unlock ();
#else
    ASSERT (getWhiteList ().isEmpty ());
#endif

    // (1) set m_bF1 to indicate the marking phase
    m_lckL1.lock ();
    if (m_bF1)
      {
        // recursively invoke garbage collection, skip it
        m_lckL1.unlock ();
        return 0;
      }
#if defined(_DEBUG)
    if (!getWhiteList ().isEmpty ())
      {
        DPRINTF ("White List is not empty\n");
        __debugbreak ();
      }
#endif

    m_nWhiteColor = !m_nWhiteColor;

    // note - not a must, but is better for reducing Black objects in GCBL
    MemoryFence ();

    m_bF1 = true;

    // note - after F1 set *AND* color swapped, mutator may insert new entries
    // into Barrier List, even collector is holding the L1 lock.

    ASSERT (getNewList ().isEmpty ());          // access NewList must under L1 locked
    m_lckL1.unlock ();

    // F1 is set, we can access Black List
    ASSERT (getBlackList ().isEmpty ());
    ASSERT (getGrayList ().isEmpty ());


    //////////////////////////////
    GlobalMemoryFence ();                       //
    //////////////////////////////

    // (2) add root objects to gray list
    do for (ObjectList::Iterator iter (getWhiteList ()); iter;)
        {
          CPouch *pPouch = iter;
          if (pPouch->isLocked ())
            {
              if (convertWhiteToBlack (pPouch))
                {
                  iter.next ();
                  getWhiteList ().remove (pPouch);
                  getGrayList ().pushTail (pPouch);
                  continue;
                }
              ASSERT ("Internal Error - all root objects should be white color.");
            }
          iter.next ();
        }
    while (0);

    do
      {
        // (3) traverse & convert gray to black
        CPouch *pPouch;
        m_lckTraverse.gcLock ();
        while ((pPouch = getGrayList ().popHead ()) != 0)
          {
            m_lckTraverse.gcYield ();                           // allow collector be paused

#if defined(HNXGC_CONFIG_UNALIGNACCESS)
            bool bConflict;
            while (1)
              {
                bConflict = false;
                if (m_lckRollback.enterTry ())
                  {
                    bConflict = true;
                  }
                if (!bConflict)
                  {
                    pPouch->traverse (&bConflict);
                  }
                if (m_lckRollback.leaveTry ())
                  {
                    bConflict = true;
                  }
                if (!bConflict) break;

                // note - if improper use SMP + UNALIGNACESS in UP environment,
                // that may cause deadlock. must yield to allow mutator leave
                // critical area.
                //
                // Sleep(1);
              }
#else
            pPouch->traverse ();
#endif
            getBlackList ().pushTail (pPouch);
          }
        m_lckTraverse.gcUnlock ();

        GCBarrierList lstBarrier (getBarrierList ().atomicFlush ());
        if (lstBarrier.isEmpty ())
          {
            // do Barrier Test first to avoid too much GMF operations

            //////////////////////////////
            GlobalMemoryFence ();                               //
            //////////////////////////////

            lstBarrier = getBarrierList ().atomicFlush ();
            if (lstBarrier.isEmpty ())
              {
                // (4) collector catches up the reference changes

                // start reclamation
                if (Reclamation () == 0)
                  {
                    break;
                  }
                else
                  {
                    ;                             // resurrection, continue traversal
                  }
              }
          }
        // (5) move barrier white to gray
        while ((pPouch = lstBarrier.popNode ()) != 0)
          {
            if (convertWhiteToBlack (pPouch))
              {
                getWhiteList ().remove (pPouch);
                getGrayList ().pushTail (pPouch);
              }
            // note - clearing F_INGCBL before convert it black is allowed,
            // even that will cause more Black objects in GCBL.
            resetInGCBL (pPouch);
          }
      }
    while (1);

    ASSERT (getWhiteList ().isEmpty ());

    m_lckL1.lock ();
    getBlackList ().pushTail (getNewList ());           // NewList must be also hold
    m_bF1 = false;                      // enable direct-access of Black List
    // note - clearing m_bF1 must under protection of m_lckL1 for
    // correctly adding object to BlackList instead NewList.
    m_nGCCount++;
    m_lckL1.unlock ();

#if defined(_DEBUG)
    do for (ObjectList::Iterator iter (getNewList ()); iter; iter.next ())
        {
          if (iter->getFlags ()->bitwiseTest (F_COLORMASK) == m_nWhiteColor)
            {
              DPRINTF ("NewList contains White object.\n");
              TRACECOLOR ((CPouch *)iter);
            }
        }
    while (0);
#endif

    return 0;
  }
} // end of namespace harnix
