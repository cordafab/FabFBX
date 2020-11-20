#include "unpackerFBX.h"

#include <iostream>
#include <fstream>

UnpackerFBX::UnpackerFBX()
{
   sdkManager = nullptr;
   ioSettings = nullptr;
   fbxScene = nullptr;
}

bool UnpackerFBX::createUnpacker(std::string fbxPathFile)
{
   pathFileNoExt = fbxPathFile.substr(0, fbxPathFile.size()-4);
   std::size_t botDirPos = pathFileNoExt.find_last_of("/");
   filename = pathFileNoExt.substr(botDirPos+1, pathFileNoExt.length());

   sdkManager = FbxManager::Create();
   ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT );
   sdkManager->SetIOSettings(ioSettings);
   FbxImporter* lImporter = FbxImporter::Create(sdkManager, "");
   bool lImportStatus = lImporter->Initialize(fbxPathFile.c_str(), -1, sdkManager->GetIOSettings());

   if(!lImportStatus) return false; //execStatus==false exit

   // Create a new scene so it can be populated by the imported file.
   fbxScene = FbxScene::Create(sdkManager,"myScene");
   // Import the contents of the file into the scene.
   lImporter->Import(fbxScene);
   // The file has been imported; we can get rid of the importer.
   lImporter->Destroy();

   return true; //execStatus==true
}

bool UnpackerFBX::unpack(int flag, std::string characterName, std::string skeletonName)
{
   if(flag==RIG)
   {
      std::vector<double> v;
      std::vector<std::vector<int>> f;
      unpackCharacterGeometry(characterName, v, f);
      UnpackerFBX::saveOBJ(pathFileNoExt+".obj", v, f);

      std::vector<std::string> jointsNames;
      std::vector<std::vector<double>> jointsPositions;
      std::vector<int> fathers;
      std::map<std::string, unsigned long> jointIdByName;
      unpackSkeletonTopology(skeletonName, jointsNames, jointsPositions, fathers, jointIdByName);
      UnpackerFBX::saveSkeleton(pathFileNoExt+"_skel.txt", jointsNames, jointsPositions, fathers);

      Weights skeletonWeights;
      unpackSkeletonWeights(characterName, jointsNames, jointIdByName, skeletonWeights);
      UnpackerFBX::saveWeights(pathFileNoExt+"_skelWeights.txt", skeletonWeights);

      std::vector<double> keyframesTimes;
      std::vector<std::vector<std::vector<double>>> skelKeyframes;
      unpackSkeletonAnimation(skeletonName, jointsNames, jointIdByName, keyframesTimes, skelKeyframes);
      UnpackerFBX::saveAnimation(pathFileNoExt+"_sAnim.txt",keyframesTimes,skelKeyframes);
   }

   if(flag==ANIM)
   {
      std::vector<std::string> jointsNames;
      std::vector<std::vector<double>> jointsPositions;
      std::vector<int> fathers;
      std::map<std::string, unsigned long> jointIdByName;
      unpackSkeletonTopology(skeletonName, jointsNames, jointsPositions, fathers, jointIdByName);

      std::vector<double> keyframesTimes;
      std::vector<std::vector<std::vector<double>>> skelKeyframes;
      unpackSkeletonAnimation(skeletonName, jointsNames, jointIdByName, keyframesTimes, skelKeyframes);
      UnpackerFBX::saveAnimation(pathFileNoExt+"_sAnim.txt",keyframesTimes,skelKeyframes);
   }

   if(flag==STE)
   {
      //export mdl file
      std::ofstream fp;
      std::string filenameSTE = pathFileNoExt+".mdl";
      fp.open (filenameSTE.c_str());
      fp.precision(6);
      fp.setf( std::ios::fixed, std::ios::floatfield ); // floatfield set to fixed

      if(!fp)
      {
         std::cout << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : couldn't open mdl output file " << filenameSTE << std::endl;
         exit(-1);
      }
      else
      {
         std::cout << "Export Ste model file: " << filenameSTE << std::endl;
      }

      fp << "m " << filename << ".obj" << std::endl;
      fp << "s " << filename << ".skt" << std::endl;
      fp << "w " << filename << ".skw" << std::endl;
      fp << "a " << filename << ".ska" << std::endl;

      fp.close();

      //Export the files

      std::vector<double> v;
      std::vector<std::vector<int>> f;
      unpackCharacterGeometry(characterName, v, f);
      UnpackerFBX::saveOBJ(pathFileNoExt+".obj", v, f);

      std::vector<std::string> jointsNames;
      std::vector<std::vector<double>> jointsPositions;
      std::vector<int> fathers;
      std::map<std::string, unsigned long> jointIdByName;
      unpackSkeletonTopology(skeletonName, jointsNames, jointsPositions, fathers, jointIdByName);
      UnpackerFBX::saveSkeleton(pathFileNoExt+".skt", jointsNames, jointsPositions, fathers);

      Weights skeletonWeights;
      unpackSkeletonWeights(characterName, jointsNames, jointIdByName, skeletonWeights);
      UnpackerFBX::saveWeights(pathFileNoExt+".skw", skeletonWeights);

      std::vector<double> keyframesTimes;
      std::vector<std::vector<std::vector<double>>> skelKeyframes;
      unpackSkeletonAnimation(skeletonName, jointsNames, jointIdByName, keyframesTimes, skelKeyframes);
      UnpackerFBX::saveAnimation(pathFileNoExt+".ska",keyframesTimes,skelKeyframes);
   }

   return true;
}

