

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
/*描画対象の画素配列取得。*/
bool CDxLibImageEncoder::GetScreenPixels(int* iWidth, int* iHeight, int *iStride, std::vector<unsigned char>& pixels, bool bToCovertToRgba)
{
	int iGraphWidth = 0;
	int iGraphHeight = 0;
	DxLib::GetScreenState(&iGraphWidth, &iGraphHeight, nullptr);

	/*SIHandleを毎度作成しても、寸法変更が生じた際にだけ作り直しても実行速度は大差なし*/
	int iImageHandle = DxLib::MakeARGB8ColorSoftImage(iGraphWidth, iGraphHeight);
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

	/*BGRA => RGBA*/
	if (bToCovertToRgba)
	{
		for (size_t i = 0; i < nSize - 3; i += 4)
		{
			pixels[i] = *(pPixels + i + 2);
			pixels[i + 1] = *(pPixels + i + 1);
			pixels[i + 2] = *(pPixels + i);
			pixels[i + 3] = *(pPixels + i + 3);
		}
	}
	else
	{
		memcpy(&pixels[0], pPixels, nSize);
	}

	DxLib::DeleteSoftImage(iImageHandle);

	return true;
}
