
#include <string>
#include <vector>

#include "dxlib_handle.h"
#include "dxlib_recorder.h"
#include "dxlib_map.h"

/* WIC and MF as encoder */
#include "mf_video_encoder.h"
#include "win_image.h"
#include "cpu.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

using DxLibImageHandle = DxLibHandle<&DxLib::DeleteGraph>;


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

	bool Start(EOutputType outputType, unsigned int fps);
	EOutputType GetoutputType() const { return m_outputType; }
	int GetFps() const { return m_fps; }

	EState GetState() const { return m_recorderState; }

	bool Capture(const wchar_t* imageName);
	bool CommitFrame(const int iGraphicHandle, const wchar_t* imageName);
	bool HasFrames() const { return !m_images.empty(); }

	bool End(const wchar_t* filePath);
private:
	std::vector<DxLibImageHandle> m_images;
	std::vector<std::wstring> m_imageNames;

	bool m_isAmdCpu = false;

	EOutputType m_outputType = EOutputType::Unknown;
	EState m_recorderState = EState::Idle;

	int m_fps = kDefaultFps;

	void Clear();

	void TruncateSize(int* width, int* height) const;
};

bool CDxLibRecorder::Impl::Start(EOutputType outputType, unsigned int fps)
{
	Clear();

	m_fps = fps;

	m_outputType = outputType;
	m_recorderState = EState::UnderRecording;

	return true;
}

bool CDxLibRecorder::Impl::Capture(const wchar_t* imageName)
{
	if (m_recorderState != EState::Idle && m_recorderState != EState::InitialisingVideoStream)
	{
		int iGraphWidth = 0;
		int iGraphHeight = 0;
		DxLib::GetScreenState(&iGraphWidth, &iGraphHeight, nullptr);

		TruncateSize(&iGraphWidth, &iGraphHeight);
		DxLibImageHandle dxLibGraphHandle(DxLib::MakeGraph(iGraphWidth, iGraphHeight));
		if (dxLibGraphHandle.Empty())return false;

		int iRet = DxLib::GetDrawScreenGraph(0, 0, iGraphWidth, iGraphHeight, dxLibGraphHandle.Get());
		if (iRet == -1)return false;

		m_images.push_back(std::move(dxLibGraphHandle));
		if (imageName != nullptr)m_imageNames.push_back(imageName);

		return true;
	}

	return false;
}

bool CDxLibRecorder::Impl::CommitFrame(const int iGraphicHandle, const wchar_t* imageName)
{
	int iGraphWidth = 0;
	int iGraphHeight = 0;
	int iRet = DxLib::GetGraphSize(iGraphicHandle, &iGraphWidth, &iGraphHeight);
	if (iRet == -1)return false;

	TruncateSize(&iGraphWidth, &iGraphHeight);
	DxLibImageHandle dxLibGraphHandle(DxLib::MakeGraph(iGraphWidth, iGraphHeight));
	if (dxLibGraphHandle.Empty())return false;

	iRet = DxLib::BltDrawValidGraph(iGraphicHandle, 0, 0, iGraphWidth, iGraphHeight, 0, 0, dxLibGraphHandle.Get());
	if (iRet == -1)return false;

	m_images.push_back(std::move(dxLibGraphHandle));
	if (imageName != nullptr)m_imageNames.push_back(imageName);

	return true;
}

