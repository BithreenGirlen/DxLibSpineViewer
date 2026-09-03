#ifndef DXLIB_SPINE_TEXTURE_LOADER_H_
#define DXLIB_SPINE_TEXTURE_LOADER_H_

#include <spine/TextureLoader.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#if defined(SPINE_RUNTIME_DLL_BUILD)
#include "../dxlib_spine_dll.h"
#endif

class CDxLibTextureLoader : public spine::TextureLoader
{
public:
	void load(spine::AtlasPage& atlasPage, const spine::String& textureFilePath) override;
	void unload(void* texture) override;

	/// @brief 読み込み時に、画像を乗算済みαにするか否か。既定では無効
	/// @remark Spine 4.0以降ではAtlasPage::pmaをみて合理性を判断します
	void enableConversionToPma(bool toEnable);
	bool isConversionToPmaEnabled() const noexcept;
	/// @brief ファイル読み込み時のコールバック関数を登録。*pOutImageはSIHandleに型変換すること
	void setTextureLoadCallback(void (*pFunc)(void* pUserDatum, const char* textureFilePath, size_t filePathLength, void* pOutImage), void* pUserDatum);
private:
	bool m_toConvertToPma = false;

	void (*m_pTextureLoadCallback)(void* pUserDatum, const char* textureFilePath, size_t filePathLength, void* pOutImage) = nullptr;
	void* m_pCallbackUserDatum = nullptr;
};

#endif // !DXLIB_SPINE_TEXTURE_LOADER_H_
