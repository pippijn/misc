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

#include "hnxGC_ST.h"

namespace harnix
{
  CCollector *CCollector::s_pInstance = NULL;

  void
  CCollector::NewObject (CPouch *pPouch)
  {
    getBlackList ().pushTail (pPouch);
    setColorBlack (pPouch);
  }

  void
  CCollector::HandleZRC (CPouch *pPouch)
  {
    if (m_bF1)
      {
        // invoked from reclamation of unreachable objects.
        DestructObject (pPouch);                                                // safe to call multiple times
      }
    else
      {
        if (!pPouch->isDestructed ())
          {
            bool bResurr = false;
            CDeclarator dcl (&bResurr, 0, 0);                           // to discard declaration of dependence
            pPouch->onreclaim (&dcl);                                           // only allow resurrection
            if (bResurr)
              {
                return;
              }
          }
        DestructObject (pPouch);                                                // safe to call multiple times
        // Single Thread mode, so the object is guaranteed in BlackList
        getBlackList ().remove (pPouch);
        pPouch->destroy ();
      }
  }

  int
  CCollector::GarbageCollect ()
  {
    if (m_bF1)
      {
        // recursively invoking GC
        return -1;
      }
    m_bF1 = true;
    m_nWhiteColor = !m_nWhiteColor;

    // determine root objects
    for (ObjectList::Iterator iter (getWhiteList ()); iter;)
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
          }
        else
          {
            iter.next ();
          }
      }

    CPouch *pPouch;
    while ((pPouch = getGrayList ().popHead ()) != 0)
      {
        // note - There is no racing condition for Single-Threading model.
        if (!pPouch->isDestructed ())
          {
            pPouch->traverse ();
          }
        getBlackList ().pushTail (pPouch);
      }

    if (Reclamation () == 1)
      {
        // resurrection: move Unreachable objects back to live
        getBlackList ().pushTail (getWhiteList ());
      }

    m_bF1 = false;

    return 0;
  }
} // end of namespace harnix
