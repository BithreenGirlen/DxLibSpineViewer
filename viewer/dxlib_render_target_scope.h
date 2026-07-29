#ifndef DXLIB_RENDER_TARGET_SCOPE_H_
#define DXLIB_RENDER_TARGET_SCOPE_H_

/// @brief 描画先変更
struct DxLibRenderTargetScope
{
	DxLibRenderTargetScope(int iGraphicHandle, bool toClear = true);
	~DxLibRenderTargetScope();

	int iPreviousRenderTarget = -1;
};

#endif // !DXLIB_RENDER_TARGET_SCOPE_H_
