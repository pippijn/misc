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
//	CFoo and CBar reference each other, and CBar depends on CFoo
//

class CBar;

class CFoo
{
private:
  int *m_pAge;

public:
  CMemberPtr<CFoo>                m_pNext;              // refer to self (circular referenced)
  CMemberPtr<CBar>                m_pBar;               // refer to CBar (circular referenced)
  CFoo ()
  {
    printf ("Constructing CFoo[%p]\n", this);
    m_pAge = new int(2007);
  }

  ~CFoo ()
  {
    printf ("Destructing CFoo[%p]\n", this);
    delete m_pAge;
  }

  int
  getAge ()
  {
    return *m_pAge;
  }

  HNXGC_TRAVERSE (CFoo)
  {
    HNXGC_TRAVERSE_MULTIPLE_BEGIN
    {
      m_pNext, m_pBar
    };
    HNXGC_TRAVERSE_MULTIPLE_END
  }
};

class CBar
{
public:
  CMemberPtr<CFoo>                m_pFoo;               // refer to CFoo
  CBar ()
  {
    printf ("Constructing CBar[%p]\n", this);
  }

  ~CBar ()
  {
    // accessing the CFoo object, which is guaranteed will not be
    // destructed prior to this destruction.
    printf ("Destructing CBar[%p]: Accessing CFoo [%p] Age = %u\n",
            this, (void *)m_pFoo, m_pFoo->getAge ());
  }

  HNXGC_TRAVERSE (CBar)
  {
    HNXGC_TRAVERSE_PTR (m_pFoo);
  }
  HNXGC_ONRECLAIM ()
  {
    // claim the dependence relationship
    HNXGC_DEPEND_DECLARE (m_pFoo);
  }
};

int
TestMain ()
{
  CLockedPtr<CFoo> p = gcnew CFoo;
  p->m_pBar = gcnew CBar;
  p->m_pBar->m_pFoo = p;
  p->m_pNext = p;
  p = nullptr;

  // CFoo and CBar are unreachable now
  HNXGC_COLLECT ();
  return 0;
}

/*** output ***
   Constructing CFoo[00363190]
   Constructing CBar[00363240]
   Destructing CBar[00363240]: Accessing CFoo [00363190] Age = 2007
   Destructing CFoo[00363190]
 */
