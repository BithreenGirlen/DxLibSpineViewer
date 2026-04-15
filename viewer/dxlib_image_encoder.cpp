

#include "dxlib_image_encoder.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>


bool dxlib_image_encoder::SaveScreenAsJpg(const wchar_t* filePath)
{
	int screenWidth = 0;
	int screenHeight = 0;
	DxLib::GetScreenState(&screenWidth, &screenHeight, nullptr);

	int iRet = DxLib::SaveDrawScreenToJPEG(0, 0, screenWidth, screenHeight, filePath);
	return iRet != -1;
}

bool dxlib_image_encoder::SaveScreenAsPng(const wchar_t *filePath)
{
	int screenWidth = 0;
	int screenHeight = 0;
	DxLib::GetScreenState(&screenWidth, &screenHeight, nullptr);

	int iRet = DxLib::SaveDrawScreenToPNG(0, 0, screenWidth, screenHeight, filePath);
	return iRet != -1;
}

bool dxlib_image_encoder::SaveRenderTextureAsJpg(int iGraphicHandle, const wchar_t* filePath)
{
	int textureWidth = 0;
	int textureHeight = 0;
	int iRet = DxLib::GetGraphSize(iGraphicHandle, &textureWidth, &textureHeight);
	if (iRet == -1)return false;

	iRet = DxLib::SaveDrawValidGraphToJPEG(iGraphicHandle, 0, 0, textureWidth, textureHeight, filePath);
	return iRet != -1;
}

bool dxlib_image_encoder::SaveRenderTextureAsPng(int iGraphicHandle, const wchar_t* filePath)
{
	int textureWidth = 0;
	int textureHeight = 0;
	int iRet = DxLib::GetGraphSize(iGraphicHandle, &textureWidth, &textureHeight);
	if (iRet == -1)return false;

	iRet = DxLib::SaveDrawValidGraphToPNG(iGraphicHandle, 0, 0, textureWidth, textureHeight, filePath);
	return iRet != -1;
}

bool dxlib_image_encoder::GetScreenPixels(int* iWidth, int* iHeight, int *iStride, std::vector<unsigned char>& pixels, bool toBeRgba)
{
	int screenWidth = 0;
	int screenHeight = 0;
	DxLib::GetScreenState(&screenWidth, &screenHeight, nullptr);

	int imageHandle = toBeRgba ?
		DxLib::MakeABGR8ColorSoftImage(screenWidth, screenHeight) :
		DxLib::MakeARGB8ColorSoftImage(screenWidth, screenHeight);
	if (imageHandle == -1)return false;

	int iRet = DxLib::GetDrawScreenSoftImage(0, 0, screenWidth, screenHeight, imageHandle);
	if (iRet == -1)
	{
		DxLib::DeleteSoftImage(imageHandle);
		return false;
	}

	unsigned char* pPixels = static_cast<unsigned char*>(DxLib::GetImageAddressSoftImage(imageHandle));
	if (pPixels == nullptr)
	{
		DxLib::DeleteSoftImage(imageHandle);
		return false;
	}
	int iPitch = DxLib::GetPitchSoftImage(imageHandle);
	if (iPitch == -1)
	{
		DxLib::DeleteSoftImage(imageHandle);
		return false;
	}

	*iStride = iPitch;
	*iWidth = screenWidth;
	*iHeight = screenHeight;

	size_t nSize = static_cast<size_t>(iPitch * screenHeight);
	pixels.resize(nSize);
	memcpy(&pixels[0], pPixels, nSize);

	DxLib::DeleteSoftImage(imageHandle);

	return true;
}