bool UnpackerFBX::unpackCharacterGeometry(
      const std::string & characterNodeName,
      std::vector<double> & v,
      std::vector<std::vector<int>> & f
      )
{
   //Load Character
   FbxNode * lRootNode = nullptr;
   lRootNode = fbxScene->GetRootNode();

   if(lRootNode)
   {
      std::vector<FbxNode *> nodes;
      std::vector<FbxNode *> unorderedNodes;

      int totalVertices = 0;
      int totalPolygons = 0;

      for(int i = 0; i < lRootNode->GetChildCount(); i++)
      {
         FbxNode * lNode = lRootNode->GetChild(i);
         std::string nodeName(lNode->GetName());
         size_t found = nodeName.find(characterNodeName);

         if (found != std::string::npos)
         {
            unorderedNodes.push_back(lNode);
            totalVertices += lNode->GetMesh()->GetControlPointsCount();
            totalPolygons += lNode->GetMesh()->GetPolygonCount();
         }
      }

      if(unorderedNodes.size()>1)
      {
         nodes.resize(unorderedNodes.size());
         for(unsigned long i=0; i<unorderedNodes.size();++i)
         {
            std::string name(unorderedNodes[i]->GetName());
            std::string number = name.substr(9,name.size()-9);
            int n = std::stoi( number );
            nodes[n] = unorderedNodes[i];
         }
      }
      else if(unorderedNodes.size() ==1)
      {
         nodes.push_back(unorderedNodes[0]);
      }

      v.reserve(totalVertices*3);

      std::vector<int> newFace;

      int pointOffsets = 0;
      for(unsigned long i = 0; i<nodes.size(); ++i)
      {
         FbxMesh * lMesh = nodes[i]->GetMesh();
         int lControlPointsCount = lMesh->GetControlPointsCount();
         int lPolygonCount = lMesh->GetPolygonCount();
         FbxVector4 * lControlPoints = lMesh->GetControlPoints();
         FbxAMatrix t = nodes[i]->EvaluateGlobalTransform();

         for (int i = 0; i < lControlPointsCount; i++)
         {
            FbxVector4 controlPoint = t.MultT(lControlPoints[i]);
            v.push_back(controlPoint[0]);
            v.push_back(controlPoint[1]);
            v.push_back(controlPoint[2]);
         }

         for (int i = 0; i < lPolygonCount; i++)
         {
            //std::cout << "NEW FACE: ";
            int lPolygonSize = lMesh->GetPolygonSize(i);
            newFace.clear();
            for (int j = 0; j < lPolygonSize; j++)
            {
               int lControlPointIndex = lMesh->GetPolygonVertex(i, j);
               newFace.push_back(pointOffsets+lControlPointIndex+1);
               //std::cout << pointOffsets+lControlPointIndex << " ";
            }
            f.push_back(newFace);
            //std::cout << std::endl;
         }
         pointOffsets += lControlPointsCount;
      }
   }
   return true;
}

