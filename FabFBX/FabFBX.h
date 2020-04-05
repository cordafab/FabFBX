#ifndef FBXCONVERT_H
#define FBXCONVERT_H

#include <string>
#include <vector>
#include <map>

#include "fbxsdk.h"
#include "fbxsdk/fileio/fbxiosettings.h"

#include "weights.h"

class FabFBX{

public:
   FabFBX();

   bool init(std::string fbxPathFile);

   bool convert();

   bool exportCharacterGeometry(
         const std::string & characterNodeName,
         std::vector<double> & v,
         std::vector<std::vector<int>> & f
         );
   bool exportSkeletonTopology(
         const std::string & skeletonNodeName,
         std::vector<std::string> & jointsNames,
         std::vector<double> & jointsPositions,
         std::vector<int> & fathers,
         std::map<std::string, unsigned long> & jointIdByName
         );
   bool exportSkeletonWeights(
         const std::string & characterNodeName,
         const std::vector<std::string> & jointsNames,
         const std::map<std::string, unsigned long> & jointIdByName,
         Weights & skeletonWeights
         );
   bool exportSkeletonAnimation(
         const std::string & skeletonNodeName,
         const std::vector<std::string> & jointsNames,
         const std::map<std::string, unsigned long> & jointIdByName,
         std::vector<double> & keyframesTimes,
         std::vector<std::vector<std::vector<double>>> & skelKeyframes //THIS IS UGLY
         );

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
         const std::vector<std::vector<int> > &f
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

   FbxManager * lSdkManager;
   FbxIOSettings * ios;
   FbxScene * lScene;
};

#endif // FBXCONVERT_H
