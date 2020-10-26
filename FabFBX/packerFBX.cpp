#include "packerFBX.h"

#include <iostream>
#include <fstream>

#define PACK 10

PackerFBX::PackerFBX()
{
   lSdkManager = nullptr;
   ios = nullptr;
   lScene = nullptr;
   lResult = true;

   mesh = nullptr;
   skeletonRoot = nullptr;
}

bool PackerFBX::createPacker(std::string rigPathFile)
{
   pathFileNoExt = rigPathFile.substr(0, rigPathFile.size()-4);
   std::size_t botDirPos = pathFileNoExt.find_last_of("/");
   filename = pathFileNoExt.substr(botDirPos+1, pathFileNoExt.length());

   // Prepare the FBX SDK.
   //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
   lSdkManager = FbxManager::Create();
   if( !lSdkManager )
   {
      FBXSDK_printf("Error: Unable to create FBX Manager!\n");
      exit(1);
   }
   else FBXSDK_printf("Autodesk FBX SDK version %s\n", lSdkManager->GetVersion());

   //Create an IOSettings object. This object holds all import/export settings.
   FbxIOSettings* tIos = FbxIOSettings::Create(lSdkManager, IOSROOT);
   lSdkManager->SetIOSettings(tIos);
   ios = lSdkManager->GetIOSettings();

   //Load plugins from the executable directory (optional)
   FbxString lPath = FbxGetApplicationDirectory();
   lSdkManager->LoadPluginsDirectory(lPath.Buffer());

   //Create an FBX scene. This object holds most objects imported/exported from/to files.
   lScene = FbxScene::Create(lSdkManager, "My Scene");
   if( !lScene )
   {
      FBXSDK_printf("Error: Unable to create FBX scene!\n");
      exit(1);
   }

   // create scene info
   FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(lSdkManager,"SceneInfo");
   sceneInfo->mTitle = filename.c_str();
   //sceneInfo->mSubject = "Illustrates the creation and animation of a deformed cylinder.";
   //sceneInfo->mAuthor = "ExportScene01.exe sample program.";
   //sceneInfo->mRevision = "rev. 1.0";
   //sceneInfo->mKeywords = "deformed cylinder";
   //sceneInfo->mComment = "no particular comments required.";

   // we need to add the sceneInfo before calling AddThumbNailToScene because
   // that function is asking the scene for the sceneInfo.
   lScene->SetSceneInfo(sceneInfo);

   return true; //execStatus==true
}

bool PackerFBX::pack()
{
   //AddThumbnailToScene(lScene);

   mesh = createMesh();
   skeletonRoot = createSkeleton();

   // Build the node tree.
   FbxNode* lRootNode = lScene->GetRootNode();
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
   if( lSdkManager ) lSdkManager->Destroy();
   if( lResult )
   {
      FBXSDK_printf("Program Success!\n");
      return true; //execStatus==true
   }
   return false;
}

