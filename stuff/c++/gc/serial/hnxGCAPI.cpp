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

using namespace harnix;

//
// _hnxgc_operator_new - create a managed memory block for specified object(s)
//
#if defined HNX_CONFIG_FASTCALL
extern "C" HNXGC_API void *__cdecl
_hnxgc_operator_new (void *pRuntimeClassInfo, size_t nObjSize, size_t nTotalSize, size_t nFlags) throw()
{
  return _hnxgc_operator_new_fast (pRuntimeClassInfo, nObjSize, nTotalSize, nFlags);
}

extern "C" HNXGC_API void *__fastcall
_hnxgc_operator_new_fast (void *pRuntimeClassInfo, size_t nObjSize, size_t nTotalSize, size_t nFlags) throw()
#else
extern "C" HNXGC_API void *__cdecl
_hnxgc_operator_new (void *pRuntimeClassInfo, size_t nObjSize, size_t nTotalSize, size_t nFlags) throw()
#endif
{
  bool bHasSize;
  size_t nNumOfElem;

  // calculate bHasSize and nNumOfElem

  if (nObjSize > sizeof (size_t))
    {
      nNumOfElem = nTotalSize / nObjSize;
      bHasSize = (nTotalSize % nObjSize) != 0;
      ASSERT ((nTotalSize % nObjSize) ? (nTotalSize % nObjSize) == sizeof (size_t) : true);
    }
  else
    {
      bHasSize = nFlags != 0;
      nNumOfElem = (nTotalSize - bHasSize * sizeof (size_t)) / nObjSize;
    }

  return CPouch::CreateInstance (
           (IRuntimeClassInfo *)pRuntimeClassInfo,
           bHasSize, nNumOfElem, nTotalSize);
}

#if defined HNX_CONFIG_FASTCALL
extern "C" HNXGC_API void *__cdecl
_hnxgc_assign_lptr (void *dest, void *src) throw()
{
  return _hnxgc_assign_lptr_fast (dest, src);
}

extern "C" HNXGC_API void *__fastcall
_hnxgc_assign_lptr_fast (void *dest, void *src) throw()
#else
extern "C" HNXGC_API void *__cdecl
_hnxgc_assign_lptr (void *dest, void *src) throw()
#endif
{
  CPouch *pPouch;

  pPouch = *(CPouch **)dest;
  if (pPouch)
    {
      pPouch = pPouch->RealPtr ();
      if (pPouch->getLECount ()->unlock ())
        {
          CCollector::GetInstance ().HandleZRC (pPouch);
        }
    }

  *(void **)dest = src;

  pPouch = (CPouch *)src;
  if (pPouch)
    {
      pPouch = pPouch->RealPtr ();
      pPouch->getLECount ()->lock ();
    }

  return src;
}

#if defined HNX_CONFIG_FASTCALL
extern "C" HNXGC_API void *__cdecl
_hnxgc_assign_mptr (void *dest, void *src) throw()
{
  return _hnxgc_assign_mptr_fast (dest, src);
}

extern "C" HNXGC_API void *__fastcall
_hnxgc_assign_mptr_fast (void *dest, void *src) throw()
#else
extern "C" HNXGC_API void *__cdecl
_hnxgc_assign_mptr (void *dest, void *src) throw()
#endif
{
  CPouch *pPouch;

  pPouch = *(CPouch **)dest;
  if (pPouch)
    {
      pPouch = pPouch->RealPtr ();
      if (pPouch->getLECount ()->release ())
        {
          CCollector::GetInstance ().HandleZRC (pPouch);
        }
    }

  *(void **)dest = src;

  pPouch = (CPouch *)src;
  if (pPouch)
    {
      pPouch = pPouch->RealPtr ();
      pPouch->getLECount ()->addRef ();
    }
  return src;
}

extern "C" HNXGC_API int __cdecl
_hnxgc_garbage_collect (void *pType) throw()
{
  if (pType == 0)
    {
      return CCollector::GetInstance ().GarbageCollect ();
    }
  else if (pType == _hnxgc_gc_SkipFinalGC)
    {
      CCollector::GetInstance ().skipFinalGC ();
      return 0;
    }
  else if ((uintptr_t)pType >= 0x1000)
    {
      if (*(void **)pType == _hnxgc_gc_Init)
        {
          _hnxgc_gc_init_t *pParam = (_hnxgc_gc_init_t *)pType;
          DPRINTF ("version %s vs %s\n", pParam->m_pVersion, HNXGC_VERSION);
          DPRINTF ("versionsig %s vs %s\n", pParam->m_pVersionSig, HNXGC_VERSIONSIG);
          pParam->m_pBuildSig = HNXGC_BUILDSIG;
          CCollector::Initialization ();
          return 0;
        }
      else if (*(void **)pType == _hnxgc_gc_Fini)
        {
          CCollector::Termination ();
          return 0;
        }
    }

  return -1;
}

extern "C" HNXGC_API void __cdecl
_hnxgc_traverse_report (void *caller, size_t n, void **ppObj) throw()
{
  CCollector::GetInstance ().TraverseReport (caller, n, (CPouch **)ppObj);
}

extern "C" HNXGC_API void __cdecl
_hnxgc_onreclaim_resurrect (void *caller) throw()
{
  return CCollector::GetInstance ().OnReclaimResurrect (caller);
}

extern "C" HNXGC_API void __cdecl
_hnxgc_onreclaim_declare (void *caller, size_t n, void **ppObj) throw()
{
  CCollector::GetInstance ().OnReclaimDeclare (caller, n, (CPouch **)ppObj);
}

extern "C" HNXGC_API void *__cdecl
_hnxgc_onreclaim_change_depender (void *caller, void *pDepender) throw()
{
  return CCollector::GetInstance ().OnReclaimChangeDepender (caller,
                                                             reinterpret_cast<CPouch *> (pDepender)->RealPtr ());
}

extern "C" HNXGC_API int __cdecl
_hnxgc_destruct_object (void *pObj) throw()
{
  CCollector::GetInstance ().DestructObject (reinterpret_cast<CPouch *> (pObj)->RealPtr ());
  return 0;
}
