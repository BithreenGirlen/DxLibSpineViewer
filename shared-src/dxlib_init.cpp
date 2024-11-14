
#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#include "dxlib_init.h"

SDxLibInit::SDxLibInit(void* pWindowHandle)
{
	int iRet = -1;
	iRet = DxLib::SetOutApplicationLogValidFlag(FALSE);
	if (iRet == -1)return;

#ifdef _WIN32
	HWND hWnd = static_cast<HWND>(pWindowHandle);
	if (hWnd != nullptr)
	{
		iRet = DxLib::SetUserWindow(hWnd);
		if (iRet == -1)return;
	}
	iRet = DxLib::SetUserWindowMessageProcessDXLibFlag(hWnd != nullptr ? FALSE : TRUE);
	if (iRet == -1)return;

	iRet = DxLib::SetChangeScreenModeGraphicsSystemResetFlag(hWnd != nullptr ? FALSE : TRUE);
	if (iRet == -1)return;

	iRet = DxLib::ChangeWindowMode(TRUE);
	if (iRet == -1)return;
#endif
	iRet = DxLib::SetMultiThreadFlag(TRUE);
	if (iRet == -1)return;

	iRet = DxLib::SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);
	if (iRet == -1)return;

	iDxLibInitialised = DxLib::DxLib_Init();

	iRet = DxLib::SetDrawScreen(DX_SCREEN_BACK);
	if (iRet == -1)
	{
		DxLib::DxLib_End();
		iDxLibInitialised = -1;
		return;
	}

	iRet = DxLib::SetDrawMode(DX_DRAWMODE_BILINEAR);
	if (iRet == -1)return;

	iRet = DxLib::SetTextureAddressMode(DX_TEXADDRESS_WRAP);
	if (iRet == -1)return;
}

SDxLibInit::~SDxLibInit()
{
	if (iDxLibInitialised != -1)
	{
		DxLib::DxLib_End();
	}
}
