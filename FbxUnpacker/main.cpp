#include <iostream>

#include "fbxConvert.h"

using namespace std;

int main(int argc,char *argv[])
{
   if(argc==2)
   {
      convert(argv[1]);
   }

   return 0;
}
