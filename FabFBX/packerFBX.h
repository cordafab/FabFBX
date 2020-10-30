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

   static void readRigFile(std::string filename,
                           std::string & charFilename,
                           std::string & skelFilename
                           );

   static void readOBJ(std::string filename,
                       std::vector<std::vector<float> > & v,
                       std::vector<std::vector<int> > & f
                       );

   static void readSkeletonFile(std::string filename,
                                std::vector<std::string> & names,
                                std::vector<int> & fathers,
                                std::vector<std::vector<float>> & pos
                                );

private:
   void saveFBX();

   FbxNode* createMesh();
   FbxNode* createSkeleton();
   void createWeights();


   std::string fullPathFile;
   std::string pathFileNoExt;
   std::string pathNoFile;
   std::string filename;

   FbxManager* sdkManager;
   FbxIOSettings * ioSettings;
   FbxScene* fbxScene;
   bool fbxSdkResult;

   FbxNode* mesh;
   FbxNode* skeletonRoot;

   std::vector<std::vector<float>> char_v;
   std::vector<std::vector<int>> char_f;

   std::vector<std::string> joint_names;
   std::vector<int> joint_fathers;
   std::vector<std::vector<float>> joint_pos;
   std::vector<FbxNode*> fbx_joints;
};

#endif // PACKERFBX_H
