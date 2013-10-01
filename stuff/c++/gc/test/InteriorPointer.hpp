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
//	Keep an object alive by an interior pointer
//

class CFoo
{
public:
  int m_nAge;
  int m_nData[50];
  CFoo ()
  {
    printf ("Constructing CFoo[%p]\n", this);
    for (int i = 0; i < 50; i++)
      {
        m_nData[i] = 1000 + i;
      }
  }

  ~CFoo ()
  {
    printf ("Destructing CFoo[%p]\n", this);
  }

  HNXGC_ENABLE (CFoo);
};

CLockedPtr<int>
f1 ()
{
  CLockedPtr<CFoo> p1 = gcnew CFoo;
  CLockedPtr<int> p2 = InteriorPointer (p1->m_nData + 20, p1);
  p1 = nullptr;
  printf (">>> clear p1 >>>\n");
  printf ("*p2 = %u\n", *p2);
  printf (">>> f1 return >>>\n");
  return p2;
}

int
TestMain ()
{
  CLockedPtr<int> p3 = f1 ();
  printf (">>> TestMain() >>>\n");
  printf ("*p = %u\n", *p3);
  for (int i = 0; i < 10; i++)
    {
      printf ("p[%u] = %u \n", i, p3[i]);
    }
  return 0;
}

/*** output ***
   Constructing CFoo[003B2F70]
   >>> clear p1 >>>
 *p2 = 1020
   >>> f1 return >>>
   >>> TestMain() >>>
 *p = 1020
   p[0] = 1020
   p[1] = 1021
   p[2] = 1022
   p[3] = 1023
   p[4] = 1024
   p[5] = 1025
   p[6] = 1026
   p[7] = 1027
   p[8] = 1028
   p[9] = 1029
   Destructing CFoo[003B2F70]
 */
