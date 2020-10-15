#include <iostream>

#include "FabFBX.h"

int main(int argc,char *argv[])
{
   FabFBX fabFBX;
   bool execStatus = 0;

   if(argc==2)
   {
      if(!fabFBX.createUnpacker(argv[1])) return 1;
      execStatus = fabFBX.unpack(RIG);
   }
   else if(argc==4)
   {
      if(!fabFBX.createUnpacker(argv[1])) return 1;
      std::string characterName(argv[2]);
      std::string skeletonName(argv[3]);
      execStatus = fabFBX.unpack(RIG, characterName, skeletonName);
   }
   else if(argc>2)
   {
      std::string flag(argv[2]);

      if(flag.compare("STE")==0)
      {
         if(!fabFBX.createUnpacker(argv[1])) return 1;
         if (argc==5)
         {
            std::string characterName(argv[3]);
            std::string skeletonName(argv[4]);
            execStatus = fabFBX.unpack(STE, characterName, skeletonName);
         }
      }

      if(flag.compare("ls")==0)
      {
         if(!fabFBX.createUnpacker(argv[1])) return 1;
         execStatus = fabFBX.list();
      }

      if(flag.compare("anim")==0)
      {
         if(!fabFBX.createUnpacker(argv[1])) return 1;
         if (argc==5)
         {
            std::string characterName(argv[3]);
            std::string skeletonName(argv[4]);
            execStatus = fabFBX.unpack(ANIM, characterName, skeletonName);
         }
      }
   }

   return !execStatus;
}
