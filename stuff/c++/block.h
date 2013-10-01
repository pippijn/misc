#ifndef BLOCK_H
#define BLOCK_H

#include <cstddef>
#include <cassert>

namespace memory
{
  namespace balloc
  {
    /* forward declaration */

    template<class N, size_t S>
    struct block;
    
    template<class N, size_t S>
    struct unit
    {
      block<N, S> *blockptr;
      union
      {
        unit<N, S> *next;
        char node[sizeof (N)];
      };
    };
    
    template<class N, size_t S>
    struct blockhdr
    {
      block<N, S> *prev;
      block<N, S> *next;
      
      blockhdr (block<N, S> *prev = 0, block<N, S> *next = 0)
      : prev (prev), next (next)
      {
      }
    };
    
    template<class N, size_t S>
    struct block : public blockhdr<N, S> 
    {
      size_t units_used;
      unit<N, S> *first_unitptr;
      
      unit<N, S> units[S];
      
      block (block<N, S> *prev, block<N, S> *next)
      : blockhdr<N, S> (prev, next), units_used (1), first_unitptr (&units[1])
      {
        /* link it */
        
        prev->next = next->prev = this;
        
        /* initialize units */
        
        units[0].blockptr = this;
        
        units[S - 1].blockptr = this;
        units[S - 1].next = 0;
        
        unit<N, S> *unitptr = &units[1];
        unit<N, S> *next_unitptr = &units[2];
        
        for (size_t n = S - 2; n--;)
          {
            unitptr->blockptr = this;
            unitptr++->next = next_unitptr++;
          }
      }
    };
    
    template<class T, size_t S, class N = T>
    class allocator
    {
    private:
      typedef T node_type;

    public:
      typedef size_t size_type;
      typedef ptrdiff_t difference_type;
      typedef T *pointer;
      typedef const T *const_pointer;
      typedef T &reference;
      typedef const T &const_reference;
      typedef T value_type;
      
      template<typename O> struct rebind
      {
        typedef allocator<O, S> other;
      };
      
      pointer address (reference x) const
      {
        return &x;
      }
      const_pointer address (const_reference x) const
      {
        return &x;
      }
      
      allocator ()
      {
        head.next = reinterpret_cast<block *> (&tail);
        tail.prev = reinterpret_cast<block *> (&head);
        
        assert (S >= 2);
      }
      
      template<class T1, class N1>
      allocator (const allocator<T1, S, N1> &)
      {
        head.next = reinterpret_cast<block *> (&tail);
        tail.prev = reinterpret_cast<block *> (&head);
        assert (S >= 2);
      }
      
      allocator<T, S, N> &operator = (const allocator<T, S, N> &)
      {
        return *this;
      }
      
      pointer allocate (size_type n, const void *hint = 0)
      {
        assert (n == 1);
        return reinterpret_cast<pointer> (_Charalloc (sizeof (T)));
      }
      
      char *_Charalloc (size_type n)
      {
        assert (n == sizeof (node_type));
        
        unit *unitptr;
        
        if (head.next == &tail)
          {
            /* no chunks available */
            new block (reinterpret_cast<block *> (&head),  /* get a new one */
                       reinterpret_cast<block *> (&tail));
            unitptr = &head.next->units[0]; /* use its first unit */
          }
        else
          {
            unitptr = head.next->first_unitptr; /* get the unit */
            ++head.next->units_used; /* and log it */
            if ((head.next->first_unitptr = unitptr->next) == 0)
              {
                /* no more units available in this block, delink it */
                
                head.next = head.next->next;
                head.next->prev = reinterpret_cast<block *> (&head);
              }
          }
        
        /* return the node portion of the unit */
        
        return reinterpret_cast<char *> (&unitptr->node);
      }
      
      void deallocate (void *p, size_type n)
      {
        assert (p != 0 && n == 1);
        
        unit *unitptr = reinterpret_cast<unit *> (reinterpret_cast<char *> (p) - offsetof (unit, node));
        block *blockptr = unitptr->blockptr;
        
        if (--blockptr->units_used == 0)
          {
            blockptr->prev->next = blockptr->next;
            blockptr->next->prev = blockptr->prev;
            delete blockptr;
          }
        else if ((unitptr->next = blockptr->first_unitptr) == 0)
          {
            blockptr->first_unitptr = unitptr;
            
            blockptr->next = head.next;
            head.next = head.next->prev = blockptr;
          }
        else
          {
            blockptr->first_unitptr = unitptr;
          }
      }
      
      
      void construct (pointer p, const T &val)
      {
        new ((void*)p)T (val);
      }
      
      void destroy (pointer p)
      {
        p->T::~T ();
      }
      
      size_type max_size () const
      {
        size_type n = (size_type) (-1) / sizeof (node_type);
        return (0 < n ? n : 1);
      }
      
      
      bool operator == (const allocator<T, S, N> &r) const
      {
        return this == &r;
      }
      
      bool operator != (const allocator<T, S, N> &r) const
      {
        return this != &r;
      }
      
    private:
      typedef unit<node_type, S> unit;
      typedef blockhdr<node_type, S> blockhdr;
      typedef block<node_type, S> block;
      
      blockhdr head;
      blockhdr tail;
    };
  } // namespace balloc

  template<typename T>
  class block
  {
    void *operator new (size_t bytes)
    {
      return balloc::allocator<T, sizeof (T)>::allocate (bytes);
    }

    void operator delete (void *ptr)
    {
      balloc::allocator<T, sizeof (T)>::deallocate (ptr);
    }

    void destroy ()
    {
      delete this;
    }
  };
} // namespace memory

#endif // BLOCK_H
