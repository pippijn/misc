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

#if !defined(HNXGC_CONFIG_NOSHORTHAND)
#define CWeakPtr                wp
#define CLockedPtr              lp
#define CMemberPtr              mp
#endif


namespace harnix
{
  // Null Pointer type
  class _typeNullPtr
  {
  };


  // _IndirectPointer - sub routines manipulating indirect pointer
  class _IndirectPointer
  {
  public:
    static bool
    Test (void *ptr)
    {
      return (((unsigned)(size_t) ptr) & 1) != 0;
    }

    template<typename T>
    static T *
    Set (T *ptr)
    {
      if (!Test (ptr))
        {
          return reinterpret_cast<T *> (reinterpret_cast<char *> (ptr) + 1);
        }
      else
        {
          return ptr;
        }
    }

    template<typename T>
    static T *
    Clr (T *ptr)
    {
      if (Test (ptr))
        {
          return reinterpret_cast<T *> (reinterpret_cast<char *> (ptr) - 1);
        }
      else
        {
          return ptr;
        }
    }

    template<typename T>
    static T *
    Get (T *ptr)
    {
      if (Test (ptr))
        {
          return *(T **)Clr (ptr);
        }
      else
        {
          return ptr;
        }
    }
  };

  //
  // CWeakPtr - ignored by GC system, can be used to replace IN type of function params
  //
  template<typename T>
  class CWeakPtr
  {
    T *m_pObj;

  public:
    //
    // NULL pointer
    //
    CWeakPtr (const _typeNullPtr *) throw()
      : m_pObj (0)
    {
    }

    const _typeNullPtr *
    operator = (const _typeNullPtr *p)
    {
      m_pObj = 0;
      return p;
    }

#if !defined(_MANAGED)
    CWeakPtr (const nullptr_t &) throw()
      : m_pObj (0)
    {
    }

    const nullptr_t &
    operator = (const nullptr_t &p)
    {
      m_pObj = 0;
      return p;
    }
#endif



    void
    _setRaw (void *pObj)
    {
      m_pObj = reinterpret_cast<T *> (pObj);
    }

    void *
    _getRaw () const
    {
      return m_pObj;
    }

    T *
    _get () const
    {
      return _IndirectPointer::Get (m_pObj);
    }

    CWeakPtr () throw()
      : m_pObj (0)
    {
    }

    ~CWeakPtr ()  throw()
    {
    }

    operator T * () const throw() {
      return _get ();
    }
    T *
    operator -> () const throw()
    {
      return _get ();
    }

    T &
    operator * () const throw()
    {
      return *_get ();
    }

    operator _PouchPtr () const throw() {
      return _PouchPtr (m_pObj);
    }
  };

  // _CWeakPtrProxy - used by CLockedPtr to ensure appropriate conversion path
  template<typename T>
  class _CWeakPtrProxy
  {
    void *m_pObj;

  public:
    void *
    _getRaw () const
    {
      return m_pObj;
    }

    _CWeakPtrProxy (const CWeakPtr<T> &v)
    {
      m_pObj = v._getRaw ();
    }
  };

  //
  // CLockedPtr - Strong reference to object, as return type, auto variable.
  //
  template<typename T>
  struct _CLockedPtrRef
  {
    void *m_pPtr;
  };

  template<typename T>
  class CLockedPtr
  {
    T *m_pObj;
    T *
    _get () const
    {
      return _IndirectPointer::Get (m_pObj);
    }

  public:
    CLockedPtr () throw()
      : m_pObj (0)
    {
    }

    ~CLockedPtr ()  throw()
    {
      if (m_pObj)
        {
          _hnxgc_assign_lptr (&m_pObj, 0);
        }
    }

    //
    // NULL pointer
    //
    CLockedPtr (const _typeNullPtr *) throw()
      : m_pObj (0)
    {
    }

    const _typeNullPtr *
    operator = (const _typeNullPtr *p)
    {
      this->~CLockedPtr ();
      // m_pObj = 0;
      return p;
    }

#if !defined(_MANAGED)
    CLockedPtr (const nullptr_t &) throw()
      : m_pObj (0)
    {
    }

    const nullptr_t &
    operator = (const nullptr_t &p)
    {
      this->~CLockedPtr ();
      // m_pObj = 0;
      return p;
    }
#endif


