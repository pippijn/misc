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
  class DListEntry
  {
  public:
    void *m_pNext;
    void *m_pPrev;
  };

  template<typename T>
  class _DListEntryT
  {
  public:
    T *m_pNext;
    T *m_pPrev;
  };

  template<typename T, DListEntry * (*pGetField)(T *)>
  class DoubleLinkedList
  {
  private:
    T *m_pHead;
    T *m_pTail;

  public:
    static _DListEntryT<T> *
    getField (T *pObj)
    {
      return reinterpret_cast<_DListEntryT<T> *> (pGetField (pObj));
    }

    DoubleLinkedList ()
    {
      m_pHead = 0;
      m_pTail = 0;
    }

    bool
    isEmpty ()
    {
      return m_pHead == 0;
    }

    T *
    getHead ()
    {
      return m_pHead;
    }

    T *
    getTail ()
    {
      return m_pTail;
    }

    void
    remove (T *_pNode)
    {
      _DListEntryT<T> *pNode = getField (_pNode);
      T *pPrev = pNode->m_pPrev;
      T *pNext = pNode->m_pNext;
      if (pPrev)
        {
          getField (pPrev)->m_pNext = pNext;
        }
      else
        {
          m_pHead = pNext;
        }
      if (pNext)
        {
          getField (pNext)->m_pPrev = pPrev;
        }
      else
        {
          m_pTail = pPrev;
        }
      pNode->m_pPrev = 0;
      pNode->m_pNext = 0;
    }

    void
    removeAll ()
    {
      while (m_pHead)
        {
          remove (m_pHead);
        }
    }

    void
    insertBefore (T *_pNode, T *_pPosi)
    {
      _DListEntryT<T> *pNode = getField (_pNode);
      pNode->m_pNext = _pPosi;
      if (_pPosi)
        {
          T **ppPrev = &getField (_pPosi)->m_pPrev;
          T *pPrev = *ppPrev;
          pNode->m_pPrev = pPrev;
          if (pPrev)
            {
              getField (pPrev)->m_pNext = _pNode;
            }
          *ppPrev = _pNode;
        }
      else
        {
          pNode->m_pPrev = 0;
        }
      if (!pNode->m_pPrev)
        {
          m_pHead = _pNode;
        }
      if (!pNode->m_pNext)
        {
          m_pTail = _pNode;
        }
    }

    void
    insertAfter (T *_pNode, T *_pPosi)
    {
      _DListEntryT<T> *pNode = getField (_pNode);
      pNode->m_pPrev = _pPosi;
      if (_pPosi)
        {
          T **ppNext = &getField (_pPosi)->m_pNext;
          T *pNext = *ppNext;
          pNode->m_pNext = pNext;
          if (pNext)
            {
              getField (pNext)->m_pPrev = _pNode;
            }
          *ppNext = _pNode;
        }
      else
        {
          pNode->m_pNext = 0;
        }
      if (!pNode->m_pNext)
        {
          m_pTail = _pNode;
        }
      if (!pNode->m_pPrev)
        {
          m_pHead = _pNode;
        }
    }

    void
    pushHead (T *pNode)
    {
      insertBefore (pNode, m_pHead);
    }

    void
    pushTail (T *pNode)
    {
      insertAfter (pNode, m_pTail);
    }

    T *
    popHead ()
    {
      T *retval = getHead ();

      if (retval)
        {
          remove (retval);
        }
      return retval;
    }

    T *
    popTail ()
    {
      T *retval = getTail ();

      if (retval)
        {
          remove (retval);
        }
      return retval;
    }

    void
    pushHead (DoubleLinkedList &vList)
    {
      if (vList.m_pTail)
        {
          getField (vList.m_pTail)->m_pNext = m_pHead;
          if (m_pHead)
            {
              getField (m_pHead)->m_pPrev = vList.m_pTail;
            }
          m_pHead = vList.m_pHead;
          if (!m_pTail)
            {
              m_pTail = m_pHead;
            }
          vList.m_pHead = 0;
          vList.m_pTail = 0;
        }
    }

    void
    pushTail (DoubleLinkedList &vList)
    {
      if (vList.m_pHead)
        {
          getField (vList.m_pHead)->m_pPrev = m_pTail;
          if (m_pTail)
            {
              getField (m_pTail)->m_pNext = vList.m_pHead;
            }
          m_pTail = vList.m_pTail;
          if (!m_pHead)
            {
              m_pHead = m_pTail;
            }
          vList.m_pHead = 0;
          vList.m_pTail = 0;
        }
    }

    class Iterator
    {
    public:
      T *m_pCurrPosi;
      Iterator (DoubleLinkedList &lst)
        : m_pCurrPosi (lst.m_pHead)
      {
      }

      void
      next ()
      {
        m_pCurrPosi = DoubleLinkedList::getField (m_pCurrPosi)->m_pNext;
      }

      T *
      operator -> ()
      {
        return m_pCurrPosi;
      }

      T &
      operator * ()
      {
        return *m_pCurrPosi;
      }

      operator T * ()
      {
        return m_pCurrPosi;
      }
      operator bool ()
      {
        return m_pCurrPosi != 0;
      }
    };
    friend class Iterator;                      // need for GNU C++
  };
} // end of namespace harnix

/*
   Example:
   class CFoo {
   public:
        DListEntry	m_link1;
        static DListEntry * GetLink1(CFoo *_this){
                return &_this->m_link1;
        }
        CFoo() {
                printf("CFoo[%p]\n", this);
        }
   };


   typedef DoubleLinkedList<CFoo, &CFoo::GetLink1>		ObjectList;
   ObjectList lst;

   int _tmain(int argc, _TCHAR* argv[])
   {
        lst.pushHead(new CFoo);
        lst.pushHead(new CFoo);
        lst.pushHead(new CFoo);
        for(ObjectList::Iterator iter(lst);iter;iter.next()) {
                printf("[%p]\n", &(*iter));
        }
        return 0;
   }
 */
