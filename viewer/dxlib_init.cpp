
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

		iRet = DxLib::SetUserWindowMessageProcessDXLibFlag(FALSE);
		if (iRet == -1)return;
	}

	iRet = DxLib::ChangeWindowMode(TRUE);
	if (iRet == -1)return;

	iRet = DxLib::SetMultiThreadFlag(TRUE);
	if (iRet == -1)return;
#endif
	iRet = DxLib::SetChangeScreenModeGraphicsSystemResetFlag(FALSE);
	if (iRet == -1)return;

	iRet = DxLib::SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);
	if (iRet == -1)return;

	iDxLibInitialised = DxLib::DxLib_Init();
	if (iDxLibInitialised == -1)return;

	/* Not fatal even if failed. */
	DxLib::SetDrawScreen(DX_SCREEN_BACK);
	DxLib::SetDrawMode(DX_DRAWMODE_BILINEAR);
	DxLib::SetTextureAddressMode(DX_TEXADDRESS_WRAP);
}

SDxLibInit::~SDxLibInit()
{
	if (iDxLibInitialised != -1)
	{
		DxLib::DxLib_End();
	}
}
