#ifndef RONA_MALLOC_H
#define RONA_MALLOC_H

#include <cstdlib>
#include <new>

namespace rona
{
  namespace nothrow
  {
    namespace detail
    {
      struct memory
      {
        memory (std::size_t x) throw ()
        : n (x)
        {
        }
        
        template<typename T>
        operator T * () const throw ()
        {
          return static_cast<T *> (std::malloc (n * sizeof (T)));
        }

      private:
        const std::size_t n;
        memory ();
      };
      
      template<>
      inline memory::operator void * () const throw ()
      {
        return this->operator char * ();
      }
    }

    static inline detail::memory
    malloc (std::size_t n = 1) throw ()
    {
      return detail::memory (n);
    }
    
    template<typename T>
    static inline T *
    realloc (T *p, std::size_t n) throw ()
    {
      return static_cast<T *> (std::realloc (p, n * sizeof (T)));
    }
    
    template<>
    static inline void *
    realloc (void *p, std::size_t n) throw ()
    {
      return std::realloc (p, n);
    }
  
    using std::free;
  }
}

namespace rona
{
  namespace throws
  {
    namespace detail
    {
      struct memory
      {
        memory (std::size_t x) throw ()
        : n (x)
        {
        }
        
        template<typename T>
        operator T * () const throw (std::bad_alloc)
        {
          T * const ptr = static_cast<T *> (std::malloc (n * sizeof (T)));
          if (!ptr)
            throw std::bad_alloc ();
          
          return ptr;
        }
  
      private:
        const std::size_t n;
        memory ();
      };
      
      template<>
      inline memory::operator void * () const throw (std::bad_alloc)
      {
        return this->operator char * ();
      }
    }
    
    static inline detail::memory
    malloc (std::size_t n = 1) throw (std::bad_alloc)
    {
      return detail::memory (n);
    }
    
    template<typename T>
    static inline T *
    realloc (T *p, std::size_t n) throw (std::bad_alloc)
    {
      T *const ptr = static_cast<T *> (std::realloc (p, n * sizeof (T)));
      if (!ptr)
        throw std::bad_alloc ();
      
      return ptr;
    }
    
    template<>
    static inline void *
    realloc (void *p, std::size_t n) throw (std::bad_alloc)
    {
      return realloc<char> (static_cast<char *> (p), n);
    }
    
    using std::free;
  }
}

namespace rona
{
  namespace nothrow
  {
    namespace detail
    {
      struct placement
      {
        placement (std::size_t x) throw ()
        : n (x)
        {
        }
        
        template<typename T>
        operator T * () const throw ()
        {
          T *ptr = malloc (n);
          return new (ptr) T;
        }

      private:
        const std::size_t n;
        placement ();
      };
    }

    static inline detail::placement
    construct (std::size_t n = 1) throw ()
    {
      return detail::placement (n);
    }
  }

  namespace throws
  {
    namespace detail
    {
      struct placement
      {
        placement (std::size_t x) throw ()
        : n (x)
        {
        }
        
        template<typename T>
        operator T * () const throw (std::bad_alloc)
        {
          T *ptr = malloc (n);
          return new (ptr) T;
        }

      private:
        const std::size_t n;
        placement ();
      };
    }

    static inline detail::placement
    construct (std::size_t n = 1) throw (std::bad_alloc)
    {
      return detail::placement (n);
    }
  }
}

#endif // RONA_MALLOC_H
