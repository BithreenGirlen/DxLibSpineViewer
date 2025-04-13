
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
using DxLibSoftImageHandle = DxLibHandle<&DxLib::DeleteSoftImage>;


class CDxLibRecorder::Impl
{
public:
	Impl()
	{
		m_bAmd = cpu::IsAmd();
	}
	~Impl()
	{
		Clear();
	}

	bool Start(bool bToBeVideo)
	{
		Clear();
		m_bToBeVideo = bToBeVideo;

		return true;
	}

	bool Capture(const wchar_t* pwzFilePath)
	{
		int iGraphWidth = 0;
		int iGraphHeight = 0;
		DxLib::GetScreenState(&iGraphWidth, &iGraphHeight, nullptr);

		/*
		* Truncate video dimension to be multiple of 4 to prevent AMD CPU from hanging on final output
		* in spite of successful return values of mediatype setup and sample delivering.
		*/
		if (m_bAmd && m_bToBeVideo)
		{
			iGraphWidth &= 0xfffffffc;
			iGraphHeight &= 0xfffffffc;
		}

		DxLibImageHandle dxLibGraphHandle(DxLib::MakeGraph(iGraphWidth, iGraphHeight));
		if (dxLibGraphHandle.Get() == -1)return false;

		int iRet = DxLib::GetDrawScreenGraph(0, 0, iGraphWidth, iGraphHeight, dxLibGraphHandle.Get());
		if (iRet == -1)return false;

		m_images.push_back(std::move(dxLibGraphHandle));
		if (pwzFilePath != nullptr)m_imageFilePaths.push_back(pwzFilePath);

		return true;
	}

	bool End(EOutputType eOutputType, const wchar_t* pwzFilePath)
	{
		if (pwzFilePath == nullptr)
		{
			Clear();
			return false;
		}

		if (eOutputType == EOutputType::kPngs)
		{
			for (size_t i = 0; i < m_images.size() && i < m_imageFilePaths.size(); ++i)
			{
				auto& image = m_images[i];

				SImageFrame s{};
				GetTexturePixels(image.Get(), &s);
				image.Reset();
				if (!s.pixels.empty())
				{
					std::wstring wstrFilePath = pwzFilePath + m_imageFilePaths[i] + L".png";
					win_image::SaveImageAsPng(wstrFilePath.c_str(), &s);
				}
			}
		}
		else if (eOutputType == EOutputType::kGif)
		{
			win_image::CWicGifEncoder sWicGifEncoder;
			bool bRet = sWicGifEncoder.Initialise(pwzFilePath);
			if (bRet)
			{
				for (auto& image : m_images)
				{
					SImageFrame s{};
					GetTexturePixels(image.Get(), &s);
					image.Reset();
					if(!s.pixels.empty())sWicGifEncoder.CommitFrame(&s);
				}
				sWicGifEncoder.End();
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

			bool bRet = sMfVideoEncoder.Start(pwzFilePath, iVideoWidth, iVideoHeight, 60);
			if (bRet)
			{
				for (auto& image : m_images)
				{
					SImageFrame s{};
					GetTexturePixels(image.Get(), &s);
					image.Reset();
					if (!s.pixels.empty())sMfVideoEncoder.AddCpuFrame(s.pixels.data(), static_cast<unsigned long>(s.pixels.size()), true);
				}

				sMfVideoEncoder.End();
			}
		}

		Clear();

		return true;
	}
private:
	std::vector<DxLibImageHandle> m_images;
	std::vector<std::wstring> m_imageFilePaths;

	bool m_bAmd = false;
	bool m_bToBeVideo = false;

	void Clear()
	{
		m_images.clear();
		m_imageFilePaths.clear();

		m_bToBeVideo = false;
	}

	void GetTexturePixels(int iImageHandle, SImageFrame* s)
	{
		if (s == nullptr)return;

		int iGraphWidth = 0;
		int iGraphsHeight = 0;
		int iRet = DxLib::GetGraphSize(iImageHandle, &iGraphWidth, &iGraphsHeight);
		if (iRet == -1)return;

		int iPitch = 0;
		void* pPixels = nullptr;
		DxLib::COLORDATA* pFormat = nullptr;
		iRet = DxLib::GraphLock(iImageHandle, &iPitch, &pPixels, &pFormat);
		if (iRet == -1)return;

		s->iStride = iPitch;
		s->uiWidth = iGraphWidth;
		s->uiHeight = iGraphsHeight;

		size_t nSize = static_cast<size_t>(iPitch * iGraphsHeight);
		s->pixels.resize(nSize);
		memcpy(&s->pixels[0], pPixels, nSize);

		DxLib::GraphUnLock(iImageHandle);
	}
};

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
