
#include <atlbase.h>
#include <wincodec.h>

#include "win_image.h"

#pragma comment (lib,"Windowscodecs.lib")

namespace win_image
{
	enum class EFrameMetadata
	{
		None,
		Gif
	};

	bool CommitImageFrame(IWICImagingFactory *pWicImagingFactory, IWICBitmapEncoder* pWicBitmapEncoder, SImageFrame* s, bool bHasAlpha = true, EFrameMetadata frameMetaData = EFrameMetadata::None)
	{
		if (pWicImagingFactory == nullptr || pWicBitmapEncoder == nullptr || s == nullptr)return false;

		CComPtr<IWICBitmapFrameEncode> pWicBitmapFrameEncode;
		CComPtr<IPropertyBag2> pPropertyBag;
		HRESULT hr = pWicBitmapEncoder->CreateNewFrame(&pWicBitmapFrameEncode, &pPropertyBag);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->Initialize(pPropertyBag);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->SetSize(s->uiWidth, s->uiHeight);
		if (FAILED(hr))return false;

		CComPtr<IWICBitmap> pWicBitmap;
		hr = pWicImagingFactory->CreateBitmapFromMemory
		(
			s->uiWidth,
			s->uiHeight,
			bHasAlpha ? GUID_WICPixelFormat32bppRGBA : GUID_WICPixelFormat32bppRGB,
			s->iStride,
			static_cast<UINT>(s->pixels.size()),
			s->pixels.data(),
			&pWicBitmap
		);
		if (FAILED(hr))return false;

		if (frameMetaData == EFrameMetadata::Gif)
		{
			CComPtr<IWICMetadataQueryWriter> pWicMetadataQueryWriter;
			hr = pWicBitmapFrameEncode->GetMetadataQueryWriter(&pWicMetadataQueryWriter);
			if (FAILED(hr))return false;

			const auto SetDelayMetadata = [&pWicMetadataQueryWriter]()
				-> bool
				{
					PROPVARIANT sPropVariant{};
					sPropVariant.vt = VT_UI2;
					sPropVariant.uiVal = 8;
					HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/grctlext/Delay", &sPropVariant);
					return SUCCEEDED(hr);
				};

			const auto SetDisposalMetaData = [&pWicMetadataQueryWriter]()
				-> bool
				{
					PROPVARIANT sPropVariant{};
					sPropVariant.vt = VT_UI1;
					sPropVariant.bVal = 2;
					HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/grctlext/Disposal", &sPropVariant);
					return SUCCEEDED(hr);
				};
			const auto SetTransparencyFlag = [&pWicMetadataQueryWriter]()
				-> bool
				{
					PROPVARIANT sPropVariant{};
					sPropVariant.vt = VT_BOOL;
					sPropVariant.boolVal = 1;
					HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/grctlext/TransparencyFlag", &sPropVariant);
					return SUCCEEDED(hr);
				};
			const auto SetTransparentColorIndex = [&pWicMetadataQueryWriter]()
				-> bool
				{
					PROPVARIANT sPropVariant{};
					sPropVariant.vt = VT_UI1;
					sPropVariant.bVal = 255;
					HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/grctlext/TransparentColorIndex", &sPropVariant);
					return SUCCEEDED(hr);
				};

			SetDisposalMetaData() && SetTransparencyFlag() && SetDelayMetadata() && SetTransparentColorIndex();
		}

		hr = pWicBitmapFrameEncode->WriteSource(pWicBitmap, nullptr);
		if (FAILED(hr))return false;

		hr = pWicBitmapFrameEncode->Commit();

		return SUCCEEDED(hr);
	}
}