void PackerFBX::saveFBX()
{
   // Save the scene.
   int pFileFormat=-1;
   bool pEmbedMedia=false;

   int lMajor, lMinor, lRevision;
   bool lStatus = true;

   // Create an exporter.
   FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

   if( pFileFormat < 0 || pFileFormat >= lSdkManager->GetIOPluginRegistry()->GetWriterFormatCount() )
   {
      // Write in fall back format in less no ASCII format found
      pFileFormat = lSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat();

      //Try to export in ASCII if possible
      int lFormatIndex, lFormatCount = lSdkManager->GetIOPluginRegistry()->GetWriterFormatCount();

      for (lFormatIndex=0; lFormatIndex<lFormatCount; lFormatIndex++)
      {
         if (lSdkManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
         {
            FbxString lDesc =lSdkManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
            const char *lASCII = "ascii";
            if (lDesc.Find(lASCII)>=0)
            {
               pFileFormat = lFormatIndex;
               break;
            }
         }
      }
   }

   // Set the export states. By default, the export states are always set to
   // true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below
   // shows how to change these states.
   ios->SetBoolProp(EXP_FBX_MATERIAL,        true);
   ios->SetBoolProp(EXP_FBX_TEXTURE,         true);
   ios->SetBoolProp(EXP_FBX_EMBEDDED,        pEmbedMedia);
   ios->SetBoolProp(EXP_FBX_SHAPE,           true);
   ios->SetBoolProp(EXP_FBX_GOBO,            true);
   ios->SetBoolProp(EXP_FBX_ANIMATION,       true);
   ios->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

   // Initialize the exporter by providing a filename.
   if(lExporter->Initialize((pathFileNoExt+".fbx").c_str(), pFileFormat, lSdkManager->GetIOSettings()) == false)
   {
      FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
      FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
      lResult = false;
   }
   else  {

      FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
      FBXSDK_printf("FBX file format version %d.%d.%d\n\n", lMajor, lMinor, lRevision);

      // Export the scene.
      lStatus = lExporter->Export(lScene);

      // Destroy the exporter.
      lExporter->Destroy();

      if(lResult == false)
      {
         FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
      }
   }
}

FbxNode* PackerFBX::createMesh()
{
   FbxPatch* lPatch = FbxPatch::Create(lScene,"Character");

   // Set patch properties.
   lPatch->InitControlPoints(4, FbxPatch::eBSpline, 7, FbxPatch::eBSpline);
   lPatch->SetStep(4, 4);
   lPatch->SetClosed(true, false);

   FbxVector4* lVector4 = lPatch->GetControlPoints();
   int i;

   for (i = 0; i < 7; i++)
   {
      double lRadius = 15.0;
      double lSegmentLength = 20.0;
      lVector4[4*i + 0].Set(lRadius, 0.0, (i-3)*lSegmentLength);
      lVector4[4*i + 1].Set(0.0, -lRadius, (i-3)*lSegmentLength);
      lVector4[4*i + 2].Set(-lRadius, 0.0, (i-3)*lSegmentLength);
      lVector4[4*i + 3].Set(0.0, lRadius, (i-3)*lSegmentLength);
   }

   FbxNode* lNode = FbxNode::Create(lScene,"Character");

   // Rotate the cylinder along the X axis so the axis
   // of the cylinder is the same as the bone axis (Y axis)
   FbxVector4 lR(-90.0, 0.0, 0.0);
   lNode->LclRotation.Set(lR);
   lNode->SetNodeAttribute(lPatch);

   return lNode;
}

FbxNode* PackerFBX::createSkeleton()
{
   std::string name = "Skeleton";

   // Create skeleton root.
   FbxString lRootName(name.c_str());
   lRootName += "Root";
   FbxSkeleton* lSkeletonRootAttribute = FbxSkeleton::Create(lScene, lRootName);
   lSkeletonRootAttribute->SetSkeletonType(FbxSkeleton::eRoot);
   FbxNode* lSkeletonRoot = FbxNode::Create(lScene,lRootName.Buffer());
   lSkeletonRoot->SetNodeAttribute(lSkeletonRootAttribute);
   lSkeletonRoot->LclTranslation.Set(FbxVector4(0.0, -40.0, 0.0));

   // Create skeleton first limb node.
   FbxString lLimbNodeName1(name.c_str());
   lLimbNodeName1 += "LimbNode1";
   FbxSkeleton* lSkeletonLimbNodeAttribute1 = FbxSkeleton::Create(lScene,lLimbNodeName1);
   lSkeletonLimbNodeAttribute1->SetSkeletonType(FbxSkeleton::eLimbNode);
   lSkeletonLimbNodeAttribute1->Size.Set(1.0);
   FbxNode* lSkeletonLimbNode1 = FbxNode::Create(lScene,lLimbNodeName1.Buffer());
   lSkeletonLimbNode1->SetNodeAttribute(lSkeletonLimbNodeAttribute1);
   lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(0.0, 40.0, 0.0));

   // Create skeleton second limb node.
   FbxString lLimbNodeName2(name.c_str());
   lLimbNodeName2 += "LimbNode2";
   FbxSkeleton* lSkeletonLimbNodeAttribute2 = FbxSkeleton::Create(lScene,lLimbNodeName2);
   lSkeletonLimbNodeAttribute2->SetSkeletonType(FbxSkeleton::eLimbNode);
   lSkeletonLimbNodeAttribute2->Size.Set(1.0);
   FbxNode* lSkeletonLimbNode2 = FbxNode::Create(lScene,lLimbNodeName2.Buffer());
   lSkeletonLimbNode2->SetNodeAttribute(lSkeletonLimbNodeAttribute2);
   lSkeletonLimbNode2->LclTranslation.Set(FbxVector4(0.0, 40.0, 0.0));

   // Build skeleton node hierarchy.
   lSkeletonRoot->AddChild(lSkeletonLimbNode1);
   lSkeletonLimbNode1->AddChild(lSkeletonLimbNode2);

   return lSkeletonRoot;
}

void PackerFBX::createWeights()
{



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
}
