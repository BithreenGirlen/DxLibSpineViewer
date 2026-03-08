
#include <Windows.h>
#include <dwrite_1.h>
#include <atlbase.h>

#include "win_font.h"

#pragma comment (lib,"Dwrite.lib")


class CWinFont::Impl
{
public:
	Impl()
	{
		HRESULT hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWrireFactory));
		if (FAILED(hr))return;

		hr = m_pDWrireFactory->GetSystemFontCollection(&m_pDWriteFontCollection);

		int iRet = ::GetUserDefaultLocaleName(m_swzLocaleName, sizeof(m_swzLocaleName) / sizeof(wchar_t));
	}
	~Impl()
	{

	}

	const wchar_t* const GetLocaleName() const
	{
		return m_swzLocaleName;
	}

	std::wstring FindLocaleFontName(const wchar_t* pwzFontFamilyName)
	{
		UINT uiFontFamilyIndex = 0;
		BOOL iExist = FALSE;
		HRESULT hr = m_pDWriteFontCollection->FindFamilyName(pwzFontFamilyName, &uiFontFamilyIndex, &iExist);
		if (FAILED(hr) || iExist == FALSE)return std::wstring();

		CComPtr<IDWriteFontFamily> pDWriteFontFamily;
		hr = m_pDWriteFontCollection->GetFontFamily(uiFontFamilyIndex, &pDWriteFontFamily);
		if (FAILED(hr))return std::wstring();

		CComPtr<IDWriteLocalizedStrings> pDWriteLocalisedStrings;
		hr = pDWriteFontFamily->GetFamilyNames(&pDWriteLocalisedStrings);
		if (FAILED(hr))return std::wstring();

		UINT uiLocaleIndex = 0;
		iExist = FALSE;
		hr = pDWriteLocalisedStrings->FindLocaleName(m_swzLocaleName, &uiLocaleIndex, &iExist);
		if (FAILED(hr))return std::wstring();
		if (iExist == FALSE)uiLocaleIndex = 0;

		UINT32 uiStringLength = 0;
		hr = pDWriteLocalisedStrings->GetStringLength(uiLocaleIndex, &uiStringLength);
		if (FAILED(hr))return std::wstring();

		std::wstring wstrLocaleFontName(uiStringLength + 1ULL, '\0');
		hr = pDWriteLocalisedStrings->GetString(uiLocaleIndex, &wstrLocaleFontName[0], static_cast<UINT32>(wstrLocaleFontName.size()));

		return wstrLocaleFontName;
	}

	std::vector <std::wstring> GetSystemFontFamilyNames()
	{
		std::vector<std::wstring> systemFontFamilyNames;

		UINT32 uiFontFamilyCount = m_pDWriteFontCollection->GetFontFamilyCount();
		for (UINT32 i = 0; i < uiFontFamilyCount; ++i)
		{
			CComPtr<IDWriteFontFamily> pDWriteFontFamily;
			HRESULT hr = m_pDWriteFontCollection->GetFontFamily(i, &pDWriteFontFamily);
			if (FAILED(hr))continue;

			CComPtr<IDWriteLocalizedStrings> pDWriteLocalisedStrings;
			hr = pDWriteFontFamily->GetFamilyNames(&pDWriteLocalisedStrings);
			if (FAILED(hr))continue;

			UINT uiLocaleIndex = 0;
			BOOL iExist = FALSE;
			hr = pDWriteLocalisedStrings->FindLocaleName(m_swzLocaleName, &uiLocaleIndex, &iExist);
			if (FAILED(hr))continue;
			if (iExist == FALSE)uiLocaleIndex = 0;

			UINT32 uiStringLength = 0;
			hr = pDWriteLocalisedStrings->GetStringLength(uiLocaleIndex, &uiStringLength);
			if (FAILED(hr))continue;

			std::wstring wstrBuffer(uiStringLength + 1UL, L'\0');
			hr = pDWriteLocalisedStrings->GetString(uiLocaleIndex, &wstrBuffer[0], static_cast<UINT32>(wstrBuffer.size()));
			if (SUCCEEDED(hr))
			{
				systemFontFamilyNames.push_back(wstrBuffer);
			}
		}

		return systemFontFamilyNames;
	}

	std::vector<std::wstring> FindFontFilePaths(const wchar_t* pwzFontFamilyName, bool bBold, bool bItalic)
	{
		std::vector<std::wstring> fontFilePaths;
		if (pwzFontFamilyName == nullptr)return fontFilePaths;

		UINT uiFontFamilyIndex = 0;
		BOOL iExist = FALSE;
		HRESULT hr = m_pDWriteFontCollection->FindFamilyName(pwzFontFamilyName, &uiFontFamilyIndex, &iExist);
		if (FAILED(hr) || iExist == FALSE)return fontFilePaths;

		CComPtr<IDWriteFontFamily> pDWriteFontFamily;
		hr = m_pDWriteFontCollection->GetFontFamily(uiFontFamilyIndex, &pDWriteFontFamily);
		if (FAILED(hr))return fontFilePaths;

		CComPtr<IDWriteFont> pDWriteFont;
		hr = pDWriteFontFamily->GetFirstMatchingFont(
			bBold ? DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,
			bItalic ? DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL,
			&pDWriteFont
		);
		if (FAILED(hr))return fontFilePaths;

		CComPtr<IDWriteFontFace> pDWriteFontFace;
		hr = pDWriteFont->CreateFontFace(&pDWriteFontFace);
		if (FAILED(hr))return fontFilePaths;

		UINT32 uiFileCount = 0;
		hr = pDWriteFontFace->GetFiles(&uiFileCount, nullptr);
		if (FAILED(hr))return fontFilePaths;

		std::vector<CComPtr<IDWriteFontFile>> writeFontFiles(uiFileCount, nullptr);
		hr = pDWriteFontFace->GetFiles(&uiFileCount, &*writeFontFiles.data());
		if (FAILED(hr))return fontFilePaths;

		for (const auto& writeFontFile : writeFontFiles)
		{
			const void* pFontFileReferenceKey = nullptr;
			UINT32 uiKeyLength = 0;
			hr = writeFontFile->GetReferenceKey(&pFontFileReferenceKey, &uiKeyLength);
			if (FAILED(hr))continue;

			CComPtr<IDWriteFontFileLoader> pDWriteFonfFileLoader;
			hr = writeFontFile->GetLoader(&pDWriteFonfFileLoader);
			if (FAILED(hr))continue;

			CComPtr<IDWriteLocalFontFileLoader> pDWriteLocalFontFileLoader;
			hr = pDWriteFonfFileLoader->QueryInterface(&pDWriteLocalFontFileLoader);
			if (FAILED(hr))continue;

			UINT32 uiPathLength = 0;
			hr = pDWriteLocalFontFileLoader->GetFilePathLengthFromKey(pFontFileReferenceKey, uiKeyLength, &uiPathLength);
			if (FAILED(hr))continue;

			std::wstring wstrBuffer(uiPathLength + 1UL, L'\0');
			hr = pDWriteLocalFontFileLoader->GetFilePathFromKey(pFontFileReferenceKey, uiKeyLength, &wstrBuffer[0], static_cast<UINT32>(wstrBuffer.size()));
			if (SUCCEEDED(hr))
			{
				fontFilePaths.push_back(wstrBuffer.data());
			}
		}

		return fontFilePaths;
	}
private:
	CComPtr<IDWriteFactory> m_pDWrireFactory;
	CComPtr<IDWriteFontCollection> m_pDWriteFontCollection;

	wchar_t m_swzLocaleName[LOCALE_NAME_MAX_LENGTH]{};
};

CWinFont::CWinFont()
{
	m_pImpl = new CWinFont::Impl();
}

CWinFont::~CWinFont()
{
	delete m_pImpl;
}

const wchar_t* const CWinFont::GetLocaleName()
{
	return m_pImpl->GetLocaleName();
}

std::wstring CWinFont::FindLocaleFontName(const wchar_t* pwzFontFamilyName)
{
	return m_pImpl->FindLocaleFontName(pwzFontFamilyName);
}

std::vector<std::wstring> CWinFont::GetSystemFontFamilyNames()
{
	return m_pImpl->GetSystemFontFamilyNames();
}

std::vector<std::wstring> CWinFont::FindFontFilePaths(const wchar_t* pwzFontFamilyName, bool bBold, bool bItalic)
{
	return m_pImpl->FindFontFilePaths(pwzFontFamilyName, bBold, bItalic);
}
