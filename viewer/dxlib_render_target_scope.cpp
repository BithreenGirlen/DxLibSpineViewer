

#include "dxlib_render_target_scope.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

DxLibRenderTargetScope::DxLibRenderTargetScope(int iGraphicHandle, bool toClear)
	:iPreviousRenderTarget(DxLib::GetDrawScreen())
{
	DxLib::SetDrawScreen(iGraphicHandle);
	if (toClear)DxLib::ClearDrawScreen();
}

DxLibRenderTargetScope::~DxLibRenderTargetScope()
{
	DxLib::SetDrawScreen(iPreviousRenderTarget == -1 ? DX_SCREEN_BACK : iPreviousRenderTarget);
}
