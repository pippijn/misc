#include <iostream>
#include <sstream>

using namespace std;

int main() {
   unsigned char one[10]   = "moo1";
   unsigned char two[10]   = "moo2";
   unsigned char three[10] = "moo3";
   unsigned char four[10]  = "moo4";

   ostringstream s;
   s << one << two << three << four << endl;

   cout << s.str();
}
