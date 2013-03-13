#include <vector>
#include <iostream>

// This program was used to demonstrate a bug in EZTrace in October 2011
// when compiled with -O3, running the program with eztrace causes a SIGSEGV
// this was due to the memory module:
// the implementation of malloc returned a buffer that is not 'aligned'

// This bug was fixed by commit #701

int main (int argc, char **argv)
{
  int rank = 0;
  std::cout << "MyRank : "<<rank <<std::endl;
  std::vector<double> a;
  a.resize(10);

  return 0;
}
