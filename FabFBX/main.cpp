#include <iostream>

#include "FabFBX.h"

int main(int argc,char *argv[])
{
   FabFBX fabFBX;

   if(argc!=2) return 1;

   if(!fabFBX.create(argv[1])) return 1;

   bool execStatus;
   execStatus = fabFBX.convert();

   return !execStatus;
}
