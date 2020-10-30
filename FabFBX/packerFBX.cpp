#include "packerFBX.h"

#include <iostream>
#include <fstream>
#include <sstream>

#define PACK 10

PackerFBX::PackerFBX()
{
   sdkManager = nullptr;
   ioSettings = nullptr;
   fbxScene = nullptr;
   fbxSdkResult = true;

   mesh = nullptr;
   skeletonRoot = nullptr;
}

bool PackerFBX::createPacker(std::string rigPathFile)
{
   fullPathFile = rigPathFile;
   pathFileNoExt = rigPathFile.substr(0, rigPathFile.size()-4);
   std::size_t botDirPos = pathFileNoExt.find_last_of("/");
   filename = pathFileNoExt.substr(botDirPos+1, pathFileNoExt.length());
   pathNoFile = rigPathFile.substr(0, botDirPos+1);

   // Prepare the FBX SDK.
   //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
   sdkManager = FbxManager::Create();
   if( !sdkManager )
   {
      FBXSDK_printf("Error: Unable to create FBX Manager!\n");
      exit(1);
   }
   else FBXSDK_printf("Autodesk FBX SDK version %s\n", sdkManager->GetVersion());

   //Create an IOSettings object. This object holds all import/export settings.
   FbxIOSettings* tIos = FbxIOSettings::Create(sdkManager, IOSROOT);
   sdkManager->SetIOSettings(tIos);
   ioSettings = sdkManager->GetIOSettings();

   //Load plugins from the executable directory (optional)
   FbxString lPath = FbxGetApplicationDirectory();
   sdkManager->LoadPluginsDirectory(lPath.Buffer());

   //Create an FBX scene. This object holds most objects imported/exported from/to files.
   fbxScene = FbxScene::Create(sdkManager, "My Scene");
   if( !fbxScene )
   {
      FBXSDK_printf("Error: Unable to create FBX scene!\n");
      exit(1);
   }

   // create scene info
   FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(sdkManager,"SceneInfo");
   sceneInfo->mTitle = filename.c_str();

   // we need to add the sceneInfo before calling AddThumbNailToScene because
   // that function is asking the scene for the sceneInfo.
   fbxScene->SetSceneInfo(sceneInfo);

   return true; //execStatus==true
}

bool PackerFBX::pack()
{
   std::string charFilename;
   std::string skelFilename;

   loadRigFile(fullPathFile,charFilename, skelFilename);
   loadOBJ(pathNoFile+charFilename, char_v, char_f);
   loadSkeleton(pathNoFile+skelFilename, joint_names, joint_fathers, joint_pos);

   mesh = packCharacterGeometry();
   skeletonRoot = packSkeletonTopology();

   // Build the node tree.
   FbxNode* lRootNode = fbxScene->GetRootNode();
   lRootNode->AddChild(mesh);
   lRootNode->AddChild(skeletonRoot);

   // Store poses
   //createWeights();
   //StoreBindPose(lScene, lPatch);
   //StoreRestPose(lScene, lSkeletonRoot);

   // Animation
   //AnimateSkeleton(lScene, lSkeletonRoot);

   saveFBX();

   //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
   if( sdkManager ) sdkManager->Destroy();
   if( fbxSdkResult )
   {
      FBXSDK_printf("Program Success!\n");
      return true; //execStatus==true
   }
   return false; //execStatus==false
}

void PackerFBX::saveFBX()
{
   // Save the scene.
   int pFileFormat=-1; //Fbx Binary format
   bool pEmbedMedia=false;

   int lMajor, lMinor, lRevision;
   bool lStatus = true;

   // Create an exporter.
   FbxExporter* lExporter = FbxExporter::Create(sdkManager, "");

   // Set the export states. By default, the export states are always set to
   // true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below
   // shows how to change these states.
   ioSettings->SetBoolProp(EXP_FBX_MATERIAL,        true);
   ioSettings->SetBoolProp(EXP_FBX_TEXTURE,         true);
   ioSettings->SetBoolProp(EXP_FBX_EMBEDDED,        pEmbedMedia);
   ioSettings->SetBoolProp(EXP_FBX_SHAPE,           true);
   ioSettings->SetBoolProp(EXP_FBX_GOBO,            true);
   ioSettings->SetBoolProp(EXP_FBX_ANIMATION,       true);
   ioSettings->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

   // Initialize the exporter by providing a filename.
   if(lExporter->Initialize((pathFileNoExt+".fbx").c_str(), pFileFormat, sdkManager->GetIOSettings()) == false)
   {
      FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
      FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
      fbxSdkResult = false;
   }
   else  {
      FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
      FBXSDK_printf("FBX file format version %d.%d.%d\n\n", lMajor, lMinor, lRevision);
      // Export the scene.
      lStatus = lExporter->Export(fbxScene);
      // Destroy the exporter.
      lExporter->Destroy();
      if(fbxSdkResult == false)
      {
         FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
      }
   }
}

