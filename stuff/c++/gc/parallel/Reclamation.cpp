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
  // note - must put into here to multi-threading safty (this unit has no
  // contention with other threads)
  void
  _dummy_empty ()
  {
  }

  int
  CCollector::Reclamation ()
  {
    ObjectList lstDestructed;                                           // destructed object list

    // step 401 - determine unreachable objects
    do for (ObjectList::Iterator iter (getWhiteList ()); iter;)
        {
          CPouch *pPouch = iter;
          iter.next ();

          if (pPouch->isDestructing ())
            {
              // An unreachable object is said being destructed by mutator.
              // The object is guaranteed be alive during the destructing,
              // therefore, when it became unreachable, the destruction has
              // completed. Herein, it is safe prepare removing it from the
              // system when it reaches ZRC.

              getWhiteList ().remove (pPouch);
              lstDestructed.pushTail (pPouch);
            }
          else
            {
              pPouch->getFlags ()->bitwiseOR (F_UNREACHABLE);
            }
        }
    while (0);

    // step 402 - invoke OnReclaim routine of all unreachable objects
    bool bResurr = false;
    do for (ObjectList::Iterator iter (getWhiteList ()); iter; iter.next ())
        {
          CPouch *pPouch = iter;
          CDeclarator dcl (&bResurr, pPouch, pPouch);

          pPouch->onreclaim (&dcl);                             // call OnReclaim routine for
          // each unreachable object to give them a chance to resurrect.
        }
    while (0);

    if (bResurr)
      {
        // note - some OnReclaim routines have relinked unreachable
        // objects to live objects, and these actions have been logged
        // into GCBarrierList.

        // 1. clear the D-List
        CDependence *pRec;
        while ((pRec = m_lstDependences.popNode ()) != 0)
          {
            delete pRec;
          }

        // 2. unmark unreachabe objects
        do for (ObjectList::Iterator iter (getWhiteList ()); iter; iter.next ())
            {
              CPouch *pPouch = iter;
              pPouch->getFlags ()->bitwiseAND (~F_UNREACHABLE);
            }
        while (0);

        // 3. move back destructed objecs
        getWhiteList ().pushTail (lstDestructed);

        // return back and continue traversal again.
        return 1;
      }

    // note - From now, unreachable objects can be dropped to ZRC or/and
    // be explicitly destructed.

    ObjectList lstStandby;                                              // objects that is depended by others
    ObjectList *pDestructing = &getWhiteList ();
    ObjectList *pStandby = &lstStandby;
    uint_flags nColorDestructing = 0;                           // identify Q-Destructing or Q-Standby
    for (;;)
      {
        // note - Q-Standby is empty at beginning for every loop

        // step 403 - scan D-List
        do for (DRecList::Iterator iter (m_lstDependences); iter;)
            {
              CPouch *pDepender = iter->m_pDepender;
              // note - D-List records have been limited to among unreachable
              // objects, others are discarded in CCollector::OnReclaimDeclare.

              if (pDepender->isDestructing ())
                {
                  CDependence *pRec = iter;
                  iter.remove ();
                  delete pRec;
                  continue;
                }

              // move dependee from Q-Destructing to Q-Standby
              CPouch *pDependee = iter->m_pDependee;
              if (pDependee->getFlags ()->bitwiseCompareExchange (
                    F_DEPENDENCE, nColorDestructing,
                    F_DEPENDENCE & (~nColorDestructing)
                    ) == nColorDestructing)
                {
                  // this ensures a dependee would only move once from
                  // Q-Destructing to Q-Standby
                  pDestructing->remove (pDependee);
                  pStandby->pushTail (pDependee);
                }

              iter.next ();
            }
        while (0);

        // step 404 - check Q-Destructing
        if (pDestructing->isEmpty ())
          {
            // No destruction can be taken, no depender will vanish
            break;
          }

        // step 405 - destructing objects in Q-Destructing
        CPouch *pPouch;
        while ((pPouch = pDestructing->popHead ()) != 0)
          {
            // There is not other threads asynchronously destructing these
            // unreachable objects. Currently only one thread performing
            // reclamation and nesting HandleZRC function call.
            if (pPouch->isDestructing ())
              {
                // previous nesting HandleZRC function has destructed the object
                lstDestructed.pushTail (pPouch);
              }
            else
              {
                setDestructing (pPouch);
                pPouch->destruct ();
                lstDestructed.pushTail (pPouch);
              }
          }

        // step 407 - remove records in D-List that the depender of the
        // record has been destructed. (emerged into step 403)

        // step 408 - switch Q-Destructing and Q-Standby
        ObjectList *pTemp = pDestructing;
        pDestructing = pStandby;
        pStandby = pTemp;
        nColorDestructing = F_DEPENDENCE & ~nColorDestructing;
      }           // loop back D-List scanning

    //
    // Destructions of objects have done
    //
    if (!pStandby->isEmpty ())
      {
        // circular-depended relationship found! all objects are moved
        // from Q-Destructing to Q-Standby.

        // 1. clear D-List
        do for (DRecList::Iterator iter (m_lstDependences); iter;)
            {
              CDependence *pRec = iter;
              iter.remove ();
              delete pRec;
            }
        while (0);

        // 2. TODO: log these circular-depended objects

        // 3. skip app-level reclamation of Q-Standby objects.
        lstDestructed.pushTail (*pStandby);
      }

    // step 406 - System-level reclamation
    //	all unreachable objects are destructed (or skipped for the reason of
    //	circular depending relationship), there is neither live objects nor
    //  any destructor refering them.
    CPouch *pPouch;
    while ((pPouch = lstDestructed.popHead ()) != 0)
      {
        // RC should be zero, but cannot guaranteed for some bad-behavor
        // application does not drop reference at destruction, or existing
        // some circular-depended objects.

        // ASSERT(pPouch->getLECount()->isZero());

        // after GC traversal, GCBL can *only* contain Black objects.
        ASSERT (!pPouch->isInGCBL ());

        pPouch->destroy ();
      }

    // By now, Q-Destructed, Q-Destructing, D-List, Q-Standby should be empty
    ASSERT (pDestructing->isEmpty ());
    ASSERT (pStandby->isEmpty ());
    ASSERT (lstDestructed.isEmpty ());
    ASSERT (m_lstDependences.isEmpty ());
    return 0;
  }
} // end of namespace harnix
