#include "fbxConvert.h"

#include <iostream>
#include <fstream>

#include <fbxsdk.h>

void convert(std::string fbxPathFile)
{
   std::string skeletonName  = "Skeleton";
   std::string characterName = "Character";

   std::string pathFileNoExt = fbxPathFile.substr(0, fbxPathFile.size()-4);

   FbxManager * lSdkManager = nullptr;
   FbxIOSettings * ios = nullptr;
   FbxScene * lScene = nullptr;
   bool error;




   //init
   lSdkManager = FbxManager::Create();
   ios = FbxIOSettings::Create(lSdkManager, IOSROOT );
   lSdkManager->SetIOSettings(ios);
   FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
   bool lImportStatus = lImporter->Initialize(fbxPathFile.c_str(), -1, lSdkManager->GetIOSettings());
   if(!lImportStatus)
   {
      error = true;
   }
   else
   {
      // Create a new scene so it can be populated by the imported file.
      lScene = FbxScene::Create(lSdkManager,"myScene");
      // Import the contents of the file into the scene.
      lImporter->Import(lScene);
      // The file has been imported; we can get rid of the importer.
      lImporter->Destroy();
   }
   FbxNode * lRootNode;
   //end init

   //Load Character
   lRootNode = lScene->GetRootNode();

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
         size_t found = nodeName.find(characterName);

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

      std::vector<double> v;
      v.reserve(totalVertices*3);
      std::vector<std::vector<int>> f;
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
      //Export Character to file
      saveTrimeshObj(pathFileNoExt+".obj", v, f);
   }
   //End Export Character



   //Load Skeleton
   std::vector<std::string> jointNames;
   std::vector<double>      jointsPositions;
   std::vector<int>         fathers;

   //Read FBX Skeleton
   lRootNode = lScene->GetRootNode();
   if(lRootNode)
   {
      for(int i = 0; i < lRootNode->GetChildCount(); i++)
      {
         FbxNode * lNode = lRootNode->GetChild(i);
         if(strcmp(lNode->GetName(),skeletonName.c_str())>=0)
         {
            navigateSkeleton(jointNames,
                             jointsPositions,
                             fathers,
                             lNode,
                             -1);
         }
      }
   }

   //Export Skeleton to file
   saveSkeleton(pathFileNoExt+"_skel.txt",
                jointNames,
                jointsPositions,
                fathers);


   //end export Skeleton




   //export Skeleton Weights
   Weights skeletonWeights;

   std::map<std::string, unsigned long> nodeIdByName;

   //Read FBX Skeleton Weights
   lRootNode = lScene->GetRootNode();

   if(lRootNode)
   {

      std::vector<FbxNode *> nodes;
      std::vector<FbxNode *> unorderedNodes;
      for(int i = 0; i < lRootNode->GetChildCount(); i++)
      {
         FbxNode * lNode = lRootNode->GetChild(i);
         std::string nodeName(lNode->GetName());
         size_t found = nodeName.find(characterName);
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
      if(jointNames.size()>0)
      {
         unsigned long numNodes = jointNames.size();
         skeletonWeights.create(totalVertices,numNodes);

         //compute the jointName-jointId conversion map
         for(unsigned long i=0; i<numNodes; ++i)
         {
            std::string name = jointNames[i];
            nodeIdByName[name] = i;
         }
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
                        // Sometimes, the mesh can have less points than at the time of the skinning
                        // because a smooth operator was active when skinning but has been deactivated during export.
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
                        skeletonWeights.setWeight(nodeIdByName.at(name), vertexOffset[nodeIndex]+lIndex, lWeight);
                     }
                  }
               }
            }
         }
      }
   }

   //Export to file
   saveWeights(pathFileNoExt+"_skelWeights.txt", skeletonWeights);
   //end export Skeleton Weights




   //export Skeleton Animation
   std::vector<double>                           keyframesTimes;
   std::vector<std::vector<std::vector<double>>> skelKeyframes; //This is sooo ugly

   //Read the animation
   //compute the jointName-jointId conversion map
   FbxTime startTime, stopTime;
   FbxLongLong mAnimationLength;
   FbxAnimLayer* animLayer = nullptr;

   int i;
   for (i = 0; i < lScene->GetSrcObjectCount<FbxAnimStack>(); i++)
   {
      FbxAnimStack* lAnimStack = lScene->GetSrcObject<FbxAnimStack>(i);
      FbxString animStackName = lAnimStack->GetName();
      FbxTakeInfo* takeInfo = lScene->GetTakeInfo(animStackName);
      startTime = takeInfo->mLocalTimeSpan.GetStart();
      stopTime = takeInfo->mLocalTimeSpan.GetStop();
      mAnimationLength = startTime.GetFrameCount(FbxTime::eFrames24) - stopTime.GetFrameCount(FbxTime::eFrames24) + 1;
      animLayer = lAnimStack->GetMember<FbxAnimLayer>(0);
   }
   {
      for (FbxLongLong i = startTime.GetFrameCount(FbxTime::eFrames24); i <= stopTime.GetFrameCount(FbxTime::eFrames24); ++i)
      {
         FbxTime currTime;
         currTime.SetFrame(i, FbxTime::eFrames24);
         double t = ((double)(currTime.GetMilliSeconds())) / 1000.0;
         std::vector<std::vector<double>> deformedKeyframes(jointNames.size());
         FbxNode* lRootNode = lScene->GetRootNode();
         if(lRootNode)
         {
            for(int j = 0; j < lRootNode->GetChildCount(); j++)
            {
               FbxNode * lNode = lRootNode->GetChild(j);

               if(strcmp(lNode->GetName(),skeletonName.c_str())>=0)
               {
                  getNodeKeyframe(lNode, currTime, deformedKeyframes, nodeIdByName, animLayer);
               }
            }
         }

         //animator->addKeyframe(t,cageKeyframes,restKeyframes,deformedKeyframes);
         //asyncAnimator->addSkelKeyframe(t,deformedKeyframes);
         skelKeyframes.push_back(deformedKeyframes);
         keyframesTimes.push_back(t);
      }
   }

   //Export to file
   saveSkelAnimation(pathFileNoExt+"_sAnim.txt",keyframesTimes,skelKeyframes);
   //end export Skeleton Animation
}