FbxNode* PackerFBX::packCharacterGeometry()
{
   FbxMesh* lMesh = FbxMesh::Create(fbxScene,"Character");
   lMesh->InitControlPoints(char_v.size());
   //FbxVector4* lControlPoints = lMesh->GetControlPoints();
   for(unsigned long i=0; i<char_v.size(); ++i)
   {
      std::vector<float> v = char_v[i];
      FbxVector4 controlPoint(v[0],v[1],v[2], 0.0);
      lMesh->SetControlPointAt(controlPoint,i);
   }

   for(unsigned long i=0; i<char_f.size(); ++i)
   {
      lMesh->BeginPolygon();
      for(unsigned long j=0; j<char_f[i].size(); ++j)
      {
         lMesh->AddPolygon(char_f[i][j]-1);
      }
      lMesh->EndPolygon();
   }

   FbxNode* lNode = FbxNode::Create(fbxScene,"Character");
   lNode->SetNodeAttribute(lMesh);
   fbxScene->AddNode(lNode);

   return lNode;
}

FbxNode* PackerFBX::packSkeletonTopology()
{
   std::string name = "Skeleton";
   FbxNode* rootNode;

   fbx_joints.resize(joint_names.size());

   for(unsigned long i=0; i<joint_names.size(); ++i)
   {
      FbxString nodeName(joint_names[i].c_str());
      FbxSkeleton* joint = FbxSkeleton::Create(fbxScene,nodeName);
      if(joint_fathers[i]==-1)   joint->SetSkeletonType(FbxSkeleton::eRoot);
      else                       joint->SetSkeletonType(FbxSkeleton::eLimbNode);

      joint->Size.Set(1.0);
      fbx_joints[i] = FbxNode::Create(fbxScene,nodeName.Buffer());
      fbx_joints[i]->SetNodeAttribute(joint);

      if(joint_fathers[i]!=-1)
      {
         double localTx =  joint_pos[i][0] - joint_pos[joint_fathers[i]][0];
         double localTy =  joint_pos[i][1] - joint_pos[joint_fathers[i]][1];
         double localTz =  joint_pos[i][2] - joint_pos[joint_fathers[i]][2];
         fbx_joints[i]->LclTranslation.Set(FbxVector4(localTx,localTy,localTz));
      }
      else
      {
         fbx_joints[i]->LclTranslation.Set(FbxVector4(joint_pos[i][0],joint_pos[i][1],joint_pos[i][2]));
      }
   }

   for(unsigned long i=0; i<joint_fathers.size(); ++i)
   {
      if(joint_fathers[i]==-1) rootNode=fbx_joints[i];
      else fbx_joints[joint_fathers[i]]->AddChild(fbx_joints[i]);
   }

   fbxScene->AddNode(rootNode);

   return rootNode;
}

void PackerFBX::packSkeletonWeights()
{
   /*
   int i, j;
   FbxAMatrix lXMatrix;

   FbxNode* lRoot = skeletonRoot;
   FbxNode* lLimbNode1 = skeletonRoot->GetChild(0);
   FbxNode* lLimbNode2 = lLimbNode1->GetChild(0);

   // Bottom section of cylinder is clustered to skeleton root.
   FbxCluster *lClusterToRoot = FbxCluster::Create(lScene,"");
   lClusterToRoot->SetLink(lRoot);
   lClusterToRoot->SetLinkMode(FbxCluster::eTotalOne);
   for(i=0; i<4; ++i)
      for(j=0; j<4; ++j)
         lClusterToRoot->AddControlPointIndex(4*i + j, 1.0 - 0.25*i);

   // Center section of cylinder is clustered to skeleton limb node.
   FbxCluster* lClusterToLimbNode1 = FbxCluster::Create(lScene, "");
   lClusterToLimbNode1->SetLink(lLimbNode1);
   lClusterToLimbNode1->SetLinkMode(FbxCluster::eTotalOne);

   for (i =1; i<6; ++i)
      for (j=0; j<4; ++j)
         lClusterToLimbNode1->AddControlPointIndex(4*i + j, (i == 1 || i == 5 ? 0.25 : 0.50));


   // Top section of cylinder is clustered to skeleton limb.

   FbxCluster * lClusterToLimbNode2 = FbxCluster::Create(lScene,"");
   lClusterToLimbNode2->SetLink(lLimbNode2);
   lClusterToLimbNode2->SetLinkMode(FbxCluster::eTotalOne);

   for (i=3; i<7; ++i)
      for (j=0; j<4; ++j)
         lClusterToLimbNode2->AddControlPointIndex(4*i + j, 0.25*(i - 2));

   // Now we have the Patch and the skeleton correctly positioned,
   // set the Transform and TransformLink matrix accordingly.
   FbxScene* lScene = mesh->GetScene();
   if( lScene ) lXMatrix = mesh->EvaluateGlobalTransform();

   lClusterToRoot->SetTransformMatrix(lXMatrix);
   lClusterToLimbNode1->SetTransformMatrix(lXMatrix);
   lClusterToLimbNode2->SetTransformMatrix(lXMatrix);



   if( lScene ) lXMatrix = lRoot->EvaluateGlobalTransform();
   lClusterToRoot->SetTransformLinkMatrix(lXMatrix);


   if( lScene ) lXMatrix = lLimbNode1->EvaluateGlobalTransform();
   lClusterToLimbNode1->SetTransformLinkMatrix(lXMatrix);


   if( lScene ) lXMatrix = lLimbNode2->EvaluateGlobalTransform();
   lClusterToLimbNode2->SetTransformLinkMatrix(lXMatrix);


   // Add the clusters to the patch by creating a skin and adding those clusters to that skin.
   // After add that skin.

   FbxGeometry* lPatchAttribute = (FbxGeometry*) mesh->GetNodeAttribute();
   FbxSkin* lSkin = FbxSkin::Create(lScene, "");
   lSkin->AddCluster(lClusterToRoot);
   lSkin->AddCluster(lClusterToLimbNode1);
   lSkin->AddCluster(lClusterToLimbNode2);
   lPatchAttribute->AddDeformer(lSkin);
   */
}

