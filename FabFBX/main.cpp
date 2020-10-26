#include <iostream>

#include "unpackerFBX.h"
#include "packerFBX.h"

int main(int argc,char *argv[])
{
   UnpackerFBX unpackerFBX;
   bool execStatus = 0;

   if(argc==2)
   {
      if(!unpackerFBX.createUnpacker(argv[1])) return 1;
      execStatus = unpackerFBX.unpack(RIG);
   }
   else if(argc==4)
   {
      if(!unpackerFBX.createUnpacker(argv[1])) return 1;
      std::string characterName(argv[2]);
      std::string skeletonName(argv[3]);
      execStatus = unpackerFBX.unpack(RIG, characterName, skeletonName);
   }
   else if(argc>2)
   {
      std::string flag(argv[2]);

      if(flag.compare("STE")==0)
      {
         if(!unpackerFBX.createUnpacker(argv[1])) return 1;
         if (argc==5)
         {
            std::string characterName(argv[3]);
            std::string skeletonName(argv[4]);
            execStatus = unpackerFBX.unpack(STE, characterName, skeletonName);
         }
      }

      if(flag.compare("ls")==0)
      {
         if(!unpackerFBX.createUnpacker(argv[1])) return 1;
         execStatus = unpackerFBX.list();
      }

      if(flag.compare("anim")==0)
      {
         if(!unpackerFBX.createUnpacker(argv[1])) return 1;
         if (argc==5)
         {
            std::string characterName(argv[3]);
            std::string skeletonName(argv[4]);
            execStatus = unpackerFBX.unpack(ANIM, characterName, skeletonName);
         }
      }

      if(flag.compare("pack")==0)
      {
         PackerFBX packerFBX;
         if(!packerFBX.createPacker(argv[1])) return 1;
         execStatus = packerFBX.pack();
      }
   }

   return !execStatus;
}
