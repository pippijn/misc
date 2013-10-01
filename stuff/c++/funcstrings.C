#include <boost/function.hpp>
#include <boost/implicit_cast.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <exception>
#include <map>
#include <string>
#include <sstream>
#include <iterator>
#include <iostream>
#include <vector>
#include <cmath>
 
using std::string;
using std::vector;
 
template <typename T, typename A>
struct string_wrapper
{
  T (* p) (A);
 
  string_wrapper(T (* const p_) (A)): p(p_) {}
 
  string operator() (vector<string> const & v) const
  { return boost::lexical_cast<string>(p(boost::lexical_cast<A>(v.at(0)))); }
};
 
template <typename T, typename A>
string_wrapper<T, A> string_wrap(T (* const p) (A)) { return p; }
 
int main()
{
  try
  {
    typedef std::map<string, boost::function<string(vector<string> const &)> > Commands;
    Commands commands;
 
    commands["sin"] = string_wrap(boost::implicit_cast<double(*)(double)>(std::sin));
    commands["cos"] = string_wrap(boost::implicit_cast<double(*)(double)>(std::cos));
 
    string cmd, args;
    while(std::cout << "command: ", std::getline(std::cin >> cmd, args))
    {
      Commands::const_iterator const i = commands.find(cmd);
      if(i == commands.end()) throw std::runtime_error("no such command");
      std::istringstream ss(args);
      std::istream_iterator<string> b(ss), e;
      std::cout << i->second(vector<string>(b, e)) << '\n';
    }
  }
  catch (std::exception const & e)
  {
    std::cerr << "error: " << e.what() << '\n';
  }
}