void PackerFBX::packSkeletonAnimation()
{

}

void PackerFBX::loadRigFile(std::string filename,
                            std::string & charFilename,
                            std::string & skelFilename)
{
   std::ifstream file(filename);

   if (!file.is_open())
   {
      std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : loadOBJ() : couldn't open input file " << filename << std::endl;
      exit(-1);
   }

   std::string line;
   while (std::getline(file, line))
   {
      std::istringstream iss(line);

      std::vector<std::string> lineData{
         std::istream_iterator<std::string>(iss), {}
      };

      if (lineData[0].compare("m")==0)
      {
         charFilename = lineData[1];
      }

      if (lineData[0].compare("s")==0)
      {
         skelFilename = lineData[1];
      }
   }

   file.close();
}

void PackerFBX::loadOBJ(std::string filename,
                        std::vector<std::vector<float>> & v,
                        std::vector<std::vector<int>> & f)
{
   std::ifstream file(filename);

   if (!file.is_open())
   {
      std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : loadOBJ() : couldn't open input file " << filename << std::endl;
      exit(-1);
   }

   std::string line;
   while (std::getline(file, line))
   {
      std::istringstream iss(line);

      std::vector<std::string> lineData{
         std::istream_iterator<std::string>(iss), {}
      };

      for(auto s:lineData)
      {
         std::cout << s << " ";
      }
      std::cout << std::endl;
      if(lineData.size()>0)
      {
         if (lineData[0].compare("v")==0)
         {
            std::vector<float> coords;
            coords.push_back(std::stof(lineData[1]));
            coords.push_back(std::stof(lineData[2]));
            coords.push_back(std::stof(lineData[3]));
            v.push_back(coords);

            //std::cout << "v " << coords[0] << " " << coords[1] << " " << coords[2] << std::endl;
         }

         if (lineData[0].compare("f")==0)
         {
            std::vector<int> face;

            if(lineData.size()==4)
            {
               face.push_back(std::stoi(lineData[1]));
               face.push_back(std::stoi(lineData[2]));
               face.push_back(std::stoi(lineData[3]));


               //std::cout << "f " << face[0] << " " << face[1] << " " << face[2] << std::endl;

            } else
               if(lineData.size()==5)
               {
                  face.push_back(std::stoi(lineData[1]));
                  face.push_back(std::stoi(lineData[2]));
                  face.push_back(std::stoi(lineData[3]));
                  face.push_back(std::stoi(lineData[4]));

                  //std::cout << "f " << face[0] << " " << face[1] << " " << face[2] << " " << face[3] << std::endl;
               }
            f.push_back(face);
         }
      }
   }

   file.close();
}

void PackerFBX::loadSkeleton(std::string filename,
                                 std::vector<std::string> & names,
                                 std::vector<int> & fathers,
                                 std::vector<std::vector<float>> & pos
                                 )
{
   std::ifstream file(filename);

   if (!file.is_open())
   {
      std::cerr << "ERROR : " << __FILE__ << ", line " << __LINE__ << " : loadOBJ() : couldn't open input file " << filename << std::endl;
      exit(-1);
   }

   std::string line;
   while (std::getline(file, line))
   {
      std::istringstream iss(line);

      std::vector<std::string> lineData{
         std::istream_iterator<std::string>(iss), {}
      };

      if (lineData[0].compare("j")==0)
      {
         names.push_back(lineData[2]);
         fathers.push_back(std::stoi(lineData[3]));

         std::vector<float> coords;
         coords.push_back(std::stof(lineData[4]));
         coords.push_back(std::stof(lineData[5]));
         coords.push_back(std::stof(lineData[6]));
         pos.push_back(coords);

         //std::cout << lineData[2] << " " << lineData[3] << " " << lineData[4] << " " << lineData[5] << " " << lineData[6] << std::endl;
      }

   }

   file.close();
}
