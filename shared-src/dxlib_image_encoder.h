﻿#ifndef DXLIB_IMAGE_ENCODER_H_
#define DXLIB_IMAGE_ENCODER_H_

#include <vector>

class CDxLibImageEncoder
{
public:
	CDxLibImageEncoder();
	~CDxLibImageEncoder();

	static bool SaveScreenAsJpg(const wchar_t* wszFilePath);
	static bool SaveScreenAsPng(const wchar_t* wszFilePath);

	static bool GetScreenPixels(int* iWidth, int* iHeight, int *iStride, std::vector<unsigned char>& pixels, bool bToCovertToRgba = true);

	static void GetScreenSize(int* iWidth, int* iHeight);
private:

};

#endif // !DXLIB_IMAGE_ENCODER_H_
