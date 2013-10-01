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


//
//	automatic invoke garbage collection at exit
//

class CFoo
{
public:
  CFoo ()
  {
    printf ("GCAtExit - CFoo() %p\n", this);
  }

  ~CFoo ()
  {
    printf ("GCAtExit - ~CFoo() %p\n", this);
  }

  mp<CFoo>                                m_pNext;
  HNXGC_TRAVERSE (CFoo)
  {
    HNXGC_TRAVERSE_PTR (m_pNext);
  }
};

int
TestMain ()
{
  lp<CFoo> p = gcnew CFoo;
  p->m_pNext = p;
  //	_hnxgc_garbage_collect(_hnxgc_gc_SkipAtExit);
  return 0;
}

/*** output ***
   Constructing CFoo[00363190]
   Constructing CBar[00363240]
   Destructing CBar[00363240]: Accessing CFoo [00363190] Age = 2007
   Destructing CFoo[00363190]
 */
