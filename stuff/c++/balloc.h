/**
 * balloc.h: Block allocation of memory segments.
 *
 * Copyright © 2007 Pippijn van Steenhoven / The Ermyth Team
 * Rights to this code are as documented in doc/pod/gplicense.pod.
 *
 * $Id: balloc.h,v 1.4 2007-09-16 18:54:42 pippijn Exp $
 */

#ifndef ERMYTH_BALLOC_H
#define ERMYTH_BALLOC_H

#include <vector>

extern void (*NullDealloc)(void*);
extern void (*NullNDealloc)(void*, size_t);

template<class T,
         int NewInit = 100, // The number of elements that should be allocated
                            // if we give out our last element
         void* (*Alloc) (size_t) = &::operator new,
         void (*Dealloc) (void *) = &::operator delete,
         void (*NDealloc) (void *, size_t) = NullNDealloc>
struct balloc
{
  static void initialise (int chunks = 100)
  {
    for (int i = 0; i < chunks; ++i)
      {
        T *p = static_cast<T *> (Alloc (sizeof (T)));
        T::get_list ().push_back (p);
      }
  }

  static void finalise ()
  {
    typename alloc_vector::iterator first = T::get_list ().begin ();
    typename alloc_vector::iterator last = T::get_list ().end ();
    while (first != last)
      {
        T *p = *first;
        ++first;
        if (NDealloc != NullNDealloc)
          NDealloc (p, sizeof (T));
        else
          Dealloc (p);
      }
    T::get_list ().clear ();
  }

  void *operator new (size_t bytes)
  {
    // extend our memory by another set of blocks if we run out of them
    if (expect_false (T::get_list ().size () == 0))
      T::initialise (NewInit);
    
    T *p = T::get_list ().back ();
    T::get_list ().pop_back ();
    return p;
  }

  void operator delete (void *p)
  {
    // this cast is legal, because we know that p will always be of type T *
    T::get_list ().push_back (static_cast<T *> (p));
  }

protected:
  balloc ()
  {
  }

  virtual ~balloc ()
  {
  }

private:
  typedef std::vector<T *> alloc_vector;

  static alloc_vector &get_list ()
  {
    static alloc_vector m_free;
    return m_free;
  }
};

#endif
