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
  class _typeOnReclaim
  {
  public:
    operator int ()
    {
      return 999;
    }
  };

  //
  //  CRuntimeClassInfo - helper class to provide IRuntimeClassInfo interaface
  //
  // note - the extra _CRuntimeClassInfo is for buggy GNU C++, don't merge it
  // with class CRuntimeClassInfo.
  template<typename T>
  class _CRuntimeClassInfo
    : public IRuntimeClassInfo
  {
  private:
    typedef void (*PFnOnReclaim)(T *, void *);
    void *__stdcall
    ExtendQuery (void *)
    {
      return 0;
    }

    size_t __stdcall
    GetSize ()
    {
      return sizeof (T);
    }

    void __stdcall
    Traverse (void *pThis, void *caller)
    {
      reinterpret_cast<T *> (pThis)->_HnxGC_Traverse (caller);
    }

    void __stdcall
    OnReclaim (void *pThis, void *caller)
    {
#if defined(HNX_CONFIG__IF_EXISTS)
      __if_exists (T::_HnxGC_OnReclaim)
      {
        reinterpret_cast<T *> (pThis)->_HnxGC_OnReclaim (caller);
      }
#else
      PFnOnReclaim pOnReclaim = (PFnOnReclaim) T::hasMember (_typeOnReclaim ());
      if (pOnReclaim)
        {
          pOnReclaim (reinterpret_cast<T *> (pThis), caller);
        }
#endif
    }

    void __stdcall
    Destructor (void *pThis)
    {
      reinterpret_cast<T *> (pThis)->~T ();
    }
  };
  template<typename T>
  class CRuntimeClassInfo
  {
  public:
    static _CRuntimeClassInfo<T> _;
    static IRuntimeClassInfo *
    GetIRuntimeClassInfo ()
    {
      return &_;
    }
  };
  template<typename T>
  _CRuntimeClassInfo<T>   CRuntimeClassInfo<T>::_;

  //
  //  CRuntimeClassInfoNative - ClassInfo for object without reference to others
  //
  template<typename T>
  class _CRuntimeClassInfoNative
    : public IRuntimeClassInfo
  {
  private:
    void *__stdcall
    ExtendQuery (void *)
    {
      return 0;
    }

    size_t __stdcall
    GetSize ()
    {
      return sizeof (T);
    }

    void __stdcall
    Traverse (void *pThis, void *caller)
    {
    }

    void __stdcall
    OnReclaim (void *pThis, void *caller)
    {
    }

    void __stdcall
    Destructor (void *pThis)
    {
      reinterpret_cast<T *> (pThis)->~T ();
    }
  };
  template<typename T>
  class CRuntimeClassInfoNative
  {
  public:
    static _CRuntimeClassInfoNative<T> _;
    static IRuntimeClassInfo *
    GetIRuntimeClassInfo ()
    {
      return &_;
    }
  };
  template<typename T>
  _CRuntimeClassInfoNative<T>     CRuntimeClassInfoNative<T>::_;
} // end of namespace harnix


//
//  Detect destructor for object with size <= sizeof(size_t)
//
namespace harnix
{
  class _typeTestArray
  {
  };                                                    // for array size word testing
}

inline
void *operator new
[] (size_t nTotalSize, const harnix::_typeTestArray &, size_t nObjSize)
{
  static char buf[sizeof (size_t) * 3 + sizeof (bool)];

  if (nTotalSize == sizeof (size_t) + nObjSize * 2)
    {
      (bool &)buf[nTotalSize] = true;
    }
  else
    {
      (bool &)buf[nTotalSize] = false;
    }
  return buf;
}

inline
void operator delete
[] (void *, const harnix::_typeTestArray &, size_t)
{
}

namespace harnix
{
  template<int type>
  class _CDetectDtor
  {
  public:
    template<typename T>
    static size_t
    Test (const T &)
    {
      return 0;
    }
  };

  template<>
  class _CDetectDtor<0>
  {
  public:
    template<typename T>
    static size_t
    Test (const T &)
    {
      if (*(bool *)(::new (*(_typeTestArray *)0, sizeof (T))T[2] + 2))
        {
          return 1;
        }
      else
        {
          return 0;
        }
    }
  };