    //
    // Move Constructor / Assignment - move reference from operand to this
    //
    // Note - moving locked reference is transparent to garbage collector,
    // therefore clearing can precede storing, and it is allowed to lose all
    // references to a locked object for a transient period.
    CLockedPtr (CLockedPtr<T> &v) throw()
    {
      m_pObj = v.m_pObj;
      v.m_pObj = 0;
    }

    CWeakPtr<T>
    operator = (CLockedPtr<T> &v) throw()
    {
      if (m_pObj != v.m_pObj)
        {
          this->~CLockedPtr ();
          new (this)CLockedPtr<T> (v);
        }
      return *this;
    }

    CLockedPtr (_CLockedPtrRef<T> v) throw()
    {
      m_pObj = *reinterpret_cast<T **> (v.m_pPtr);
      *reinterpret_cast<T **> (v.m_pPtr) = 0;
    }

    CWeakPtr<T>
    operator = (_CLockedPtrRef<T> v) throw()
    {
      if (m_pObj != *reinterpret_cast<T **> (v.m_pPtr))
        {
          this->~CLockedPtr ();
          new (this)CLockedPtr<T> (v);
        }
      return *this;
    }

    //
    // Copy Constructor / Assignment - duplicate a reference to this CLockedPtr
    //
    CLockedPtr (_CWeakPtrProxy<T> v) throw()
      : m_pObj (0)
    {
      if (v._getRaw ())
        {
          _hnxgc_assign_lptr (&m_pObj, v._getRaw ());
        }
    }

    CWeakPtr<T>
    operator = (_CWeakPtrProxy<T> v) throw ()
    {
      if (m_pObj != v._getRaw ())
        {
          _hnxgc_assign_lptr (&m_pObj, v._getRaw ());
        }
      return *this;
    }

    //
    // Conversion - export weak reference
    //
    operator CWeakPtr<T> () const throw() {
      CWeakPtr<T> retval;
      retval.
      _setRaw (m_pObj);
      return retval;
    }
    operator _CLockedPtrRef<T> () throw() {
      _CLockedPtrRef<T> retval;
      retval.m_pPtr = &m_pObj;
      return retval;
    }

    //
    // duplicate - explicit make a copy of reference.
    //
    CLockedPtr<T>
    duplicate () const throw()
    {
      return CLockedPtr<T
                        > (CWeakPtr<T> (*this));
    }

    //
    // attach - clear original reference and assume the new reference
    //
    CWeakPtr<T>
    attach (void *p) throw()
    {
      this->~CLockedPtr ();
      m_pObj = reinterpret_cast<T *> (p);
      return *this;
    }

    //
    // detach - return original reference and reset the smart pointer
    //
    void *
    detach () throw()
    {
      void *retval = m_pObj;

      m_pObj = 0;
      return retval;
    }

    operator T * () const throw() {
      return _get ();
    }
    T *
    operator -> () const throw()
    {
      return _get ();
    }

    T &
    operator * () const throw()
    {
      return *_get ();
    }

    operator _PouchPtr () const throw() {
      return _PouchPtr (m_pObj);
    }
  };
  // Note
  //	1 - assignment and attach operations must not return a 'CLockedPtr<T>'
  //		type of *this, which may "move" the reference to a temporary smart
  //		pointer and then finally lost the reference.
  //  2 - should not define 'const CLockedPtr<T> &' style copy/assign methods.
  //		Temporary objects are favor to 'const CLockedPtr<T> &' than
  //		'CLockedPtr<T> &'.


  //
  // CMemberPtr - working as field of managed object
  //
  template<typename T>
  class CMemberPtr
  {
    T *m_pObj;
    T *
    _get () const
    {
      return _IndirectPointer::Get (m_pObj);
    }

  public:
    CMemberPtr () throw()
      : m_pObj (0)
    {
    }

    ~CMemberPtr ()  throw()
    {
      if (m_pObj)
        {
          _hnxgc_assign_mptr (&m_pObj, 0);
        }
    }

    //
    // NULL pointer
    //
    CMemberPtr (const _typeNullPtr *) throw()
      : m_pObj (0)
    {
    }

