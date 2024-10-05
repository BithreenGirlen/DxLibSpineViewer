#ifndef DXLIB_IMAGE_ENCODER_H_
#define DXLIB_IMAGE_ENCODER_H_

#include <string>
#include <vector>

class CDxLibImageEncoder
{
public:
	CDxLibImageEncoder();
	~CDxLibImageEncoder();

	static bool SaveScreenAsPng(const std::wstring& wstrFilePath, void* pWindowHandle = nullptr);
	static bool GetScreenPixels(int* iWidth, int* iHeight, int *iStride, std::vector<unsigned char>& pixels, void* pWindowHandle = nullptr);
private:
	static void GetScreenSize(int* iWidth, int* iHeight, void* pWindowHandle = nullptr);
};

#endif // !DXLIB_IMAGE_ENCODER_H_
