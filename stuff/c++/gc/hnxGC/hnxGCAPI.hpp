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


namespace harnix
{
  // runtime class info interface required by garbage collector
  struct IRuntimeClassInfo
  {
    virtual void *__stdcall ExtendQuery (void *) = 0;
    virtual size_t __stdcall GetSize () = 0;
    virtual void __stdcall Traverse (void *, void *) = 0;
    virtual void __stdcall OnReclaim (void *, void *) = 0;
    virtual void __stdcall Destructor (void *) = 0;
  };
} // end of namespace harnix


#if     !defined(HNXGC_CONFIG_NODLL)

#ifdef HNXGC_EXPORTS
#define HNXGC_API __declspec (dllexport)
#else
#define HNXGC_API __declspec (dllimport)
#endif

#else

#define HNXGC_API

#endif


//
//  _hnx_operator_new - create a managed memory block
//
#if defined(HNX_CONFIG_FASTCALL)
extern "C" HNXGC_API void *__fastcall _hnxgc_operator_new_fast (void *pRuntimeClassInfo, size_t objSize, size_t totalSize, size_t nFlags) throw();

#if !defined(HNXGC_EXPORTS)
static inline
void *
_hnxgc_operator_new (void *pRuntimeClassInfo, size_t objSize, size_t totalSize, size_t nFlags)
{
  return _hnxgc_operator_new_fast (pRuntimeClassInfo, objSize, totalSize, nFlags);
}

#endif
#endif

#if defined(HNXGC_EXPORTS) || !defined(HNX_CONFIG_FASTCALL)
extern "C" HNXGC_API void *__cdecl _hnxgc_operator_new (void *pRuntimeClassInfo, size_t objSize, size_t totalSize, size_t nFlags) throw();
#endif


//
//  _hnxgc_assign_lptr - assign to a CLockedPtr smart pointer
//
#if defined(HNX_CONFIG_FASTCALL)
extern "C" HNXGC_API void *__fastcall _hnxgc_assign_lptr_fast (void *dest, void *src) throw();

#if !defined(HNXGC_EXPORTS)
static inline
void *
_hnxgc_assign_lptr (void *dest, void *src) throw()
{
  return _hnxgc_assign_lptr_fast (dest, src);
}

#endif
#endif

#if defined(HNXGC_EXPORTS) || !defined(HNX_CONFIG_FASTCALL)
extern "C" HNXGC_API void *__cdecl _hnxgc_assign_lptr (void *dest, void *src) throw();
#endif

//
//  _hnxgc_assign_mptr - assign to CMemberPtr smart pointer
//
#if defined(HNX_CONFIG_FASTCALL)
extern "C" HNXGC_API void *__fastcall _hnxgc_assign_mptr_fast (void *dest, void *src) throw();

#if !defined(HNXGC_EXPORTS)
static inline
void *
_hnxgc_assign_mptr (void *dest, void *src) throw()
{
  return _hnxgc_assign_mptr_fast (dest, src);
}

#endif
#endif

#if defined(HNXGC_EXPORTS) || !defined(HNX_CONFIG_FASTCALL)
extern "C" HNXGC_API void *__cdecl _hnxgc_assign_mptr (void *dest, void *src) throw();
#endif


//
//  _hnxgc_garbage_collect - perform garbage collection
//
extern "C" HNXGC_API int __cdecl _hnxgc_garbage_collect (void *pType = 0) throw();
namespace harnix
{
  void *const _hnxgc_gc_WaitCollect = (void *)1;
  void *const _hnxgc_gc_ZRCCollect = (void *)2;
  void *const _hnxgc_gc_TraversePause = (void *)3;
  void *const _hnxgc_gc_TraverseResume = (void *)4;
  void *const _hnxgc_gc_SkipFinalGC = (void *)5;
  void *const _hnxgc_globalmemoryfence = (void *)101;
  struct _hnxgc_gc_init_t
  {
    void *m_pExtType;
    char *m_pVersion;
    char *m_pVersionSig;
    char *m_pBuildSig;
  };
  void *const _hnxgc_gc_Init = (void *)1;
  struct _hnxgc_gc_fini_t
  {
    void *m_pExtType;
  };
  void *const _hnxgc_gc_Fini = (void *)2;
}


//
//	_hnxgc_traverse_report - Traverse routine callback function
//
extern "C" HNXGC_API void __cdecl _hnxgc_traverse_report (void *caller, size_t n, void **ppObj) throw();


//
//  OnReclaim routine callback function
//
extern "C" HNXGC_API void *__cdecl _hnxgc_onreclaim_change_depender (void *caller, void *pDepender) throw();

extern "C" HNXGC_API void __cdecl _hnxgc_onreclaim_declare (void *caller, size_t n, void **ppObj) throw();

extern "C" HNXGC_API void __cdecl _hnxgc_onreclaim_resurrect (void *caller) throw();


//
//	destruct live object
//
extern "C" HNXGC_API int __cdecl _hnxgc_destruct_object (void *pObj) throw();


//
//  other shorthand functions
//
static inline
void *
_hnxgc_lock_object (void *pObj)
{
  void *dummyPtr = 0;

  _hnxgc_assign_lptr (&dummyPtr, pObj);
  return pObj;
}

static inline
void *
_hnxgc_unlock_object (void *pObj)
{
  void *dummyPtr = pObj;

  _hnxgc_assign_lptr (&dummyPtr, 0);
  return pObj;
}

static inline
void *
_hnxgc_addref_object (void *pObj)
{
  void *dummyPtr = 0;

  _hnxgc_assign_mptr (&dummyPtr, pObj);
  return pObj;
}

static inline
void *
_hnxgc_release_object (void *pObj)
{
  void *dummyPtr = pObj;

  _hnxgc_assign_mptr (&dummyPtr, 0);
  return pObj;
}