    const _typeNullPtr *
    operator = (const _typeNullPtr *p)
    {
      this->~CMemberPtr ();
      // m_pObj = 0;
      return p;
    }

#if !defined(_MANAGED)
    CMemberPtr (const nullptr_t &) throw()
      : m_pObj (0)
    {
    }

    const nullptr_t &
    operator = (const nullptr_t &p)
    {
      this->~CMemberPtr ();
      // m_pObj = 0;
      return p;
    }
#endif


    //
    // Copy Constructor / Assignment - duplicate the reference to the referent object
    //
    CMemberPtr (const CMemberPtr &v) throw()
      : m_pObj (0)
    {
      if (v.m_pObj)
        {
          _hnxgc_assign_mptr (&m_pObj, v.m_pObj);
        }
    }

    CMemberPtr &
    operator = (const CMemberPtr &v) throw()
    {
      if (m_pObj != v.m_pObj)
        {
          _hnxgc_assign_mptr (&m_pObj, v.m_pObj);
        }
      return *this;
    }

    //
    // construct/assign from a weak pointer
    //
    CMemberPtr (CWeakPtr<T> pObj) throw()
      : m_pObj (0)
    {
      if (pObj._getRaw ())
        {
          _hnxgc_assign_mptr (&m_pObj, pObj._getRaw ());
        }
    }

    CMemberPtr &
    operator = (CWeakPtr<T> pObj) throw ()
    {
      if (m_pObj != pObj._getRaw ())
        {
          _hnxgc_assign_mptr (&m_pObj, pObj._getRaw ());
        }
      return *this;
    }

    //
    // Conversion - export weak reference and weakproxy for CLockedPtr
    //
    operator CWeakPtr<T> () const {
      CWeakPtr<T> retval;
      retval.
      _setRaw (m_pObj);
      return retval;
    }
    operator _CWeakPtrProxy<T> () const throw() {
      return _CWeakPtrProxy<T
                            > (this->operator CWeakPtr<T> ());
    }


    operator T * () const throw() {
      return _get ();
    }
    T *
    operator -> () const throw()
    {
      return _get ();
    }

    T &
    operator * () const throw()
    {
      return *_get ();
    }

    operator _PouchPtr () const throw() {
      return _PouchPtr (m_pObj);
    }
  };

  //
  //  gcnew - create a managed object or an array of managed objects
  //
  //  (internal use) - create CLockedPtr at gcnew operation
  class _OpNewHelper
  {
  public:
    template<typename T>
    CLockedPtr<T>
    operator << (T *p)
    {
      CLockedPtr<T> retval;
      retval.attach (p);
      return retval;
    }
  };
#define HNXGC_NEW       harnix::_OpNewHelper () << new (*(harnix::_typeGCNew *)0)

#if     !defined(HNXGC_CONFIG_NOSHORTHAND)
#define gcnew                   HNXGC_NEW
#endif
  // Example:
  //	 p = gcnew CFoo(7, "Seven");
  //	 p = gcnew CFoo[7];
  //


  //
  //	Interior Pointer Base
  //
  class _CInteriorPtrBase
  {
    void *m_pData;                                              // points to interior native data
    _PouchPtr m_pObj;                                           // points to managed block
    _CInteriorPtrBase (void *pData, _PouchPtr pObj) throw()
      : m_pData (pData)
      , m_pObj (0)
    {
      if (pObj)
        {
          _hnxgc_assign_mptr (&m_pObj, pObj);
        }
    }

  public:
    ~_CInteriorPtrBase ()
    {
      if (m_pObj)
        {
          _hnxgc_assign_mptr (&m_pObj, 0);
        }
    }

    template<typename T>
    static CLockedPtr<T>
    CreateInstance (T *pData, _PouchPtr pObj)
    {
      CLockedPtr<T> retval;
      CLockedPtr<_CInteriorPtrBase> p = HNXGC_NEW _CInteriorPtrBase (pData, pObj);
      retval.attach (_IndirectPointer::Set (p.
                                            detach ()));
      return retval;
    }

    HNXGC_TRAVERSE (_CInteriorPtrBase)
    {
      HNXGC_TRAVERSE_PTR (::harnix::_PouchPtr (m_pObj));
    }
    operator _PouchPtr () const throw() {
      return m_pObj;
    }
  };


