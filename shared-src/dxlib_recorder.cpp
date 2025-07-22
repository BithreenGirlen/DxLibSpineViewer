
#include <string>
#include <vector>

#include "dxlib_handle.h"
#include "dxlib_recorder.h"
#include "dxlib_clock.h"
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
	EState GetState() const { return m_recorderState; }
	EOutputType GetoutputType() const { return m_outputType; }

	bool HasTimePassed();
	bool Capture(const wchar_t* filePath);

	bool End(const wchar_t* filePath);
private:
	std::vector<DxLibImageHandle> m_images;
	std::vector<std::wstring> m_imageFilePaths;

	bool m_isAmdCpu = false;

	EOutputType m_outputType = EOutputType::Video;
	EState m_recorderState = EState::Idle;

	CDxLibClock m_clock;
	int m_fps = kDefaultFps;

	void Clear();
};

bool CDxLibRecorder::Impl::Start(EOutputType outputType, unsigned int fps)
{
	Clear();

	m_fps = fps;

	m_outputType = outputType;
	m_recorderState = EState::UnderRecording;

	m_clock.Restart();

	return true;
}

bool CDxLibRecorder::Impl::HasTimePassed()
{
	float elapsedTime = m_clock.GetElapsedTime();
	float timeToWait = 1.f / m_fps;

	return ::isgreaterequal(elapsedTime, timeToWait);
}

bool CDxLibRecorder::Impl::Capture(const wchar_t* filePath)
{
	if (m_recorderState != EState::Idle && m_recorderState != EState::InitialisingVideoStream)
	{
		if (HasTimePassed())
		{
			int iGraphWidth = 0;
			int iGraphHeight = 0;
			DxLib::GetScreenState(&iGraphWidth, &iGraphHeight, nullptr);

			/*
			* Truncate video dimension to be multiple of 4 to prevent AMD CPU from hanging on final output
			* in spite of successful return values of mediatype setup and sample delivering.
			*/
			if (m_isAmdCpu && m_outputType == EOutputType::Video)
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

			m_clock.Restart();

			return true;
		}
	}

	return false;
}

bool CDxLibRecorder::Impl::End(const wchar_t* filePath)
{
	if (filePath == nullptr)
	{
		Clear();
		return false;
	}

	if (m_outputType == EOutputType::Pngs)
	{
		for (size_t i = 0; i < m_images.size() && i < m_imageFilePaths.size(); ++i)
		{
			auto& image = m_images[i];

			CDxLibMap s(image.Get());
			if (s.IsAccessible())
			{
				std::wstring wstrFilePath = filePath + m_imageFilePaths[i] + L".png";
				win_image::SaveImageAsPng(wstrFilePath.c_str(), s.width, s.height, s.stride, s.pPixels, true);
			}
			image.Reset();
		}
	}
	else if (m_outputType == EOutputType::Gif)
	{
		win_image::CWicGifEncoder wicGifEncoder;
		bool bRet = wicGifEncoder.Initialise(filePath);
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
	else if (m_outputType == EOutputType::Video)
	{
		CMfVideoEncoder mfVideoEncoder;
		int iVideoWidth = 0;
		int iVideoHeight = 0;
		if (!m_images.empty())
		{
			DxLib::GetGraphSize(m_images[0].Get(), &iVideoWidth, &iVideoHeight);
		}

		/* Initialising input media types takes time. In the meantime, pause rendering. */
		m_recorderState = EState::InitialisingVideoStream;
		bool bRet = mfVideoEncoder.Start(filePath, iVideoWidth, iVideoHeight, m_fps);
		if (bRet)
		{
			for (auto& image : m_images)
			{
				CDxLibMap s(image.Get());
				mfVideoEncoder.AddCpuFrame(s.pPixels, static_cast<unsigned long>(s.stride * s.height), true);
				image.Reset();
			}

			mfVideoEncoder.End();
		}
	}

	Clear();

	return true;
}

void CDxLibRecorder::Impl::Clear()
{
	m_images.clear();
	m_imageFilePaths.clear();

	m_outputType = EOutputType::Video;
	m_recorderState = EState::Idle;
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
CDxLibRecorder::EState CDxLibRecorder::GetState() const
{
	return m_impl->GetState();
}
bool CDxLibRecorder::HasTimePassed() const
{
	return m_impl->HasTimePassed();
}
bool CDxLibRecorder::CaptureFrame(const wchar_t* pwzFileName)
{
	return m_impl->Capture(pwzFileName);
}
bool CDxLibRecorder::End(const wchar_t* pwzFilePath)
{
	return m_impl->End(pwzFilePath);
}
