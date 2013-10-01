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
//	Resurrection - put the unreachable object back to live world
//

class CFoo
{
private:
  int m_nAge;
  bool m_bResurrected;

public:
  CMemberPtr<CFoo>                m_pNext;              // refer to self (circular referenced)
  CFoo ()
  {
    printf ("Constructing CFoo[%p]\n", this);
    m_nAge = 1234;
    m_bResurrected = false;
  }

  ~CFoo ()
  {
    printf ("Destructing CFoo[%p]\n", this);
  }

  HNXGC_TRAVERSE (CFoo)
  {
    HNXGC_TRAVERSE_PTR (m_pNext);
  }
  HNXGC_ONRECLAIM ()
  {
    if (!m_bResurrected)
      {
        m_bResurrected = true;
        s_pDying = InteriorPointer (this, _PouchPtr (this));
        s_pDying->m_pNext = nullptr;
        printf ("OnReclaim: Resurrect %p\n", this);
        HNXGC_RESURRECT ();
      }
  }
  static CLockedPtr<CFoo> s_pDying;             // hold the resurrected object
};

CLockedPtr<CFoo> CFoo::s_pDying;

void
Unreachable ()
{
  // resurrected from circular referenced garbage
  CLockedPtr<CFoo> p = gcnew CFoo;
  p->m_pNext = p;
  p = nullptr;

  HNXGC_COLLECT ();
  printf ("s_pDying = %p\n", (void *)CFoo::s_pDying);
  CFoo::s_pDying = nullptr;
}

void
ZeroRC ()
{
  // resurrected from zero-referenced object
  CLockedPtr<CFoo> p = gcnew CFoo;
  p = nullptr;
  printf ("s_pDying = %p\n", (void *)CFoo::s_pDying);
  CFoo::s_pDying = nullptr;
}

int
TestMain ()
{
  //Unreachable();
  ZeroRC ();
  return 0;
}

/*** output ***
   Constructing CFoo[00363400]
   OnReclaim: Resurrect 00363400
   s_pDying = 00363400
   Destructing CFoo[00363400]
   Constructing CFoo[00363400]
   OnReclaim: Resurrect 00363400
   s_pDying = 00363400
   Destructing CFoo[00363400]
 */
