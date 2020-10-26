#ifndef PACKERFBX_H
#define PACKERFBX_H

#include <string>
#include <vector>
#include <map>

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

class PackerFBX
{
public:
   PackerFBX();
   bool createPacker(std::string rigPathFile);
   bool pack();

private:
   void saveFBX();
   FbxNode* createMesh();
   FbxNode* createSkeleton();
   void createWeights();

   std::string pathFileNoExt;
   std::string filename;

   FbxManager* lSdkManager;
   FbxIOSettings * ios;
   FbxScene* lScene;
   bool lResult;

   FbxNode* mesh;
   FbxNode* skeletonRoot;
};

#endif // PACKERFBX_H
