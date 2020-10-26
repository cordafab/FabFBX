#include "packerFBX.h"

#define PACK 10

PackerFBX::PackerFBX()
{
   lSdkManager = NULL;
   ios = NULL;
   lScene = NULL;
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

   //FbxNode* lPatch = CreatePatch(lScene, "Patch");
   //FbxNode* lSkeletonRoot = CreateSkeleton(lScene, "Skeleton");

   // Build the node tree.
   //FbxNode* lRootNode = lScene->GetRootNode();
   //lRootNode->AddChild(lPatch);
   //lRootNode->AddChild(lSkeletonRoot);

   // Store poses
   //LinkPatchToSkeleton(lScene, lPatch, lSkeletonRoot);
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