//Skeleton
void navigateSkeleton(std::vector<std::string> & names,
                      std::vector<double>      & jointsPositions,
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

   jointsPositions.push_back(lTranslation[0]);
   jointsPositions.push_back(lTranslation[1]);
   jointsPositions.push_back(lTranslation[2]);

   fathers.push_back(father);

   for(int i = 0; i < node->GetChildCount(); i++)
   {
      navigateSkeleton(names,
                       jointsPositions,
                       fathers,
                       node->GetChild(i),
                       nodeId);
   }
}

void saveSkeleton (const std::string                 & filename,
                   const std::vector<std::string>    & names,
                   const std::vector<double>         & jointsPositions,
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
      fp << "j "
         << i << " "
         << names[i] << " "
         << fathers[i] << " "
         << jointsPositions[i*3+0] << " "
         << jointsPositions[i*3+1] << " "
         << jointsPositions[i*3+2]
         << std::endl;
   }

   fp.close();
}


//Weights
void saveWeights(const std::string & filename,
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
void saveSkelAnimation(const std::string                                   & filename,
                       const std::vector<double>                           & t,
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

   fp << "t " << "c" << std::endl;   //animation exported c (r+t separated values) OR m (local matrix)

   for( unsigned long  i = 0; i < t.size(); ++i )
   {
      fp << "k " << t[i] << std::endl;   //keyframe
   }

   for( unsigned long i = 0; i < t.size(); ++i )
   {

      for( unsigned long j = 0; j < skelKeyframes[i].size(); ++j )
      {

         std::vector<double> kf = skelKeyframes[i][j];

         /*fp << "s "  //skel
            << i << " "
            << kf[0] << " "  << kf[1] << " "  << kf[2]  << " " << kf[3]  << " "
            << kf[4] << " "  << kf[5] << " "  << kf[6]  << " " << kf[7]  << " "
            << kf[8] << " "  << kf[9] << " "  << kf[10] << " " << kf[11] << " "
            << kf[12] << " " << kf[13] << " " << kf[14] << " " << kf[15] <<
               std::endl;*/
         fp << "s "  //skel
            << i << " "
            << kf[0] << " " << kf[1] << " " << kf[2] << " "
            << kf[3] << " " << kf[4] << " " << kf[5] <<
               std::endl;

      }
   }
   fp.close();
}

void getNodeKeyframe(      FbxNode                              * node,
                           const FbxTime                        & t,
                           std::vector<std::vector<double>>     & deformedKeyframes,
                           const std::map<std::string, unsigned long> & nodeIdByName,
                           FbxAnimLayer* animLayer)
{
   unsigned long i = nodeIdByName.at(node->GetName());

   FbxAMatrix defMat  = node->EvaluateLocalTransform(t);

   std::vector<double> keyframe(6); //rx, ry, rz, tx, ty, tz


   if(i==0)
   {
      FbxAnimCurveNode* lAnimCurveNodeT = node->LclTranslation.GetCurveNode();
      FbxAnimCurve* lAnimCurveNodeTx = lAnimCurveNodeT->GetCurve(0);
      FbxAnimCurve* lAnimCurveNodeTy = lAnimCurveNodeT->GetCurve(1);
      FbxAnimCurve* lAnimCurveNodeTz = lAnimCurveNodeT->GetCurve(2);
      keyframe[3] = lAnimCurveNodeTx->EvaluateIndex(t.GetFrameCount());
      keyframe[4] = lAnimCurveNodeTy->EvaluateIndex(t.GetFrameCount());
      keyframe[5] = lAnimCurveNodeTz->EvaluateIndex(t.GetFrameCount());
  }

   FbxAnimCurveNode* lAnimCurveNodeR = node->LclRotation.GetCurveNode();
   FbxAnimCurve* lAnimCurveNodeRx = lAnimCurveNodeR->GetCurve(0);
   FbxAnimCurve* lAnimCurveNodeRy = lAnimCurveNodeR->GetCurve(1);
   FbxAnimCurve* lAnimCurveNodeRz = lAnimCurveNodeR->GetCurve(2);
   keyframe[0] = lAnimCurveNodeRx->EvaluateIndex(t.GetFrameCount());
   keyframe[1] = lAnimCurveNodeRy->EvaluateIndex(t.GetFrameCount());
   keyframe[2] = lAnimCurveNodeRz->EvaluateIndex(t.GetFrameCount());

   deformedKeyframes[i] = keyframe;

   for(int lModelCount = 0; lModelCount < node->GetChildCount(); lModelCount++)
   {
      getNodeKeyframe(node->GetChild(lModelCount), t, deformedKeyframes, nodeIdByName, animLayer);
   }
}

std::vector<double> fromFbxMatrixToVector(const FbxAMatrix & matrix)
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

void saveTrimeshObj(const std::string         & filename ,
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
