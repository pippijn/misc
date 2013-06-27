#ifndef XMALLOC_H
#define XMALLOC_H

#include <stddef.h>

/*@only@*/ void *xmalloc (size_t bytes);
void xfree (/*@only@*/ /*@out@*/ void *ptr);

#endif /* XMALLOC_H */
