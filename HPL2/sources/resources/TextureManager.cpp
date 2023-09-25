/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *#
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "resources/TextureManager.h"
#include "engine/RTTI.h"
#include "graphics/AnimatedImage.h"
#include "graphics/Bitmap.h"
#include "graphics/Graphics.h"
#include "graphics/Image.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/Texture.h"
#include "resources/BitmapLoaderHandler.h"
#include "resources/FileSearcher.h"
#include "resources/LowLevelResources.h"
#include "resources/Resources.h"
#include "system/LowLevelSystem.h"
#include "system/String.h"

#include <memory>
#include <vector>

#include "Common_3/Utilities/Interfaces/ILog.h"
#include <FixPreprocessor.h>

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cTextureManager::cTextureManager(cGraphics* apGraphics,cResources *apResources)
		: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
							apResources->GetLowLevelSystem())
	{
		mpGraphics = apGraphics;
		mpResources = apResources;

		mpBitmapLoaderHandler = mpResources->GetBitmapLoaderHandler();

		mlMemoryUsage =0;

		mvCubeSideSuffixes.push_back("_pos_x");
		mvCubeSideSuffixes.push_back("_neg_x");
		mvCubeSideSuffixes.push_back("_pos_y");
		mvCubeSideSuffixes.push_back("_neg_y");
		mvCubeSideSuffixes.push_back("_pos_z");
		mvCubeSideSuffixes.push_back("_neg_z");
	}

	cTextureManager::~cTextureManager()
	{
		STLMapDeleteAll(m_mapAttenuationTextures);
		DestroyAll();
		Log(" Destroyed all textures\n");
	}


	Image* cTextureManager::_wrapperImageResource(const tString& asName, std::function<Image*(const tString& asName, const tWString& path, cBitmap* bitmap)> createImageHandler) {
		tWString sPath;
		BeginLoad(asName);

		Image* resource = FindImageResource(asName, sPath);
		if( resource==NULL && sPath!=_W(""))
		{
			// pTexture = FindTexture2D(asName,sPath);
			cBitmap *pBmp = mpBitmapLoaderHandler->LoadBitmap(sPath,0);
			if(!pBmp) {

				Error("Texture manager Couldn't load bitmap '%s'\n", cString::To8Char(sPath).c_str());
				EndLoad();
				return nullptr;
			}
			resource = createImageHandler(asName, sPath, pBmp);

			//Bitmap is no longer needed so delete it.
			hplDelete(pBmp);

			// mlMemoryUsage += pTexture->GetMemorySize();
			AddResource(resource);
		}

		if(resource) {
			resource->IncUserCount();
		}
		EndLoad();

		return resource;
	}


	Image* cTextureManager::Create1DImage(
		const tString& asName, bool abUseMipMaps, eTextureUsage aUsage, unsigned int alTextureSizeLevel)
	{
		return _wrapperImageResource(
			asName,
			[&abUseMipMaps](const tString& asName, const tWString& path, cBitmap* pBmp) -> Image*
			{
				auto* resource = new Image(asName, path);
				SharedTexture::BitmapLoadOptions opts = {0};
				opts.m_useMipmaps = abUseMipMaps;
				auto handle = SharedTexture::LoadFromHPLBitmap(*pBmp, opts);
				resource->SetForgeTexture(std::move(handle));
				return resource;
			});
	}

	Image* cTextureManager::Create2DImage(
		const tString& asName, bool abUseMipMaps, eTextureType aType, eTextureUsage aUsage, unsigned int alTextureSizeLevel)
	{
		return _wrapperImageResource(
			asName,
			[&abUseMipMaps](const tString& asName, const tWString& path, cBitmap* pBmp) -> Image*
			{
				auto* resource = new Image(asName, path);
				SharedTexture::BitmapLoadOptions opts = {0};
				opts.m_useMipmaps = abUseMipMaps;
				auto handle = SharedTexture::LoadFromHPLBitmap(*pBmp, opts);
				resource->SetForgeTexture(std::move(handle));

				return resource;
			});
	}
	Image* cTextureManager::CreateCubeMapImage(const tString& asPathName,bool abUseMipMaps, eTextureUsage aUsage,
				unsigned int alTextureSizeLevel) {

		tString sExt = cString::ToLowerCase(cString::GetFileExt(asPathName));

		if(sExt == "dds")
		{
			return _wrapperImageResource(asPathName,
			[&abUseMipMaps](const tString& asName, const tWString& path, cBitmap* pBmp) -> Image*
				{
					auto* resource = new Image(asName, path);
					SharedTexture::BitmapLoadOptions opts = {0};
					opts.m_useMipmaps = abUseMipMaps;
					opts.m_useCubeMap = true;
					auto handle = SharedTexture::LoadFromHPLBitmap(*pBmp, opts);
					resource->m_texture = std::move(handle);
                    return resource;
				});
		}

		tString name = cString::SetFileExt(asPathName,"");
		tWString fakeFullPath = cString::To16Char(name);
		Image* image = static_cast<Image*>(GetResource(fakeFullPath));

		BeginLoad(asPathName);
		if(!image) {
			//See if files for all faces exist
			tWStringVec vPaths;
			tWString sPath=_W("");
			for(int i=0;i <6 ;i++)
			{
				tStringVec *apFileFormatsVec = mpBitmapLoaderHandler->GetSupportedTypes();
				for(tStringVecIt it = apFileFormatsVec->begin();it!=apFileFormatsVec->end();++it)
				{
					tString sNewName = name + mvCubeSideSuffixes[i] + "." + *it;
					sPath = mpFileSearcher->GetFilePath(sNewName);

					if(sPath!=_W(""))break;
				}

				if(sPath==_W(""))
				{
					tString sNewName = name + mvCubeSideSuffixes[i];
					Error("Couldn't find %d-face '%s', for cubemap '%s' in path: '%s'\n",i,sNewName.c_str(),name.c_str(), asPathName.c_str());
					return NULL;
				}

				vPaths.push_back(sPath);
			}

			//Load bitmaps for all faces
			std::vector<cBitmap*> vBitmaps;
			for(int i=0;i<6; i++)
			{
				cBitmap* pBmp = mpBitmapLoaderHandler->LoadBitmap(vPaths[i],0);
				if(pBmp==NULL){
					Error("Couldn't load bitmap '%s'!\n",cString::To8Char(vPaths[i]).c_str());
					for(int j=0;j<(int)vBitmaps.size();j++) hplDelete(vBitmaps[j]);
					EndLoad();
					return NULL;
				}

				vBitmaps.push_back(pBmp);
			}
			ASSERT(vBitmaps.size() == 6 && "vBitmaps.size() == 6");

			SharedTexture::BitmapCubmapLoadOptions opts = {0};
			opts.m_useMipmaps = abUseMipMaps;
			auto handle = SharedTexture::CreateCubemapFromHPLBitmaps(vBitmaps, opts);
			image->SetForgeTexture(std::move(handle));

			for(int j=0;j<(int)vBitmaps.size();j++)	{
				hplDelete(vBitmaps[j]);
			}
		}

		if(image) {
			image->IncUserCount();
		}

		EndLoad();
		return image;
	}

	Image* cTextureManager::Create3DImage(
		const tString& asName, bool abUseMipMaps, eTextureUsage aUsage, unsigned int alTextureSizeLevel)
	{
		return _wrapperImageResource(
			asName,
			[ &abUseMipMaps](const tString& asName, const tWString& path, cBitmap* pBmp) -> Image*
			{
				auto* resource = new Image(asName, path);
				SharedTexture::BitmapLoadOptions opts = {0};
				opts.m_useMipmaps = abUseMipMaps;
				auto handle = SharedTexture::LoadFromHPLBitmap(*pBmp, opts);
				resource->SetForgeTexture(std::move(handle));
				return resource;
			});
	}

    AnimatedImage* cTextureManager::CreateAnimImage(
        const tString& asName, bool abUseMipMaps, eTextureType aType, eTextureUsage aUsage, unsigned int alTextureSizeLevel) {
            BeginLoad(asName);

            ///////////////////////////
            // Check the base name
            int lPos = cString::GetFirstStringPos(asName, "01");
            if (lPos < 0) {
                    Error("First frame of animation '%s' must contain '01'!\n", asName.c_str());
                    return NULL;
            }

            // Remove 01 in the string
            tString sSub1 = cString::Sub(asName, 0, lPos);
            tString sSub2 = cString::Sub(asName, lPos + 2);
            tString sBaseName = sSub1 + sSub2;

            if (sSub2.size() == 0 || sSub2[0] != '.') {
                    Error("First frame of animation '%s' must contain '01' before extension!\n", asName.c_str());
                    return NULL;
            }

            ///////////////////////////
            // Check if texture exists

            // Create a fake full path.
            tWString sFirstFramePath = mpFileSearcher->GetFilePath(asName);
            if (sFirstFramePath == _W("")) {
                    Error("First frame of animation '%s' could not be found!\n", asName.c_str());
                    return NULL;
            }
            tWString sFakeFullPath = cString::GetFilePathW(sFirstFramePath) + cString::To16Char(cString::GetFileName(sBaseName));

            AnimatedImage* animatedImage = static_cast<AnimatedImage*>(GetResource(sFakeFullPath));

            ///////////////////////////
            // Check if texture exists
            if (!animatedImage) {
                    tString sFileExt = cString::GetFileExt(sBaseName);
                    tString sFileName = cString::SetFileExt(cString::GetFileName(sBaseName), "");

                    tStringVec mvFileNames;

                    tString sTest = sFileName + "01." + sFileExt;
                    int lNum = 2;
                    tWStringVec vPaths;

                    while (true) {
                            tWString sPath = mpFileSearcher->GetFilePath(sTest);

                            if (sPath == _W("")) {
                                    break;
                            } else {
                                    vPaths.push_back(sPath);
                                    if (lNum < 10)
                                        sTest = sFileName + "0" + cString::ToString(lNum) + "." + sFileExt;
                                    else
                                        sTest = sFileName + cString::ToString(lNum) + "." + sFileExt;

                                    ++lNum;
                            }
                    }

                    if (vPaths.empty()) {
                            Error("No textures found for animation %s\n", sBaseName.c_str());
                            EndLoad();
                            return NULL;
                    }

                    std::vector<cBitmap*> vBitmaps;
                    for (size_t i = 0; i < vPaths.size(); ++i) {
                            cBitmap* pBmp = mpBitmapLoaderHandler->LoadBitmap(vPaths[i], 0);
                            if (pBmp == NULL) {
                                    Error("Couldn't load bitmap '%s'!\n", cString::To8Char(vPaths[i]).c_str());

                                    for (int j = 0; j < (int)vBitmaps.size(); j++)
                                        hplDelete(vBitmaps[j]);

                                    EndLoad();
                                    return NULL;
                            }

                            vBitmaps.push_back(pBmp);
                    }

                    // Create the animated texture

                    animatedImage = new AnimatedImage(sBaseName, sFakeFullPath);
                    std::vector<std::unique_ptr<Image>> images;
                    for (auto& bitmap : vBitmaps) {
                            std::unique_ptr<Image> image = std::make_unique<Image>();

                            SharedTexture::BitmapLoadOptions opts = { 0 };
                            opts.m_useMipmaps = abUseMipMaps;
                            opts.m_useCubeMap = aType == eTextureType_CubeMap;
                            auto handle = SharedTexture::LoadFromHPLBitmap(*bitmap, opts);
                            image->SetForgeTexture(std::move(handle));

                            // Image::InitializeFromBitmap(*image, *bitmap, desc);
                            images.push_back(std::move(image));
                    }
                    animatedImage->Initialize(std::span(images));

                    AddResource(animatedImage);
            }
            if (animatedImage) {
                    animatedImage->IncUserCount();
            }
            EndLoad();
            return animatedImage;
    }

    iTexture* cTextureManager::Create1D(
        const tString& asName, bool abUseMipMaps, eTextureUsage aUsage, unsigned int alTextureSizeLevel) {
            ASSERT(false && "Deprecated and not implemented");
            return nullptr;
    }

    iTexture* cTextureManager::Create2D(
        const tString& asName, bool abUseMipMaps, eTextureType aType, eTextureUsage aUsage, unsigned int alTextureSizeLevel) {
            ASSERT(false && "Deprecated and not implemented");
            return nullptr;
    }

    iTexture* cTextureManager::Create3D(
        const tString& asName, bool abUseMipMaps, eTextureUsage aUsage, unsigned int alTextureSizeLevel) {
            ASSERT(false && "Deprecated and not implemented");
            return nullptr;
    }

    iTexture* cTextureManager::CreateAnim(
        const tString& asFirstFrameName, bool abUseMipMaps, eTextureType aType, eTextureUsage aUsage, unsigned int alTextureSizeLevel) {
            ASSERT(false && "Deprecated and not implemented");
            return nullptr;
    }

    iTexture* cTextureManager::CreateCubeMap(
        const tString& asPathName, bool abUseMipMaps, eTextureUsage aUsage, unsigned int alTextureSizeLevel) {
            ASSERT(false && "Deprecated and not implemented");
            return nullptr;
    }


	void cTextureManager::Unload(iResourceBase* apResource)
	{

	}
	//-----------------------------------------------------------------------

	void cTextureManager::Destroy(iResourceBase* apResource)
	{
		apResource->DecUserCount();

		if(apResource->HasUsers()==false)
		{
			RemoveResource(apResource);
			delete apResource;
		}
	}

	//-----------------------------------------------------------------------

	void cTextureManager::Update(float afTimeStep)
	{
		for(auto& res: m_mapResources) {
			if (hpl::TypeInfo<hpl::AnimatedImage>::IsType(*res.second)) {
				auto* animatedImage = static_cast<hpl::AnimatedImage*>(res.second);
				animatedImage->Update(afTimeStep);
			}
		}
	}

	Image* cTextureManager::FindImageResource(const tString &asName, tWString &asFilePath) {
		Image *pTexture=NULL;

		if(cString::GetFileExt(asName)=="")
		{
			int lMaxCount =-1;

			///////////////////////
			//Iterate the different formats
			tStringVec *apFileFormatsVec = mpBitmapLoaderHandler->GetSupportedTypes();
			for(tStringVecIt it = apFileFormatsVec->begin();it!= apFileFormatsVec->end();++it)
			{
				tWString sTempPath = _W("");
				int lCount=0;

				tString sNewName = cString::SetFileExt(asName,*it);
				auto resource = static_cast<Image*> (FindLoadedResource(sNewName, sTempPath, &lCount));

				///////////////////////
				//Check if the image exists and then check if it has the hightest equal count.
				if((resource==nullptr && sTempPath!=_W("")) || resource != nullptr)
				{
					if(lCount > lMaxCount)
					{
						lMaxCount = lCount;
						asFilePath = sTempPath;
						return resource;
					}
				}
			}
		}
		return static_cast<Image*> (FindLoadedResource(asName, asFilePath));
	}


	iTexture* cTextureManager::FindTexture2D(const tString &asName, tWString &asFilePath)
	{
		iTexture *pTexture=NULL;

		if(cString::GetFileExt(asName)=="")
		{
			int lMaxCount =-1;

			///////////////////////
			//Iterate the different formats
			tStringVec *apFileFormatsVec = mpBitmapLoaderHandler->GetSupportedTypes();
			for(tStringVecIt it = apFileFormatsVec->begin();it!= apFileFormatsVec->end();++it)
			{
				tWString sTempPath = _W("");
				iTexture *pTempTex=NULL;
				int lCount=0;

				tString sNewName = cString::SetFileExt(asName,*it);
				pTempTex = static_cast<iTexture*> (FindLoadedResource(sNewName, sTempPath, &lCount));

				///////////////////////
				//Check if the image exists and then check if it has the hightest equal count.
				if((pTempTex==NULL && sTempPath!=_W("")) || pTempTex!=NULL)
				{
					if(lCount > lMaxCount)
					{
						lMaxCount = lCount;
						asFilePath = sTempPath;
						pTexture = pTempTex;
					}
				}
			}
		}
		else
		{
			pTexture = static_cast<iTexture*> (FindLoadedResource(asName, asFilePath));
		}

		return pTexture;
	}

}
