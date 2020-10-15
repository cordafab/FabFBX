#ifndef FBXCONVERT_H
#define FBXCONVERT_H

#include <string>
#include <vector>
#include <map>

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

#include "weights.h"

#define RIG 0
#define ANIM 1
#define STE 99

class FabFBX{

public:
   FabFBX();

   bool createUnpacker(std::string fbxPathFile);

   bool unpack(int flag,
                std::string characterName = "Character",
                std::string skeletonName  = "Skeleton"
                );

   bool unpackCharacterGeometry(
         const std::string & characterNodeName,
         std::vector<double> & v,
         std::vector<std::vector<int>> & f
         );
   bool unpackSkeletonTopology(
         const std::string & skeletonNodeName,
         std::vector<std::string> & jointsNames,
         std::vector<double> & jointsPositions,
         std::vector<int> & fathers,
         std::map<std::string, unsigned long> & jointIdByName
         );
   bool unpackSkeletonWeights(
         const std::string & characterNodeName,
         const std::vector<std::string> & jointsNames,
         const std::map<std::string, unsigned long> & jointIdByName,
         Weights & skeletonWeights
         );
   bool unpackSkeletonAnimation(
         const std::string & skeletonNodeName,
         const std::vector<std::string> & jointsNames,
         const std::map<std::string, unsigned long> & jointIdByName,
         std::vector<double> & keyframesTimes,
         std::vector<std::vector<std::vector<double>>> & skelKeyframes //THIS IS UGLY
         );

   bool list();

   static void navigateSkeleton(
         std::vector<std::string> & names,
         std::vector<double> & jointsPositions,
         std::vector<int> & fathers,
         FbxNode * node,
         int father
         );

   static void saveOBJ(
         const std::string & filename,
         const std::vector<double> & v,
         const std::vector<std::vector<int> > & f
         );

   static void saveSkeleton(
         const std::string & filename,
         const std::vector<std::string> & names,
         const std::vector<double> & jointsPositions,
         const std::vector<int> & fathers
         );

   static void saveWeights(
         const std::string & filename,
         const Weights & weights
         );

   static void saveAnimation(
         const std::string & filename,
         const std::vector<double> & t,
         std::vector<std::vector<std::vector<double>>> & skelKeyframes //This is sooo ugly D: (but i don't care lol)
         );

   static void getNodeKeyframe (
         FbxNode * node,
         const FbxTime & t,
         std::vector<std::vector<double>> & deformedKeyframes,
         const std::map<std::string, unsigned long> & nodeIdByName,
         double scaleFactor
         );

   static std::vector<double> fromFbxMatrixToVector(
         const FbxAMatrix & matrix
         );

private:
   std::string pathFileNoExt;
   std::string filename;

   FbxManager * lSdkManager;
   FbxIOSettings * ios;
   FbxScene * lScene;
};

#endif // FBXCONVERT_H