bool UnpackerFBX::unpackSkeletonTopology(
      const std::string & skeletonNodeName,
      std::vector<std::string> & jointsNames,
      std::vector<std::vector<double>> & jointsPositions,
      std::vector<int> & fathers,
      std::map<std::string, unsigned long> & jointIdByName
      )
{
   FbxNode * lRootNode = nullptr;
   //Read FBX Skeleton
   lRootNode = fbxScene->GetRootNode();
   if(lRootNode)
   {
      for(int i = 0; i < lRootNode->GetChildCount(); i++)
      {
         FbxNode * lNode = lRootNode->GetChild(i);
         if(strcmp(lNode->GetName(),skeletonNodeName.c_str())>=0)
         {
            UnpackerFBX::navigateSkeleton(jointsNames,
                                     jointsPositions,
                                     fathers,
                                     lNode,
                                     -1);
         }
      }
      //compute the jointName-jointId conversion map
      for(unsigned long i=0; i<jointsNames.size(); ++i)
      {
         std::string name = jointsNames[i];
         jointIdByName[name] = i;
      }
   }
   return true;
}

bool UnpackerFBX::unpackSkeletonWeights(
      const std::string & characterNodeName,
      const std::vector<std::string> & jointsNames,
      const std::map<std::string, unsigned long> & jointIdByName,
      Weights & skeletonWeights
      )
{
   FbxNode * lRootNode = nullptr;
   lRootNode = fbxScene->GetRootNode();

   if(lRootNode)
   {

      std::vector<FbxNode *> nodes;
      std::vector<FbxNode *> unorderedNodes;
      for(int i = 0; i < lRootNode->GetChildCount(); i++)
      {
         FbxNode * lNode = lRootNode->GetChild(i);
         std::string nodeName(lNode->GetName());
         size_t found = nodeName.find(characterNodeName);
         if (found != std::string::npos)
         {
            unorderedNodes.push_back(lNode);
         }
      }
      int totalVertices = 0;
      std::vector<int> vertexOffset(unorderedNodes.size());
      nodes.resize(unorderedNodes.size());
      if(unorderedNodes.size()>1)
      {
         for(unsigned long i=0; i<unorderedNodes.size();++i)
         {
            std::string name(unorderedNodes[i]->GetName());
            std::string number = name.substr(9,name.size()-9);
            int n = std::stoi( number );
            nodes[n] = unorderedNodes[i];
         }
      }
      else if(unorderedNodes.size() ==1)
      {
         nodes[0] = unorderedNodes[0];
      }
      for(unsigned long i=0; i<nodes.size();++i)
      {
         vertexOffset[i] = totalVertices;
         totalVertices += nodes[i]->GetMesh()->GetControlPointsCount();
      }
      if(jointsNames.size()>0)
      {
         unsigned long numNodes = jointsNames.size();
         skeletonWeights.create(totalVertices,numNodes);
         //end conversion map computation
         for(unsigned long nodeIndex = 0; nodeIndex<nodes.size(); ++nodeIndex)
         {
            FbxMesh* lMesh = (FbxMesh*) nodes[nodeIndex]->GetMesh();
            int lVertexCount = lMesh->GetControlPointsCount();
            //we need to get the number of clusters
            const int lSkinCount = lMesh->GetDeformerCount(FbxDeformer::eSkin);
            int totalClusterCount = 0;
            for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
            {
               totalClusterCount += ((FbxSkin *)(lMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
            }
            //if there are clusters that contain skeleton weights (1 cluster for 1 handle)
            if (totalClusterCount)
            {
               //control point = mesh vertex
               for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
               {
                  FbxSkin * lSkinDeformer = (FbxSkin *)lMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
                  int lClusterCount = lSkinDeformer->GetClusterCount();
                  for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
                  {
                     FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
                     FbxNode* linkedJoint = lCluster->GetLink();
                     if (!linkedJoint)
                        continue;
                     int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
                     for (int k = 0; k < lVertexIndexCount; ++k)
                     {
                        int lIndex = lCluster->GetControlPointIndices()[k];
                        if (lIndex >= lVertexCount)
                           continue;
                        double lWeight = lCluster->GetControlPointWeights()[k];
                        if (lWeight == 0.0)
                        {
                           continue;
                        }
                        std::string name = linkedJoint->GetName();
                        /*std::cout << name << ". ID: " << nodeIdByName.at(name)
                                     << ": " << "CP: " << lIndex
                                     << ". Offset: " << vertexOffset[nodeIndex]
                                        << ". Weight: " << lWeight << std::endl;*/
                        skeletonWeights.setWeight(jointIdByName.at(name), vertexOffset[nodeIndex]+lIndex, lWeight);
                     }
                  }
               }
            }
         }
      }
   }
   return true;
}

bool UnpackerFBX::unpackSkeletonAnimation(
      const std::string & skeletonNodeName,
      const std::vector<std::string> & jointsNames,
      const std::map<std::string, unsigned long> & jointIdByName,
      std::vector<double> & keyframesTimes,
      std::vector<std::vector<std::vector<double>>> & skelKeyframes) //This is sooo ugly
{
   double scaleFactor = 1.0;

   //compute the jointName-jointId conversion map
   FbxTime startTime, stopTime;
   FbxLongLong mAnimationLength;

   int i;
   for (i = 0; i < fbxScene->GetSrcObjectCount<FbxAnimStack>(); i++)
   {
      FbxAnimStack* lAnimStack = fbxScene->GetSrcObject<FbxAnimStack>(i);
      FbxString animStackName = lAnimStack->GetName();
      FbxTakeInfo* takeInfo = fbxScene->GetTakeInfo(animStackName);
      startTime = takeInfo->mLocalTimeSpan.GetStart();
      stopTime = takeInfo->mLocalTimeSpan.GetStop();
      mAnimationLength = startTime.GetFrameCount(FbxTime::eFrames30) - stopTime.GetFrameCount(FbxTime::eFrames30) + 1;
   }

   for (FbxLongLong i = startTime.GetFrameCount(FbxTime::eFrames30); i <= stopTime.GetFrameCount(FbxTime::eFrames30); ++i)
   {
      FbxTime currTime;
      currTime.SetFrame(i, FbxTime::eFrames30);
      double t = ((double)(currTime.GetMilliSeconds())) / 1000.0;
      std::vector<std::vector<double>> deformedKeyframes(jointsNames.size());
      FbxNode* lRootNode = fbxScene->GetRootNode();
      if(lRootNode)
      {
         for(int j = 0; j < lRootNode->GetChildCount(); j++)
         {
            FbxNode * lNode = lRootNode->GetChild(j);

            if(strcmp(lNode->GetName(),skeletonNodeName.c_str())>=0)
            {
               UnpackerFBX::getNodeKeyframe(lNode, currTime, deformedKeyframes, jointIdByName, scaleFactor);
            }
         }
      }
      skelKeyframes.push_back(deformedKeyframes);
      keyframesTimes.push_back(t);
   }

   return true;
}

bool UnpackerFBX::list()
{
   FbxNode * lRootNode = nullptr;

   lRootNode = fbxScene->GetRootNode();
   if(lRootNode)
   {
      for(int i = 0; i < lRootNode->GetChildCount(); i++)
      {
         FbxNode * lNode = lRootNode->GetChild(i);
         std::cout << lNode->GetName() << std::endl;
      }
   }
   return true;
}


//Skeleton
void UnpackerFBX::navigateSkeleton(std::vector<std::string> & names,
                              std::vector<std::vector<double>> & joints,
                              std::vector<int>         & fathers,
                              FbxNode                  * node,
                              int                        father)
{
   FbxAMatrix t0 = node->EvaluateGlobalTransform();
   FbxDouble3 lTranslation = t0.GetT();
   FbxDouble3 lRotation    = t0.GetR();
   FbxDouble3 lScaling     = t0.GetS();

   unsigned long nodeId = names.size();

   names.push_back(node->GetName());

   joints.push_back(fromFbxMatrixToVector(t0));

   //jointsPositions.push_back(lTranslation[0]);
   //jointsPositions.push_back(lTranslation[1]);
   //jointsPositions.push_back(lTranslation[2]);

   fathers.push_back(father);

   for(int i = 0; i < node->GetChildCount(); i++)
   {
      UnpackerFBX::navigateSkeleton(names,
                               joints,
                               fathers,
                               node->GetChild(i),
                               nodeId);
   }
}

void UnpackerFBX::saveSkeleton (const std::string            & filename,
                           const std::vector<std::string>    & names,
                           const std::vector<std::vector<double>>         & joints,
                           const std::vector<int>            & fathers)
{
   std::ofstream fp;
   fp.open (filename.c_str());
   fp.precision(6);
   fp.setf( std::ios::fixed, std::ios::floatfield ); // floatfield set to fixed

   if(!fp)
   {
      std::cout << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : saveSkeleton() : couldn't open output file " << filename << std::endl;
      exit(-1);
   }
   else
   {
      std::cout << "Export skeleton: " << filename << std::endl;
   }

   for( unsigned long i = 0; i < names.size(); ++i )
   {
      std::vector<double> kf = joints[i];
      fp << "j "
         << i << " "
         << names[i] << " "
         << fathers[i] << " "
         << kf[0] << " "  << kf[1] << " "  << kf[2]  << " " << kf[3]  << " "
         << kf[4] << " "  << kf[5] << " "  << kf[6]  << " " << kf[7]  << " "
         << kf[8] << " "  << kf[9] << " "  << kf[10] << " " << kf[11] << " "
         << kf[12] << " " << kf[13] << " " << kf[14] << " " << kf[15] <<
            std::endl;
   }

   fp.close();
}


//Weights
void UnpackerFBX::saveWeights(const std::string & filename,
                         const Weights     & weights)
{
   std::ofstream fp;
   fp.open (filename);
   fp.precision(6);
   fp.setf( std::ios::fixed, std::ios::floatfield ); // floatfield set to fixed

   if(!fp)
   {
      std::cout << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : saveWeights() : couldn't open output file " << filename << std::endl;
      exit(-1);
   }
   else
   {
      std::cout << "Export weights: " << filename << std::endl;
   }

   for( int i = 0; i < weights.getNumberOfVertices(); i++ )
   {

      for( int j = 0; j < weights.getNumberOfHandles(); j++ )
      {
         double w = weights.getWeight(j,i);
         if(w != 0.0)
         {
            fp << j << " " << i << " " << w << std::endl;
         }

      }
   }

   fp.close();
}


//Animation
void UnpackerFBX::saveAnimation(const std::string & filename,
                           const std::vector<double> & t,
                           std::vector<std::vector<std::vector<double>>> & skelKeyframes)
{
   std::ofstream fp;
   fp.open (filename);
   fp.precision(6);
   fp.setf( std::ios::fixed, std::ios::floatfield ); // floatfield set to fixed

   if(!fp)
   {
      std::cout << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : saveSkelAnimation() : couldn't open output file " << filename << std::endl;
      exit(-1);
   }
   else
   {
      std::cout << "Export Skeleton Animation: " << filename << std::endl;
   }

   //fp << "t " << "rt" << std::endl;   //animation exported rt curve (rotation & translation x, y, z components) OR M matrix (local transformation)

   for( unsigned long  i = 0; i < t.size(); ++i )
   {
      fp << "k " << t[i] << std::endl;   //keyframe
   }

   for( unsigned long i = 0; i < t.size(); ++i )   //for each keyframe
   {

      for( unsigned long j = 0; j < skelKeyframes[i].size(); ++j )   //for each skel joint
      {

         std::vector<double> kf = skelKeyframes[i][j];

         fp << "s "  //skel
            << i << " "
            << kf[0] << " "  << kf[1] << " "  << kf[2]  << " " << kf[3]  << " "
            << kf[4] << " "  << kf[5] << " "  << kf[6]  << " " << kf[7]  << " "
            << kf[8] << " "  << kf[9] << " "  << kf[10] << " " << kf[11] << " "
            << kf[12] << " " << kf[13] << " " << kf[14] << " " << kf[15] <<
               std::endl;
         /*fp << "s "  //skel
            << i << " "
            << kf[0] << " " << kf[1] << " " << kf[2] << " "
            << kf[3] << " " << kf[4] << " " << kf[5] <<
               std::endl;*/

      }
   }
   fp.close();
}

void UnpackerFBX::getNodeKeyframe(      FbxNode                              * node,
                                   const FbxTime                        & t,
                                   std::vector<std::vector<double>>     & deformedKeyframes,
                                   const std::map<std::string, unsigned long> & nodeIdByName,
                                   double   scaleFactor)
{
   unsigned long i = nodeIdByName.at(node->GetName());
   /*FbxAMatrix defMat  = node->EvaluateLocalTransform(t);
   std::vector<double> keyframe(6); //rx, ry, rz, tx, ty, tz

   FbxAnimCurveNode* lAnimCurveNodeT = node->LclTranslation.GetCurveNode();
   if(lAnimCurveNodeT)
   {
      FbxAnimCurve* lAnimCurveNodeTx = lAnimCurveNodeT->GetCurve(0);
      FbxAnimCurve* lAnimCurveNodeTy = lAnimCurveNodeT->GetCurve(1);
      FbxAnimCurve* lAnimCurveNodeTz = lAnimCurveNodeT->GetCurve(2);
      if(lAnimCurveNodeTx) { keyframe[3] = lAnimCurveNodeTx->Evaluate(t); keyframe[3] /=  scaleFactor;}
      if(lAnimCurveNodeTy) { keyframe[4] = lAnimCurveNodeTy->Evaluate(t); keyframe[4] /=  scaleFactor;}
      if(lAnimCurveNodeTz) { keyframe[5] = lAnimCurveNodeTz->Evaluate(t); keyframe[5] /=  scaleFactor;}
   }

   FbxAnimCurveNode* lAnimCurveNodeR = node->LclRotation.GetCurveNode();
   if(lAnimCurveNodeR)
   {
      FbxAnimCurve* lAnimCurveNodeRx = lAnimCurveNodeR->GetCurve(0);
      FbxAnimCurve* lAnimCurveNodeRy = lAnimCurveNodeR->GetCurve(1);
      FbxAnimCurve* lAnimCurveNodeRz = lAnimCurveNodeR->GetCurve(2);
      if(lAnimCurveNodeRx) { keyframe[0] = lAnimCurveNodeRx->Evaluate(t); }
      if(lAnimCurveNodeRy) { keyframe[1] = lAnimCurveNodeRy->Evaluate(t); }
      if(lAnimCurveNodeRz) { keyframe[2] = lAnimCurveNodeRz->Evaluate(t); }
   }*/

   FbxAMatrix defMat  = node->EvaluateGlobalTransform(t);

   std::vector<double> keyframe = fromFbxMatrixToVector(defMat);

   deformedKeyframes[i] = keyframe;

   for(int lModelCount = 0; lModelCount < node->GetChildCount(); lModelCount++)
   {
      UnpackerFBX::getNodeKeyframe(node->GetChild(lModelCount), t, deformedKeyframes, nodeIdByName, scaleFactor);
   }
}

std::vector<double> UnpackerFBX::fromFbxMatrixToVector(const FbxAMatrix & matrix)
{
   std::vector<double> convertedTransform(16);
   for(unsigned int i=0;i<4;++i)
   {
      for(unsigned int j=0;j<4;++j)
      {
         convertedTransform[j*4+i] = matrix.Get(j,i);  //column major //la get di fbx inverte i valori
      }
   }
   return convertedTransform;
}

void UnpackerFBX::saveOBJ(const std::string         & filename ,
                     const std::vector<double> & v        ,
                     const std::vector<std::vector<int>>    & f        )
{
   std::ofstream fp;
   fp.open (filename);
   fp.precision(6);
   fp.setf( std::ios::fixed, std:: ios::floatfield ); // floatfield set to fixed

   if(!fp)
   {
      std::cout << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : saveTrimeshObj() : couldn't open output file " << filename << std::endl;
      exit(-1);
   }
   else
   {
      std::cout << "Export character: " << filename << std::endl;
   }

   for(int i=0; i<(int)v.size(); i+=3)
   {
      fp << "v " << v[i] << " " << v[i+1] << " " << v[i+2] << std::endl;
   }

   for(int i=0; i<(int)f.size(); i++)
   {
      fp << "f ";
      for(int j=0; j<(int)f[i].size(); j++)
      {
         fp << f[i][j] << " ";
      }
      fp << std::endl;
   }

   fp.close();
}
