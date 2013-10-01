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
  //
  //  CPouch::CreateInstance - allocate space and return for operator new
  //
  void *
  CPouch::CreateInstance (IRuntimeClassInfo *pIRtClassInfo, bool bHasSize, size_t nNumOfElem, size_t nTotalSize)
  {
    if (bHasSize)
      {
        // array of objects with extra size
        CPouchHeader *pPouchHeader = new
                                     (
          ::operator new (OffsetOf(&CPouchHeader::m_nNumOfElem) + nTotalSize)
                                     )CPouchHeader;
        CPouch *pPouch = (CPouch *)&pPouchHeader->m_vUserData;

        memset (&pPouchHeader->m_nNumOfElem, 0, nTotalSize);

        pPouchHeader->m_nNumOfElem = nNumOfElem;
        pPouchHeader->m_pIRtClassInfo = pIRtClassInfo;

        CCollector::GetInstance ().NewObject (pPouch);

        return &pPouchHeader->m_nNumOfElem;
      }
    else
      {
        // an object or an array of objects without extra size
        CPouchHeader *pPouchHeader = new
                                     (
          ::operator new (OffsetOf(&CPouchHeader::m_vUserData) + nTotalSize)
                                     )CPouchHeader;
        CPouch *pPouch = (CPouch *)&pPouchHeader->m_vUserData;

        memset (&pPouchHeader->m_vUserData, 0, nTotalSize);

        pPouchHeader->m_nNumOfElem = nNumOfElem;
        pPouchHeader->m_pIRtClassInfo = pIRtClassInfo;

        CCollector::GetInstance ().NewObject (pPouch);

        return &pPouchHeader->m_vUserData;
      }
  }

  //
  // CPouch::traverse - invoke user-defined Traverse routine of the object(s)
  //
  void
  CPouch::traverse (void *caller)
  {
    CPouchHeader *pPouchHeader =
      ContainingRecord ((CPouchBody *)this, &CPouchHeader::m_vUserData);
    IRuntimeClassInfo *pRuntimeClassInfo = pPouchHeader->m_pIRtClassInfo;
    size_t nNumOfElem = pPouchHeader->m_nNumOfElem;

    if (nNumOfElem == 1)
      {
        pRuntimeClassInfo->OnReclaim (this, caller);
      }
    else
      {
        size_t nObjSize = pRuntimeClassInfo->GetSize ();
        while (nNumOfElem--)
          {
            pRuntimeClassInfo->Traverse (PointerAdjust (this, nObjSize * nNumOfElem), caller);
          }
      }
  }

  //
  // CPouch::onreclaim - invoke user-defined OnReclaim routine of the object(s)
  //
  void
  CPouch::onreclaim (void *caller)
  {
    CPouchHeader *pPouchHeader =
      ContainingRecord ((CPouchBody *)this, &CPouchHeader::m_vUserData);
    IRuntimeClassInfo *pRuntimeClassInfo = pPouchHeader->m_pIRtClassInfo;
    size_t nNumOfElem = pPouchHeader->m_nNumOfElem;

    if (nNumOfElem == 1)
      {
        pRuntimeClassInfo->OnReclaim (this, caller);
      }
    else
      {
        size_t nObjSize = pRuntimeClassInfo->GetSize ();
        while (nNumOfElem--)
          {
            pRuntimeClassInfo->OnReclaim (PointerAdjust (this, nObjSize * nNumOfElem), caller);
          }
      }
  }

  //
  // CPouch::destruct - execute user-defined destructor of object(s)
  //
  void
  CPouch::destruct ()
  {
    CPouchHeader *pPouchHeader =
      ContainingRecord ((CPouchBody *)this, &CPouchHeader::m_vUserData);

    if (getFlags ()->bitwiseCompareExchange (F_DESTRUCTED, 0, F_DESTRUCTED) == 0)
      {
        IRuntimeClassInfo *pRuntimeClassInfo = pPouchHeader->m_pIRtClassInfo;
        size_t nNumOfElem = pPouchHeader->m_nNumOfElem;
        if (nNumOfElem == 1)
          {
            pRuntimeClassInfo->Destructor (this);
          }
        else
          {
            size_t nObjSize = pRuntimeClassInfo->GetSize ();
            while (nNumOfElem--)
              {
                pRuntimeClassInfo->Destructor (PointerAdjust (this, nObjSize * nNumOfElem));
              }
          }
      }
  }

  //
  //  CPouch::destory - remove object from the system
  //
  void
  CPouch::destroy ()
  // note - the caller of this function should remove `this' from other
  // data structures, such as Black list or destructed unreachable list.
  {
    CPouchHeader *pPouchHeader =
      ContainingRecord ((CPouchBody *)this, &CPouchHeader::m_vUserData);

    delete pPouchHeader;
  }
} // end of namespace harnix
