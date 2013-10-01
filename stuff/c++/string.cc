#include <iostream>

using namespace std;

int main() {
   unsigned char one[10]   = "moo1";
   unsigned char two[10]   = "moo2";
   unsigned char three[10] = "moo3";
   unsigned char four[10]  = "moo4";

   string s = string((const char*)one) + string((const char*)two)
              + string((const char*)three) + string((const char*)four);

   cout << s << endl;
}
