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
  class SListEntry
  {
  public:
    void *m_pNext;
  };

  class CAtomicSListFence
  {
  public:
    static
    void *
    CompareExchangePtr (void *volatile *pVar, void *oldval, void *newval)
    {
      return atomicCompareExchangePtr (pVar, oldval, newval);
    }
  };

  template<typename Traits = CAtomicSListFence>
  class CAtomicSList
  {
  protected:
  public:
    void *m_pFirst;
    CAtomicSList ()
      : m_pFirst (0)
    {
    }

    CAtomicSList (void *pNode)
      : m_pFirst (pNode)
    {
    }

    void
    pushEntry (void *pNode, SListEntry *pEntry)
    {
      void *pOld = (void *)m_pFirst;
      void *pResult;

      for (;;)
        {
          pEntry->m_pNext = pOld;
          pResult = Traits::CompareExchangePtr (&m_pFirst, pOld, pNode);
          if (pResult == pOld)
            {
              break;
            }
          else
            {
              pOld = pResult;
            }
        }
    }

    void *
    flush ()
    {
      SListEntry retval;

      pushEntry (0, &retval);
      return retval.m_pNext;
    }
  };

  template<typename T, SListEntry T::*Field>
  SListEntry *
  SListEntryGetFromPtr (T *pObj)
  {
    return &(pObj->*Field);
  }

  template<typename T, SListEntry * (*pGetField)(T *),
           typename Traits = CAtomicSListFence>
  class SingleLinkedList
    : private CAtomicSList<Traits>
  {
  private:
    static SListEntry *
    getField (T *pObj)
    {
      return pGetField (pObj);
    }

    T * &
    getFirstPtr ()
    {
      // note - don't merge following statements, need for g++ 2.91 and 4.1
      void *p = &this->m_pFirst;
      T **ppTypePunned = reinterpret_cast<T **> (p);

      return *ppTypePunned;
    }

  public:
    SingleLinkedList ()
    {
    }

    SingleLinkedList (T *pFirst)
      : CAtomicSList<Traits> (pFirst)
    {
    }

    void
    atomicPushNode (T *pNode)
    {
      CAtomicSList<Traits>::pushEntry (pNode, pGetField (pNode));
    }

    void
    atomicPushList (T *pNode, SListEntry *pLastEntry)
    {
      CAtomicSList<Traits>::pushEntry (pNode, pLastEntry);
    }

    T *
    atomicFlush ()
    {
      return reinterpret_cast<T *> (CAtomicSList<Traits>::flush ());
    }

    //
    // Following non-atomic methods must not run under racing conditions.
    //
    bool
    isEmpty ()
    {
      return getFirstPtr () == 0;
    }

    SListEntry *
    getLastEntry ()
    {
      T *pNode;
      SListEntry *p = 0;

      for (pNode = getFirstPtr (); pNode;)
        {
          p = getField (pNode);
          pNode = reinterpret_cast<T *> (p->m_pNext);
        }
      return p;
    }

    T *
    popNode ()
    {
      T *retval = getFirstPtr ();

      if (retval)
        {
          getFirstPtr () = (T *)getField (retval)->m_pNext;
        }
      return retval;
    }

    void
    pushNode (T *pNode)
    {
      if (!pNode) return;

      getField (pNode)->m_pNext = getFirstPtr ();
      getFirstPtr () = pNode;
    }

    class Iterator
    {
    public:
      T **m_ppCurrPosi;
      Iterator (SingleLinkedList &lst)
        : m_ppCurrPosi (&lst.getFirstPtr ())
      {
      }

      void
      next ()
      {
        void **ppTypeTunned = &SingleLinkedList::getField (*m_ppCurrPosi)->m_pNext;

        m_ppCurrPosi = reinterpret_cast<T **> (ppTypeTunned);
      }

      void
      remove ()
      {
        *m_ppCurrPosi = (T *)SingleLinkedList::getField (*m_ppCurrPosi)->m_pNext;
      }

      T *
      operator -> ()
      {
        return *m_ppCurrPosi;
      }

      T &
      operator * ()
      {
        return **m_ppCurrPosi;
      }

      operator T * ()
      {
        return *m_ppCurrPosi;
      }
      operator bool ()
      {
        return *m_ppCurrPosi != 0;
      }
    };
    friend class Iterator;              // need for GNU C++
  };
} // end of namespace harnix

/*
   Example:
   class CFoo {
   public:
        CFoo() { printf("Constructing [%p]\n", this); }
        int i;
        SListEntry	m_vLink;
        static SListEntry * GetLink(CFoo * _this) {
                return &_this->m_vLink;
        }
   };

   typedef SingleLinkedList<CFoo, &CFoo::GetLink>	SList;
   SList lst;
   int _tmain(int argc, _TCHAR* argv[])
   {
        lst.atomicPushNode(new CFoo);
        lst.atomicPushNode(new CFoo);
        lst.atomicPushNode(new CFoo);
        SList lst2;
        lst2 = lst.atomicFlush();
        lst.atomicPushNode(new CFoo);
        lst.atomicPushNode(new CFoo);

        for(SList::Iterator iter(lst2); iter; iter.next()) {
                printf("[%p] \n", &(*iter));
        }
        printf("===============\n");
        lst2 = lst.atomicFlush();
        for(SList::Iterator iter(lst2); iter; iter.next()) {
                printf("[%p] \n", &(*iter));
        }
        return 0;
   }
 */
