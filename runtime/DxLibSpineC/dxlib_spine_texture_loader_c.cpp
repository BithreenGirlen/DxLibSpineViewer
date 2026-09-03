
#include "dxlib_spine_texture_loader_c.h"

#include <spine/extension.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#if defined(SPINE_RUNTIME_DLL_BUILD)
#include "../dxlib_spine_dll.h"
#endif

namespace dxlib_spine_texture_loader_c
{
#if	defined(_WIN32) && defined(_UNICODE)
	/* 
	 * The capacity of fixed-size buffer for filepath encoding in DxLib is in most cases 512 + 512 + 512 though,
	 * sometimes smaller capacity are allocated like 512.
	*/
	static constexpr size_t kMaxPathLength = 1024;

	static bool g_toConvertToPma = false;

	/// @brief 現在DxLibに設定されているコードページを元に、固定バッファ上でwchar_t文字列をchar文字列に変換
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

	/// @brief 現在DxLibに設定されているコードページを元に、固定バッファ上でchar文字列をwchar_t文字列に変換
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

/* ==================== Implementations for <spine/extension.h> ==================== */

void _spAtlasPage_createTexture(spAtlasPage* pAtlasPage, const char* path)
{
#if defined(SPINE_40) || defined(SPINE_41) || defined (SPINE_42)
	/* true: -1, false: 0 */
	bool toConvertToPma = (pAtlasPage->pma == 0) && g_toConvertToPma;
	if (toConvertToPma)
	{
		pAtlasPage->pma = -1;
	}
#else
	bool toConvertToPma = dxlib_spine_texture_loader_c::g_toConvertToPma;
#endif
	DxLib::SetUsePremulAlphaConvertLoad(toConvertToPma ? TRUE : FALSE);

#if	defined(_WIN32) && defined(_UNICODE)
	wchar_t wcharPathBuffer[dxlib_spine_texture_loader_c::kMaxPathLength]{};
	int nWritten = dxlib_spine_texture_loader_c::WidenInBuffer(path, wcharPathBuffer);
	int iDxLibTexture = DxLib::LoadGraph(wcharPathBuffer);
#else
	int iDxLibTexture = DxLib::LoadGraph(path);
#endif
	if (iDxLibTexture == -1)return;

	void* p = reinterpret_cast<void*>(static_cast<unsigned long long>(iDxLibTexture));

	pAtlasPage->rendererObject = p;
}

void _spAtlasPage_disposeTexture(spAtlasPage* pAtlasPage)
{
	DxLib::DeleteGraph(static_cast<int>(reinterpret_cast<unsigned long long>(pAtlasPage->rendererObject)));
}

char* _spUtil_readFile(const char* path, int* length)
{
	return _spReadFile(path, length);
}

/* ==================== end of implementations for <spine/extension.h> ==================== */


void SpineTextureLoader_enableConversionToPma(bool toEnable)
{
	dxlib_spine_texture_loader_c::g_toConvertToPma = toEnable;
}

bool SpineTextureLoader_isConversionToPmaEnabled()
{
	return dxlib_spine_texture_loader_c::g_toConvertToPma;
}