  class _typeGCNew
  {
  };                                            // for gcnew operation

  template<typename T>
  class _typeGCNewNative
  {
  };                                            // for gcnew_native operation
} // end of namespace harnix

//
//  HNXGC_TRAVERSE(cls) - define a Traverse method for this type of objects
//
#if defined(HNX_CONFIG__IF_EXISTS)
#define _HNXGC_MEMBER_DETECT()

#else
#define _HNXGC_MEMBER_DETECT()  static void *hasMember (int) { return 0; }

#endif

#define HNXGC_TRAVERSE(cls)                                                                                                     \
public:                                                                                                                                         \
  typedef cls _hnxgc_class;                                                                                               \
  _HNXGC_MEMBER_DETECT ()                                                                                                  \
  static ::harnix::IRuntimeClassInfo * GetRuntimeClassInfo () {                   \
    return ::harnix::CRuntimeClassInfo < cls > ::GetIRuntimeClassInfo (); \
  }                                                                                                                                               \
  void * \
  operator new (size_t size, const harnix::_typeGCNew &) {                 \
    return _hnxgc_operator_new (                                                                                     \
             cls::GetRuntimeClassInfo (),                                                                             \
             sizeof (cls), size, 0);                                                                                  \
  }                                                                                                                                               \
  void \
  operator delete (void *, const ::harnix::_typeGCNew &) { }                    \
  void *operator new \
  [] (size_t size, const ::harnix::_typeGCNew &) {    \
    if (sizeof (cls) <= sizeof (size_t)) {                                                            \
        static size_t nHasDtor = size_t (-1);                                                    \
        if (nHasDtor == size_t (-1)) {                                                                   \
            nHasDtor = ::harnix::                                                                           \
                       _CDetectDtor < (sizeof (cls) > sizeof (size_t)) > ::                  \
                       Test (*(cls *)0);                                                                                 \
          }                                                                                                                               \
        return _hnxgc_operator_new (                                                                             \
                 GetRuntimeClassInfo (),                                                                          \
                 sizeof (cls), size, nHasDtor);                                                           \
      } else {                                                                                                                        \
        return _hnxgc_operator_new (                                                                             \
                 GetRuntimeClassInfo (),                                                                          \
                 sizeof (cls), size, 0);                                                                          \
      }                                                                                                                                       \
  }                                                                                                                                               \
  void operator delete \
  [] (void *, const harnix::_typeGCNew &) { }                  \
  void \
  operator delete (void *pMem) {                                                                             \
    ::operator delete (pMem);                                                                                       \
  }                                                                                                                                               \
  void operator delete \
  [] (void *pMem) {                                                                  \
    ::operator delete (pMem);                                                                                       \
  }                                                                                                                                               \
  void _HnxGC_Traverse (void *caller)
// Note - we need to create a testing array for classes of which size is equal
// or smaller than sizeof(size_t), so these classes must allow array creation
// such as have default constructor public defined.
// Note2 - multiple threads can simultaneously initialize the `nHasDtor'
// because they are setting the same value. There is no static variable
// initialization problem.


//
//  Traverse Routine Helpers
//
namespace harnix
{
  //  _PouchPtr - pointer to managed memory block
  class _PouchPtr
  {
    void *m_pObj;

  public:
    explicit _PouchPtr (void *ptr) throw()
      : m_pObj (ptr)
    {
    }

    operator void * ()
    {
      return m_pObj;
    }
  };


  class _CTraverseReporter
  {
    void *m_pCaller;

  public:
    _CTraverseReporter (void *caller)
      : m_pCaller (caller)
    {
    }

    void
    report (_PouchPtr p1)
    {
      _hnxgc_traverse_report (m_pCaller, 1, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 2, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 3, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 4, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 5, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 6, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 7, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 8, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 9, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 10, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 11, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 12, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 13, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 14, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 15, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_traverse_report (m_pCaller, 16, (void **)&p1);
    }
  };
}
//
//  HNXGC_TRAVERSE_PTR(pObj, pObj2, ...) - report referent blocks
//
#define HNXGC_TRAVERSE_PTR                                                                                                      \
  ::harnix::_CTraverseReporter reporter (caller); reporter.report


//
//  HNXGC_TRAVERSE_MULTIPLE() - report multiple referent blocks
//
#define HNXGC_TRAVERSE_MULTIPLE_BEGIN   ::harnix::_PouchPtr listReferents[] =
#define HNXGC_TRAVERSE_MULTIPLE_END             _hnxgc_traverse_report (caller,          \
                                                                        sizeof (listReferents) / sizeof (void *), (void **)listReferents);


