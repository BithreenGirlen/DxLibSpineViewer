#ifndef MF_VIDEO_ENCODER_H_
#define MF_VIDEO_ENCODER_H_

#include <Windows.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class CMfVideoEncoder
{
public:
	CMfVideoEncoder();
	~CMfVideoEncoder();

	bool Start(const wchar_t* pwzFilePath, UINT32 uiWidth, UINT32 uiHeight, UINT32 uiFrameRate);

	bool AddCpuFrame(unsigned char* pPixels, unsigned long ulPixelSize, bool bIsRgba = true);
#ifdef MF_GPU_TEXTURE
	bool AddGpuFrame(void* pD3D11Texture2D);
#endif
	void End();
private:
	enum Constants{kDefaultFrameRate = 30};

	HRESULT m_hrComInit = E_FAIL;
	HRESULT m_hrMStartup = E_FAIL;

	IMFSinkWriter* m_pMfSinkWriter = nullptr;
	IMFMediaBuffer* m_pMfMediaBuffer = nullptr;
	LONGLONG m_llCurrentFrame = 0;
	UINT32 m_uiFrameRate = Constants::kDefaultFrameRate;
	DWORD m_ulOutStreamIndex = 0;

	bool AddFrame();

	void Clear();

	void ReleaseMfSinkWriter();
	bool CreateMediaBuffer(DWORD ulPixelSize);
	bool CheckMediaBufferSize(DWORD ulPixelSize);
	void ReleaseMediaBuffer();
};
#endif // !MF_VIDEO_ENCODER_H_
