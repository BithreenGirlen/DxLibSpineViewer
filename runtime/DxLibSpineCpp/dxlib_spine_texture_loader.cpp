
#include "dxlib_spine_texture_loader.h"

#include <spine/Atlas.h> /* spine::AtlasPage */
#include <spine/Extension.h>

namespace dxlib_spine_texture_loader
{
#if defined(_WIN32) && defined(_UNICODE)
	/*
	 * The capacity of fixed-size buffer for filepath encoding in DxLib is in most cases 512 + 512 + 512 though,
	 * sometimes smaller capacity are are allocated like 512.
	*/
	static constexpr size_t kMaxPathLength = 1024;

	/// @brief 現在DxLibに設定されているコードページを元に、固定長バッファ上でwchar_t文字列をchar文字列に変換
	/// @param wstr 変換元の文字列
	/// @param dst 変換先のバッファ
	/// @param dstSize 変換先の大きさ。既定引数の場合、変換前の大きさ超過確認を行わない。
	/// @return 実際に書き込まれた長さ
	static int NarrowInBuffer(const wchar_t* wstr, char* dst, int dstSize = -1)
	{
		int iCharCode = DxLib::GetUseCharCodeFormat();
		int iWcharCode = DxLib::Get_wchar_t_CharCodeFormat();

		if (dstSize != -1)
		{
			int iRet = DxLib::ConvertStringCharCodeFormat(iWcharCode, wstr, iCharCode, nullptr);
			if (iRet < 1)return 0;
			if (iRet >= dstSize)return 0;
		}

		int iLength = DxLib::ConvertStringCharCodeFormat(iWcharCode, wstr, iCharCode, dst);
		--iLength;

		return iLength;
	}

	/// @brief 現在DxLibに設定されているコードページを元に、固定長バッファ上でchar文字列をwchar_t文字列に変換
	/// @param str 変換元の文字列
	/// @param dst 変換先のバッファ
	/// @param dstSize 変換先の大きさ。既定引数の場合、変換前の大きさ超過確認を行わない。
	/// @return 実際に書き込まれた長さ
	static int WidenInBuffer(const char* str, wchar_t* dst, int dstSize = -1)
	{
		int iCharCode = DxLib::GetUseCharCodeFormat();
		int iWcharCode = DxLib::Get_wchar_t_CharCodeFormat();

		if (dstSize != -1)
		{
			int iRet = DxLib::ConvertStringCharCodeFormat(iCharCode, str, iWcharCode, nullptr);
			if (iRet < sizeof(wchar_t))return 0;
			if (iRet >= dstSize * sizeof(wchar_t))return 0;
		}

		int iLength = DxLib::ConvertStringCharCodeFormat(iCharCode, str, iWcharCode, dst);
		iLength = iLength / sizeof(wchar_t) - 1;

		return iLength;
	}
#endif  /* defined(_WIN32) && defined(_UNICODE) */
}

spine::SpineExtension* spine::getDefaultExtension()
{
	static spine::DefaultSpineExtension s_defaultSpineExtension;

	return &s_defaultSpineExtension;
}

void CDxLibTextureLoader::load(spine::AtlasPage& atlasPage, const spine::String& textureFilePath)
{
#if defined(SPINE_40) || defined(SPINE_41) || defined (SPINE_42)
	bool toConvertToPma = !atlasPage.pma && m_toConvertToPma;
	if (toConvertToPma)
	{
		atlasPage.pma = true;
	}
#else
	bool toConvertToPma = m_toConvertToPma;
#endif
	/*
	* Another way to realise both PMA and callback is to utilise software image haldle or BASEIMAGE like:
	* DxLib::LoadARGB8ColorSoftImage()
	* DxLib::ConvertPremulAlphaSoftImage()
	* DxLib::CreateGraphFromSoftImage()
	* DxLib::DeleteSoftImage();
	* But this kind of way would results in more memory consumption than the way here adopted because of
	* the difference of the size of data which is stored when an image is transferred to GPU and to be used in the event of device lost.
	* When loaded via software image handle, the data would be the uncompressed image,
	* and compressed image when directly loaded via file.
	*/
	DxLib::SetUsePremulAlphaConvertLoad(toConvertToPma ? TRUE : FALSE);
	
	int iDxLibTexture = -1;
	if (m_pTextureLoadCallback != nullptr)
	{
		m_pTextureLoadCallback(m_pCallbackUserDatum, textureFilePath.buffer(), textureFilePath.length(), &iDxLibTexture);
	}

	if (iDxLibTexture == -1)
	{
#if defined(_WIN32) && defined(_UNICODE)
		wchar_t wcharPathBuffer[dxlib_spine_texture_loader::kMaxPathLength]{};
		int nWritten = dxlib_spine_texture_loader::WidenInBuffer(textureFilePath.buffer(), wcharPathBuffer);
		iDxLibTexture = DxLib::LoadGraph(wcharPathBuffer);
#else
		iDxLibTexture = DxLib::LoadGraph(textureFilePath.buffer());
#endif
		if (iDxLibTexture == -1)return;
	}

	void* p = reinterpret_cast<void*>(static_cast<unsigned long long>(iDxLibTexture));
#if defined (SPINE_41) || defined (SPINE_42)
	atlasPage.texture = p;
#else
	atlasPage.setRendererObject(p);
#endif
}

void CDxLibTextureLoader::unload(void* texture)
{
	DxLib::DeleteGraph(static_cast<int>(reinterpret_cast<unsigned long long>(texture)));
}

void CDxLibTextureLoader::enableConversionToPma(bool toEnable)
{
	m_toConvertToPma = toEnable;
}

bool CDxLibTextureLoader::isConversionToPmaEnabled() const noexcept
{
	return m_toConvertToPma;
}

void CDxLibTextureLoader::setTextureLoadCallback(void(*pFunc)(void* pUserDatum, const char* textureFilePath, size_t filePathLength, void* pOutImage), void* pUserDatum)
{
	m_pTextureLoadCallback = pFunc;
	m_pCallbackUserDatum = pUserDatum;
}