//
//  HNXGC_TRAVERSE_CHAIN(pObj) - Traverse a nested object
//
#define HNXGC_TRAVERSE_CHAIN(pObj)              (pObj)->_HnxGC_Traverse (caller)

// Example:
//	class CFoo : public CBar {
//		CMemberPtr<CFoo> m_pNext;
//		CMemberPtr<CFoo> m_pPrev;
//		HNXGC_TRAVERSE(CFoo) {
//			HNXGC_TRAVERSE_PTR(m_pNext);
//			HNXGC_TRAVERSE_MULTIPLE_BEGIN
//			{ m_pNext, m_pPrev };
//			HNXGC_TRAVERSE_MULTIPLE_END
//			HNXGC_TRAVERSE_CHAIN((CBar*)this);
//		}
//	};


//
//  HNXGC_ENABLE(cls) - a leave object that no effective reference to others
//
#define HNXGC_ENABLE(cls)                                                                                                       \
  HNXGC_TRAVERSE (cls) { }                                                                                                  \
  void * \
  operator new (size_t size) {                                                                             \
    return ::operator new (size);                                                                           \
  }                                                                                                                                               \
  void *operator new \
  [] (size_t size) {                                                                  \
    return ::operator new (size);                                                                           \
  }

//
//  HNXGC_ONRECLAIM(cls) - define a OnReclaim routine to declare dependences
//
#if defined(HNX_CONFIG__IF_EXISTS)
#define HNXGC_ONRECLAIM()                                                                                                       \
  void _HnxGC_OnReclaim (void *caller)

#else
#define HNXGC_ONRECLAIM()                                                                                                       \
  static void *hasMember (const _typeOnReclaim &) {                                               \
    return (void *)&_hnxgc_class::_HnxGC_OnReclaim_; }                                                       \
  static void _HnxGC_OnReclaim_ (_hnxgc_class * _this, void *caller) {        \
    _this->_HnxGC_OnReclaim (caller); }                                                                      \
  void _HnxGC_OnReclaim (void *caller)

#endif

//
//  OnReclaim Routine Helpers
//
namespace harnix
{
  class _COnReclaimReporter
  {
    void *m_pCaller;

  public:
    _COnReclaimReporter (void *caller)
      : m_pCaller (caller)
    {
    }

    void
    report (_PouchPtr p1)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 1, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 2, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 3, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 4, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 5, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 6, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 7, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 8, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 9, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 10, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 11, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 12, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 13, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 14, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 15, (void **)&p1);
    }

    void
    report (_PouchPtr p1, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr, _PouchPtr)
    {
      _hnxgc_onreclaim_declare (m_pCaller, 16, (void **)&p1);
    }
  };
}
//
//  HNXGC_DEPEND_DECLARE(pObj, pObj2, ...) - declare dependence relationship(s)
//
#define HNXGC_DEPEND_DECLARE                                                                                            \
  ::harnix::_COnReclaimReporter reporter (caller); reporter.report


//
//  HNXGC_DEPEND_MULTIPLE() - declare multiple dependence relationships
//
#define HNXGC_DEPEND_MULTIPLE_BEGIN             ::harnix::_PouchPtr listDependences[] =
#define HNXGC_DEPEND_MULTIPLE_END               _hnxgc_onreclaim_declare (                       \
    sizeof (listDependences) / sizeof (void *), (void **)listDependences);


//
//  HNXGC_RESURRECT() - request resurrention of `this' object
//
#define HNXGC_RESURRECT()                               _hnxgc_onreclaim_resurrect (caller)


//
//	HNXGC_COLLECT() - perform garbage collection
//
#define HNXGC_COLLECT()                 _hnxgc_garbage_collect ()
