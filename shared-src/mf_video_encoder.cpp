

#ifdef MF_GPU_TEXTURE
#include <d3d11.h>
#endif
#include <atlbase.h>
#include <mfapi.h>

#include "mf_video_encoder.h"

#pragma comment (lib,"Mfplat.lib")
#pragma comment (lib,"Mfreadwrite.lib")
#pragma comment (lib,"Mfuuid.lib")

CMfVideoEncoder::CMfVideoEncoder()
{
	m_hrComInit = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(m_hrComInit))return;

	m_hrMStartup = ::MFStartup(MF_VERSION);
}

CMfVideoEncoder::~CMfVideoEncoder()
{
	Clear();

	if (SUCCEEDED(m_hrMStartup))
	{
		::MFShutdown();
		m_hrMStartup = E_FAIL;
	}

	if (SUCCEEDED(m_hrComInit))
	{
		::CoUninitialize();
		m_hrComInit = E_FAIL;
	}
}

bool CMfVideoEncoder::Start(const wchar_t* pwzFilePath, UINT32 uiWidth, UINT32 uiHeight, UINT32 uiFrameRate)
{
	Clear();

	m_uiFrameRate = uiFrameRate;

	HRESULT hr = E_FAIL;

	CComPtr<IMFAttributes> pMfAttriubutes;
	hr = ::MFCreateAttributes(&pMfAttriubutes, 1);
	if (FAILED(hr))return false;

	/* AMD CPU hangs on IMFSinkWriter::Finalize() in spite of successful return value of the setting below. */

	hr = pMfAttriubutes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
	if (FAILED(hr))return false;
	hr = pMfAttriubutes->SetUINT32(MF_MT_DEFAULT_STRIDE, uiWidth * 4);
	if (FAILED(hr))return false;
	hr = pMfAttriubutes->SetGUID(MF_TRANSCODE_CONTAINERTYPE, MFTranscodeContainerType_MPEG4);
	if (FAILED(hr))return false;

	hr = ::MFCreateSinkWriterFromURL(pwzFilePath, nullptr, pMfAttriubutes, &m_pMfSinkWriter);
	if (FAILED(hr))return false;

	const auto SetOutStreamMediaType = [this, &uiWidth, &uiHeight]()
		-> bool
		{
			HRESULT hr = E_FAIL;
			CComPtr<IMFMediaType> pOutMfMediaType;
			hr = ::MFCreateMediaType(&pOutMfMediaType);
			if (FAILED(hr))return false;

			hr = pOutMfMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			if (FAILED(hr))return false;
			hr = pOutMfMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
			if (FAILED(hr))return false;

			UINT32 uiBitrate = uiWidth * uiHeight * 5;
			hr = pOutMfMediaType->SetUINT32(MF_MT_AVG_BITRATE, uiBitrate);
			if (FAILED(hr))return false;
			hr = pOutMfMediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
			if (FAILED(hr))return false;

			hr = ::MFSetAttributeSize(pOutMfMediaType, MF_MT_FRAME_SIZE, uiWidth, uiHeight);
			if (FAILED(hr))return false;
			hr = ::MFSetAttributeSize(pOutMfMediaType, MF_MT_FRAME_RATE, m_uiFrameRate, 1);
			if (FAILED(hr))return false;
			hr = ::MFSetAttributeSize(pOutMfMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			if (FAILED(hr))return false;

			hr = m_pMfSinkWriter->AddStream(pOutMfMediaType, &m_ulOutStreamIndex);
			if (FAILED(hr))return false;

			return true;
		};

	bool bRet = SetOutStreamMediaType();
	if (!bRet)return false;

	const auto SetInputMediaType = [this, &uiWidth, &uiHeight]()
		-> bool
		{
			HRESULT hr = E_FAIL;
			CComPtr<IMFMediaType> pInMfMediaType;
			hr = ::MFCreateMediaType(&pInMfMediaType);
			if (FAILED(hr))return false;

			hr = pInMfMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			if (FAILED(hr))return false;
			hr = pInMfMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
			if (FAILED(hr))return false;

			hr = pInMfMediaType->SetUINT32(MF_MT_DEFAULT_STRIDE, uiWidth * 4);
			if (FAILED(hr))return false;

			hr = ::MFSetAttributeSize(pInMfMediaType, MF_MT_FRAME_SIZE, uiWidth, uiHeight);
			if (FAILED(hr))return false;
			hr = ::MFSetAttributeSize(pInMfMediaType, MF_MT_FRAME_RATE, m_uiFrameRate, 1);
			if (FAILED(hr))return false;
			hr = ::MFSetAttributeSize(pInMfMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			if (FAILED(hr))return false;

			hr = m_pMfSinkWriter->SetInputMediaType(m_ulOutStreamIndex, pInMfMediaType, nullptr);
			if (FAILED(hr))return false;

			return true;
		};

	bRet = SetInputMediaType();
	if (!bRet)return false;

	hr = m_pMfSinkWriter->BeginWriting();

	return SUCCEEDED(hr);
}

bool CMfVideoEncoder::AddCpuFrame(unsigned char* pPixels, unsigned long ulPixelSize, bool bIsRgba)
{
	if (m_pMfSinkWriter == nullptr)return false;

	HRESULT hr = E_FAIL;

	bool bRet = CheckMediaBufferSize(ulPixelSize);
	if (!bRet)return false;

	BYTE* pBuffer = nullptr;
	hr = m_pMfMediaBuffer->Lock(&pBuffer, nullptr, nullptr);
	if (FAILED(hr))return false;
	/* RGBA => BGRA */
	if (bIsRgba)
	{
		uint32_t* pSrc32 = reinterpret_cast<uint32_t*>(pPixels);
		uint32_t* pDst32 = reinterpret_cast<uint32_t*>(pBuffer);
		size_t nCount = ulPixelSize / 4;
		for (size_t i = 0; i < nCount; ++i)
		{
			pDst32[i] = ((pSrc32[i] & 0x000000ff) << 16) |
						((pSrc32[i] & 0x00ff0000) >> 16) |
						((pSrc32[i] & 0xff00ff00));
		}
	}
	else
	{
		memcpy(pBuffer, pPixels, ulPixelSize);
	}

	hr= m_pMfMediaBuffer->SetCurrentLength(ulPixelSize);
	if (FAILED(hr))return false;

	hr = m_pMfMediaBuffer->Unlock();
	if (FAILED(hr))return false;

	return AddFrame();
}
#ifdef MF_GPU_TEXTURE
/* The format should be DXGI_FORMAT_B8G8R8A8_UNORM */
bool CMfVideoEncoder::AddGpuFrame(void* pD3D11Texture2D)
{
	if (pD3D11Texture2D == nullptr)return false;

	HRESULT hr = E_FAIL;
	auto pFrameTexture = static_cast<ID3D11Texture2D*>(pD3D11Texture2D);

	ReleaseMediaBuffer();
	hr = ::MFCreateDXGISurfaceBuffer(__uuidof(ID3D11Texture2D), pFrameTexture, 0, FALSE, &m_pMfMediaBuffer);
	if (FAILED(hr))return false;

	CComPtr<IMF2DBuffer> pMf2dBuffer;
	hr = m_pMfMediaBuffer->QueryInterface(__uuidof(IMF2DBuffer), reinterpret_cast<void**>(&pMf2dBuffer));
	if (FAILED(hr))return false;

	DWORD dwLength = 0;
	hr = pMf2dBuffer->GetContiguousLength(&dwLength);
	if (FAILED(hr))return false;

	m_pMfMediaBuffer->SetCurrentLength(dwLength);

	return AddFrame();
}
#endif /* MF_GPU_TEXTURE */

void CMfVideoEncoder::End()
{
	if (m_pMfSinkWriter == nullptr)return;

	m_pMfSinkWriter->Finalize();
	Clear();
}

bool CMfVideoEncoder::AddFrame()
{
	if (m_pMfSinkWriter == nullptr || m_pMfMediaBuffer == nullptr)return false;

	CComPtr<IMFSample> pMfSample;
	HRESULT hr = ::MFCreateSample(&pMfSample);
	if (FAILED(hr)) return false;

	hr = pMfSample->AddBuffer(m_pMfMediaBuffer);
	if (FAILED(hr)) return false;

	LONGLONG llDuration = 10LL * 1000 * 1000 / m_uiFrameRate;
	hr = pMfSample->SetSampleTime(llDuration * m_llCurrentFrame);
	if (FAILED(hr)) return false;
	++m_llCurrentFrame;

	hr = pMfSample->SetSampleDuration(llDuration);
	if (FAILED(hr)) return false;

	hr = m_pMfSinkWriter->WriteSample(m_ulOutStreamIndex, pMfSample);

	return SUCCEEDED(hr);
}

void CMfVideoEncoder::Clear()
{
	ReleaseMfSinkWriter();
	ReleaseMediaBuffer();

	m_llCurrentFrame = 0;
	m_uiFrameRate = Constants::kDefaultFrameRate;
	m_ulOutStreamIndex = 0;
}

void CMfVideoEncoder::ReleaseMfSinkWriter()
{
	if (m_pMfSinkWriter != nullptr)
	{
		m_pMfSinkWriter->Release();
		m_pMfSinkWriter = nullptr;
	}
}

bool CMfVideoEncoder::CreateMediaBuffer(DWORD ulPixelSize)
{
	ReleaseMediaBuffer();
	HRESULT hr = ::MFCreateMemoryBuffer(ulPixelSize, &m_pMfMediaBuffer);
	return SUCCEEDED(hr);
}

bool CMfVideoEncoder::CheckMediaBufferSize(DWORD ulPixelSize)
{
	if (m_pMfMediaBuffer == nullptr)
	{
		return CreateMediaBuffer(ulPixelSize);
	}
	else
	{
		DWORD ulSize = 0;
		HRESULT hr = m_pMfMediaBuffer->GetMaxLength(&ulSize);
		if (ulSize < ulPixelSize)
		{
			return CreateMediaBuffer(ulPixelSize);
		}
	}
	return true;
}

void CMfVideoEncoder::ReleaseMediaBuffer()
{
	if (m_pMfMediaBuffer != nullptr)
	{
		m_pMfMediaBuffer->Release();
		m_pMfMediaBuffer = nullptr;
	}
}