  //
  //  InteriorPointer - associate interior data address with managed object
  //
  template<typename T>
  static inline
  CLockedPtr<T>
  InteriorPointer (T *pData, _PouchPtr pObj)
  {
    if (pData == pObj)
      {
        CWeakPtr<T> pWeak;
        pWeak._setRaw (pData);
        return CLockedPtr<T> (pWeak);
      }
    else if (_IndirectPointer::Test (pObj))
      {
        return _CInteriorPtrBase::CreateInstance (pData,
                                                  *reinterpret_cast<_CInteriorPtrBase *> (
                                                    _IndirectPointer::Clr ((void *)pObj)));
      }
    else
      {
        return _CInteriorPtrBase::CreateInstance (pData, pObj);
      }
  }

  //
  // interior_cast<T>(expr) - convert to pointer to base class
  //
  template<typename T, typename U>
  static inline
  CLockedPtr<T>
  interior_cast (CWeakPtr<U> p)
  {
    return InteriorPointer ((T *)(U *)p, p);
  }

  template<typename T, typename U>
  static inline
  CLockedPtr<T>
  interior_cast (CLockedPtr<U> p)
  {
    return InteriorPointer ((T *)(U *)p, p);
  }

  template<typename T, typename U>
  static inline
  CLockedPtr<T>
  interior_cast (CMemberPtr<U> p)
  {
    return InteriorPointer ((T *)(U *)p, p);
  }

  //
  //  HNXGC_DELETE(p) - mandatory delete an object
  //
  static inline
  void
  _hnxgcDelete (_PouchPtr pObj)
  {
    if (_IndirectPointer::Test (pObj))
      {
        _hnxgc_destruct_object ((_PouchPtr) * reinterpret_cast<_CInteriorPtrBase *> (
                                  _IndirectPointer::Clr ((void *)pObj)));
      }
    else
      {
        _hnxgc_destruct_object (pObj);
      }
  }

#define HNXGC_DELETE(x)                         _hnxgcDelete (x)
} // end of namespace harnix



//
//	gcnew_native(cls) - create native object(s) in managed heap
//

template<typename T>
inline
void *
operator new (size_t size, const harnix::_typeGCNewNative<T> &)
{
  return _hnxgc_operator_new (
           harnix::CRuntimeClassInfoNative<T>::GetIRuntimeClassInfo (),
           sizeof (T), size, 0);
}

template<typename T>
inline
void
operator delete (void *pMem, const harnix::_typeGCNewNative<T> &)
{
}

template<typename T>
inline
void *operator new
[] (size_t size, const harnix::_typeGCNewNative<T> &)
{
  if (sizeof (T) <= sizeof (size_t))
    {
      static size_t nHasDtor = size_t (-1);
      if (nHasDtor == size_t (-1))
        {
          nHasDtor =
            harnix::_CDetectDtor<(sizeof (T)> sizeof (size_t)) > ::
                                 Test (*(T *)0);
                                 }
                                 return _hnxgc_operator_new (
                                   harnix::CRuntimeClassInfoNative<T>::GetIRuntimeClassInfo (),
                                   sizeof (T), size, nHasDtor);
                                 }
                                 else
                                 {
                                   return _hnxgc_operator_new (
                                     harnix::CRuntimeClassInfoNative<T>::GetIRuntimeClassInfo (),
                                     sizeof (T), size, 0);
                                 }
                                 }

                                 template<typename T>
                                 inline
                                 void operator delete
                                 [] (void *pMem, const harnix::_typeGCNewNative<T> &)
                                 {
                                 }

#define HNXGC_NEW_NATIVE(cls)                                                                                           \
  harnix::_OpNewHelper () << ::new (*(harnix::_typeGCNewNative < cls > *) 0)cls
#define gcnew_native                    HNXGC_NEW_NATIVE
                                 /*
                                    Example:
                                         CLockedPtr<char> p = gcnew_native(char) [100];
                                         CLockedPtr<CStudent> p = gcnew_native(CStudent) ("brian", 33, "465 imperial st");
                                  */


                                 // note - if you don't want to pollute much CXxxPtr with Xp, uncomment following lines.
                                 //#if !defined(HNXGC_CONFIG_NOSHORTHAND)
                                 //#undef CWeakPtr
                                 //#undef CLockedPtr
                                 //#undef CMemberPtr
                                 //#endif
