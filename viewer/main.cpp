
#include <locale.h>

#include "framework.h"
#include "main_window.h"
#include "dxlib_init.h"
#include "dxlib-imgui/dxlib_imgui.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	::setlocale(LC_ALL, ".utf8");
	int iRet = 0;

	CMainWindow mainWindow;
	bool bRet = mainWindow.Create(hInstance, L"DxLibSpinePlayer");
	if (bRet)
	{
		SDxLibInit dxLibInit(mainWindow.GetHwnd());
		if (dxLibInit.iDxLibInitialised == -1)
		{
			::MessageBoxW(nullptr, L"Failed to setup DxLib.", L"Error", MB_ICONERROR);
			return iRet;
		}

		CDxLibImgui dxLibImgui("C:\\Windows\\Fonts\\yumin.ttf");
		if (!dxLibImgui.HasBeenInitialised())
		{
			::MessageBoxW(nullptr, L"Failed to setup ImGui.", L"Error", MB_ICONERROR);
			return iRet;
		}

		::ShowWindow(mainWindow.GetHwnd(), nCmdShow);
		iRet = mainWindow.MessageLoop();
	}

	return iRet;
}