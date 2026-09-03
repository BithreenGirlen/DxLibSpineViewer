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
	void load(spine::AtlasPage& page, const spine::String& path) override;
	void unload(void* texture) override;

	/// @brief 読み込み時に、画像を乗算済みαにするか否か。既定では無効
	/// @remark Spine 4.0以降ではAtlasPage::pmaをみて合理性を判断します
	void enableConversionToPma(bool toEnable);
	bool isConversionToPmaEnabled() const noexcept;
private:
	bool m_toConvertToPma = false;
};

#endif // !DXLIB_SPINE_TEXTURE_LOADER_H_