bool CDxLibRecorder::Impl::End(const wchar_t* filePath)
{
	if (filePath == nullptr)
	{
		Clear();
		return false;
	}
	std::wstring wstrFilePath = filePath;

	switch (m_outputType)
	{
	case EOutputType::Gif:
	{
		wstrFilePath += L".gif";

		win_image::CWicGifEncoder wicGifEncoder;
		bool bRet = wicGifEncoder.Initialise(wstrFilePath.c_str());
		if (bRet)
		{
			for (auto& image : m_images)
			{
				CDxLibMap s(image.Get());
				if (s.IsAccessible())
				{
					wicGifEncoder.CommitFrame(s.width, s.height, s.stride, s.pPixels, true, 1.f / m_fps);
				}
				image.Reset();
			}
			wicGifEncoder.Finalise();
		}
	}
	break;
	case EOutputType::Video:
	{
		wstrFilePath += L".mp4";

		CMfVideoEncoder mfVideoEncoder;
		int iVideoWidth = 0;
		int iVideoHeight = 0;
		if (!m_images.empty())
		{
			DxLib::GetGraphSize(m_images[0].Get(), &iVideoWidth, &iVideoHeight);
		}

		/* Initialising input media types takes time. In the meantime, pause rendering. */
		m_recorderState = EState::InitialisingVideoStream;
		bool bRet = mfVideoEncoder.Start(wstrFilePath.c_str(), iVideoWidth, iVideoHeight, m_fps);
		if (bRet)
		{
			for (auto& image : m_images)
			{
				CDxLibMap s(image.Get());
				if (s.IsAccessible())
				{
					mfVideoEncoder.AddCpuFrame(s.pPixels, static_cast<unsigned long>(s.stride * s.height), true);
				}
				image.Reset();
			}

			mfVideoEncoder.End();
		}
	}
	break;
	case EOutputType::Pngs:
	case EOutputType::Jpgs:
	{
		size_t nSize = (std::min)(m_images.size(), m_imageNames.size());
		for (size_t i = 0; i < nSize; ++i)
		{
			auto& image = m_images[i];

			CDxLibMap s(image.Get());
			if (s.IsAccessible())
			{
				std::wstring wstrSequentialFilePath = wstrFilePath + m_imageNames[i];
				if (m_outputType == EOutputType::Pngs)
				{
					wstrSequentialFilePath += L".png";
					win_image::SaveImageAsPng(wstrSequentialFilePath.c_str(), s.width, s.height, s.stride, s.pPixels, true);
				}
				else if (m_outputType == EOutputType::Jpgs)
				{
					wstrSequentialFilePath += L".jpg";
					win_image::SaveImageAsJpg(wstrSequentialFilePath.c_str(), s.width, s.height, s.stride, s.pPixels, true);
				}
			}
			image.Reset();
		}
	}
	break;
	default:
		break;
	}

	Clear();

	return true;
}

void CDxLibRecorder::Impl::Clear()
{
	m_images.clear();
	m_imageNames.clear();

	m_outputType = EOutputType::Unknown;
	m_recorderState = EState::Idle;
}

void CDxLibRecorder::Impl::TruncateSize(int* width, int* height) const
{
	/*
	* Truncate video dimension to be multiple of 4 to prevent AMD CPU from hanging on final output
	* in spite of successful return values of mediatype setup and sample delivering.
	*/
	if (m_isAmdCpu && m_outputType == EOutputType::Video)
	{
		if (width != nullptr) *width &= 0xfffffffc;
		if (height != nullptr)*height &= 0xfffffffc;
	}
}



CDxLibRecorder::CDxLibRecorder()
{
	m_impl = new CDxLibRecorder::Impl();
}

CDxLibRecorder::~CDxLibRecorder()
{
	delete m_impl;
}

bool CDxLibRecorder::Start(EOutputType outputType, unsigned int fps)
{
	return m_impl->Start(outputType, fps);
}
CDxLibRecorder::EOutputType CDxLibRecorder::GetOutputType() const
{
	return m_impl->GetoutputType();
}
int CDxLibRecorder::GetFps() const
{
	return m_impl->GetFps();
}
CDxLibRecorder::EState CDxLibRecorder::GetState() const
{
	return m_impl->GetState();
}
bool CDxLibRecorder::CaptureFrame(const wchar_t* imageName)
{
	return m_impl->Capture(imageName);
}
bool CDxLibRecorder::CommitFrame(const int iGraphicHandle, const wchar_t* imageName)
{
	return m_impl->CommitFrame(iGraphicHandle, imageName);
}
bool CDxLibRecorder::HasFrames() const
{
	return m_impl->HasFrames();
}
bool CDxLibRecorder::End(const wchar_t* pwzFilePath)
{
	return m_impl->End(pwzFilePath);
}