bool win_image::LoadImageToMemory(const wchar_t* wpzFilePath, SImageFrame* pImageFrame, float fScale)
{
	if (pImageFrame == nullptr)return false;

	SImageFrame* s = pImageFrame;

	CComPtr<IWICImagingFactory> pWicImageFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapDecoder> pWicBitmapDecoder;
	hr = pWicImageFactory->CreateDecoderFromFilename(wpzFilePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pWicBitmapDecoder);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapFrameDecode> pWicFrameDecode;
	hr = pWicBitmapDecoder->GetFrame(0, &pWicFrameDecode);
	if (FAILED(hr))return false;

	CComPtr<IWICFormatConverter> pWicFormatConverter;
	hr = pWicImageFactory->CreateFormatConverter(&pWicFormatConverter);
	if (FAILED(hr))return false;

	pWicFormatConverter->Initialize(pWicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
	if (FAILED(hr))return false;

	hr = pWicFormatConverter->GetSize(&s->uiWidth, &s->uiHeight);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapScaler> pWicBmpScaler;
	hr = pWicImageFactory->CreateBitmapScaler(&pWicBmpScaler);
	if (FAILED(hr))return false;

	hr = pWicBmpScaler->Initialize(pWicFormatConverter, static_cast<UINT>(s->uiWidth * fScale), static_cast<UINT>(s->uiHeight * fScale), WICBitmapInterpolationMode::WICBitmapInterpolationModeCubic);
	if (FAILED(hr))return false;
	hr = pWicBmpScaler.p->GetSize(&s->uiWidth, &s->uiHeight);

	CComPtr<IWICBitmap> pWicBitmap;
	hr = pWicImageFactory->CreateBitmapFromSource(pWicBmpScaler, WICBitmapCacheOnDemand, &pWicBitmap);
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapLock> pWicBitmapLock;
	WICRect wicRect{ 0, 0, static_cast<INT>(s->uiWidth), static_cast<INT>(s->uiHeight) };
	hr = pWicBitmap->Lock(&wicRect, WICBitmapLockRead, &pWicBitmapLock);
	if (FAILED(hr))return false;

	UINT uiStride;
	hr = pWicBitmapLock->GetStride(&uiStride);
	if (FAILED(hr))return false;

	s->iStride = static_cast<INT>(uiStride);
	s->pixels.resize(static_cast<size_t>(s->iStride * s->uiHeight));
	hr = pWicBitmap->CopyPixels(nullptr, uiStride, static_cast<UINT>(s->pixels.size()), s->pixels.data());
	if (FAILED(hr))return false;

	return true;
}

bool win_image::SaveImageAsPng(const wchar_t* wpzFilePath, SImageFrame* pImageFrame)
{
	if (pImageFrame == nullptr)return false;

	SImageFrame* s = pImageFrame;

	CComPtr<IWICImagingFactory> pWicImageFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapEncoder> pWicBitmapEncoder;
	hr = pWicImageFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &pWicBitmapEncoder);
	if (FAILED(hr))return false;

	CComPtr<IWICStream> pWicStream;
	hr = pWicImageFactory->CreateStream(&pWicStream);
	if (FAILED(hr))return false;

	hr = pWicStream->InitializeFromFilename(wpzFilePath, GENERIC_WRITE);
	if (FAILED(hr))return false;

	hr = pWicBitmapEncoder->Initialize(pWicStream, WICBitmapEncoderCacheOption::WICBitmapEncoderNoCache);
	if (FAILED(hr))return false;

	bool bRet = CommitImageFrame(pWicImageFactory, pWicBitmapEncoder, pImageFrame);
	if (!bRet)return false;

	hr = pWicBitmapEncoder->Commit();

	return SUCCEEDED(hr);
}

bool win_image::SaveImagesAsGif(const wchar_t* wpzFilePath, const std::vector<SImageFrame>& imageFrames)
{
	if (imageFrames.empty())return false;

	CComPtr<IWICImagingFactory> pWicImageFactory;
	HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicImageFactory));
	if (FAILED(hr))return false;

	CComPtr<IWICBitmapEncoder> pWicBitmapEncoder;
	hr = pWicImageFactory->CreateEncoder(GUID_ContainerFormatGif, nullptr, &pWicBitmapEncoder);
	if (FAILED(hr))return false;

	CComPtr<IWICStream> pWicStream;
	hr = pWicImageFactory->CreateStream(&pWicStream);
	if (FAILED(hr))return false;

	hr = pWicStream->InitializeFromFilename(wpzFilePath, GENERIC_WRITE);
	if (FAILED(hr))return false;

	hr = pWicBitmapEncoder->Initialize(pWicStream, WICBitmapEncoderCacheOption::WICBitmapEncoderNoCache);
	if (FAILED(hr))return false;

	CComPtr<IWICMetadataQueryWriter> pWicMetadataQueryWriter;
	hr = pWicBitmapEncoder->GetMetadataQueryWriter(&pWicMetadataQueryWriter);
	if (FAILED(hr))return false;

	const auto SetApplicationMetadata = [&pWicMetadataQueryWriter]()
		-> bool
		{
			PROPVARIANT sPropVariant{};
			sPropVariant.vt = VT_UI1 | VT_VECTOR;
			char szName[] = "NETSCAPE2.0";
			sPropVariant.cac.cElems = sizeof(szName) - 1;
			sPropVariant.cac.pElems = szName;
			HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/appext/application", &sPropVariant);
			return SUCCEEDED(hr);
		};
	const auto SetDataMetadata = [&pWicMetadataQueryWriter]()
		-> bool
		{
			PROPVARIANT sPropVariant{};
			sPropVariant.vt = VT_UI1 | VT_VECTOR;
			char szLoopCount[] = {0x03, 0x01, 0x00, 0x00, 0x00};
			sPropVariant.cac.cElems = sizeof(szLoopCount);
			sPropVariant.cac.pElems = szLoopCount;
			HRESULT hr = pWicMetadataQueryWriter->SetMetadataByName(L"/appext/data", &sPropVariant);
			return SUCCEEDED(hr);
		};
	bool bRet = SetApplicationMetadata() && SetDataMetadata();
	if (!bRet)return false;

	for (const auto& imageFrame : imageFrames)
	{
		bRet = CommitImageFrame(pWicImageFactory, pWicBitmapEncoder, const_cast<SImageFrame*>(&imageFrame), true, EFrameMetadata::Gif);
		if (!bRet)break;
	}

	hr = pWicBitmapEncoder->Commit();

	return SUCCEEDED(hr);
}
