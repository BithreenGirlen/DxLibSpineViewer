

#include "dxlib_image_encoder.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>


/*描画対象をJPGとして保存*/
bool CDxLibImageEncoder::SaveScreenAsJpg(const wchar_t* wszFilePath)
{
	int iGraphWidth = 0;
	int iGraphHeight = 0;
	DxLib::GetScreenState(&iGraphWidth, &iGraphHeight, nullptr);

	int iRet = DxLib::SaveDrawScreenToJPEG(0, 0, iGraphWidth, iGraphHeight, wszFilePath);
	return iRet != -1;
}
/*描画対象をPNGとして保存*/
bool CDxLibImageEncoder::SaveScreenAsPng(const wchar_t *wszFilePath)
{
	int iGraphWidth = 0;
	int iGraphHeight = 0;
	DxLib::GetScreenState(&iGraphWidth, &iGraphHeight, nullptr);

	int iRet = DxLib::SaveDrawScreenToPNG(0, 0, iGraphWidth, iGraphHeight, wszFilePath);
	return iRet != -1;
}

bool CDxLibImageEncoder::SaveGraphicAsJpg(int iGraphicHandle, const wchar_t* wszFilePath)
{
	int iGraphWidth = 0;
	int iGraphHeight = 0;
	int iRet = DxLib::GetGraphSize(iGraphicHandle, &iGraphWidth, &iGraphHeight);
	if (iRet == -1)return false;

	iRet = DxLib::SaveDrawValidGraphToJPEG(iGraphicHandle, 0, 0, iGraphWidth, iGraphHeight, wszFilePath);
	return iRet != -1;
}

bool CDxLibImageEncoder::SaveGraphicAsPng(int iGraphicHandle, const wchar_t* wszFilePath)
{
	int iGraphWidth = 0;
	int iGraphHeight = 0;
	int iRet = DxLib::GetGraphSize(iGraphicHandle, &iGraphWidth, &iGraphHeight);
	if (iRet == -1)return false;

	iRet = DxLib::SaveDrawValidGraphToPNG(iGraphicHandle, 0, 0, iGraphWidth, iGraphHeight, wszFilePath);
	return iRet != -1;
}
/*描画対象の画素配列取得。*/
bool CDxLibImageEncoder::GetScreenPixels(int* iWidth, int* iHeight, int *iStride, std::vector<unsigned char>& pixels, bool bToCovertToRgba)
{
	int iGraphWidth = 0;
	int iGraphHeight = 0;
	DxLib::GetScreenState(&iGraphWidth, &iGraphHeight, nullptr);

	int iImageHandle = bToCovertToRgba ?
		DxLib::MakeABGR8ColorSoftImage(iGraphWidth, iGraphHeight) : DxLib::MakeARGB8ColorSoftImage(iGraphWidth, iGraphHeight);
	if (iImageHandle == -1)return false;

	int iRet = DxLib::GetDrawScreenSoftImage(0, 0, iGraphWidth, iGraphHeight, iImageHandle);
	if (iRet == -1)
	{
		DxLib::DeleteSoftImage(iImageHandle);
		return false;
	}

	unsigned char* pPixels = static_cast<unsigned char*>(DxLib::GetImageAddressSoftImage(iImageHandle));
	if (pPixels == nullptr)
	{
		DxLib::DeleteSoftImage(iImageHandle);
		return false;
	}
	int iPitch = DxLib::GetPitchSoftImage(iImageHandle);
	if (iPitch == -1)
	{
		DxLib::DeleteSoftImage(iImageHandle);
		return false;
	}

	*iStride = iPitch;
	*iWidth = iGraphWidth;
	*iHeight = iGraphHeight;

	size_t nSize = static_cast<size_t>(iPitch * iGraphHeight);
	pixels.resize(nSize);

	memcpy(&pixels[0], pPixels, nSize);

	DxLib::DeleteSoftImage(iImageHandle);

	return true;
}
