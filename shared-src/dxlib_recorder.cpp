
#include <string>
#include <vector>

#include "dxlib_handle.h"
#include "dxlib_recorder.h"

/* WIC and MF as encoder */
#include "mf_video_encoder.h"
#include "win_image.h"
#include "cpu.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

using DxLibImageHandle = DxLibHandle<&DxLib::DeleteGraph>;

/// @brief Reads pixels from GPU resource
class CDxLibMap
{
public:
	CDxLibMap(int iTextureHandle)
		:m_imageHandle(iTextureHandle)
	{
		ReadPixels();
	}
	~CDxLibMap()
	{
		Unlock();
	}

	bool isRead() const { return m_isLocked; }

	int width = 0;
	int height = 0;

	int stride = 0;
	unsigned char* pPixels = nullptr;
	DxLib::COLORDATA* pFormat = nullptr;
private:
	int m_imageHandle = -1;
	bool m_isLocked = false;

	bool ReadPixels()
	{
		int iRet = DxLib::GetGraphSize(m_imageHandle, &width, &height);
		if (iRet == -1)return false;

		void* pData = nullptr;
		iRet = DxLib::GraphLock(m_imageHandle, &stride, &pData, &pFormat);
		m_isLocked = iRet != -1;
		pPixels = static_cast<unsigned char*>(pData);

		return m_isLocked;
	}
	void Unlock() const
	{
		if (m_imageHandle != -1 && m_isLocked)
		{
			DxLib::GraphUnLock(m_imageHandle);
		}
	}
};


class CDxLibRecorder::Impl
{
public:
	Impl()
	{
		m_isAmdCpu = cpu::IsAmd();
	}
	~Impl()
	{
		Clear();
	}

	bool Start(bool isToBeVideo);
	bool IsUnderRecording()const { return m_isUnderRecording; }

	bool Capture(const wchar_t* filePath);

	bool End(EOutputType eOutputType, const wchar_t* pwzFilePath);
private:
	std::vector<DxLibImageHandle> m_images;
	std::vector<std::wstring> m_imageFilePaths;

	bool m_isAmdCpu = false;
	bool m_isToBeVideo = false;
	bool m_isUnderRecording = false;

	void Clear();
};

bool CDxLibRecorder::Impl::Start(bool isToBeVideo)
{
	Clear();

	m_isToBeVideo = isToBeVideo;
	m_isUnderRecording = true;

	return true;
}

bool CDxLibRecorder::Impl::Capture(const wchar_t* filePath)
{
	if (!IsUnderRecording())
	{
		return false;
	}

	int iGraphWidth = 0;
	int iGraphHeight = 0;
	DxLib::GetScreenState(&iGraphWidth, &iGraphHeight, nullptr);

	/*
	* Truncate video dimension to be multiple of 4 to prevent AMD CPU from hanging on final output
	* in spite of successful return values of mediatype setup and sample delivering.
	*/
	if (m_isAmdCpu && m_isToBeVideo)
	{
		iGraphWidth &= 0xfffffffc;
		iGraphHeight &= 0xfffffffc;
	}

	DxLibImageHandle dxLibGraphHandle(DxLib::MakeGraph(iGraphWidth, iGraphHeight));
	if (dxLibGraphHandle.Get() == -1)return false;

	int iRet = DxLib::GetDrawScreenGraph(0, 0, iGraphWidth, iGraphHeight, dxLibGraphHandle.Get());
	if (iRet == -1)return false;

	m_images.push_back(std::move(dxLibGraphHandle));
	if (filePath != nullptr)m_imageFilePaths.push_back(filePath);

	return true;
}

bool CDxLibRecorder::Impl::End(EOutputType eOutputType, const wchar_t* filePath)
{
	if (filePath == nullptr)
	{
		Clear();
		return false;
	}

	if (eOutputType == EOutputType::kPngs)
	{
		for (size_t i = 0; i < m_images.size() && i < m_imageFilePaths.size(); ++i)
		{
			auto& image = m_images[i];

			CDxLibMap s(image.Get());
			if (s.isRead())
			{
				std::wstring wstrFilePath = filePath + m_imageFilePaths[i] + L".png";
				win_image::SaveImageAsPng(wstrFilePath.c_str(), s.width, s.height, s.stride, s.pPixels, true);
			}
			image.Reset();
		}
	}
	else if (eOutputType == EOutputType::kGif)
	{
		win_image::CWicGifEncoder sWicGifEncoder;
		bool bRet = sWicGifEncoder.Initialise(filePath);
		if (bRet)
		{
			for (auto& image : m_images)
			{
				CDxLibMap s(image.Get());
				if (s.isRead())
				{
					sWicGifEncoder.CommitFrame(s.width, s.height, s.stride, s.pPixels, true,  1/ 15.f);
				}
				image.Reset();
			}
			sWicGifEncoder.Finalise();
		}
	}
	else if (eOutputType == EOutputType::kVideo)
	{
		CMfVideoEncoder sMfVideoEncoder;
		int iVideoWidth = 0;
		int iVideoHeight = 0;
		if (!m_images.empty())
		{
			DxLib::GetGraphSize(m_images[0].Get(), &iVideoWidth, &iVideoHeight);
		}

		bool bRet = sMfVideoEncoder.Start(filePath, iVideoWidth, iVideoHeight, 60);
		if (bRet)
		{
			for (auto& image : m_images)
			{
				CDxLibMap s(image.Get());
				sMfVideoEncoder.AddCpuFrame(s.pPixels, static_cast<unsigned long>(s.stride * s.height), true);
				image.Reset();
			}

			sMfVideoEncoder.End();
		}
	}

	Clear();

	return true;
}

void CDxLibRecorder::Impl::Clear()
{
	m_images.clear();
	m_imageFilePaths.clear();

	m_isToBeVideo = false;
	m_isUnderRecording = false;
}



CDxLibRecorder::CDxLibRecorder()
{
	m_impl = new CDxLibRecorder::Impl();
}

CDxLibRecorder::~CDxLibRecorder()
{
	delete m_impl;
}

bool CDxLibRecorder::Start(EOption eOption)
{
	return m_impl->Start(eOption == EOption::kAsVideo);
}
bool CDxLibRecorder::CaptureFrame(const wchar_t* pwzFileName)
{
	return m_impl->Capture(pwzFileName);
}
bool CDxLibRecorder::End(EOutputType eOutputFormat, const wchar_t* pwzFilePath)
{
	return m_impl->End(eOutputFormat, pwzFilePath);
}
