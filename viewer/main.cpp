

#include <winsdkver.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

#include <locale.h>

#include "main_window.h"
#include "dxlib_init.h"
#include "dxlib-imgui/dxlib_imgui.h"

/* CommCtrl */
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

extern "C"
{
	_declspec(selectany) _declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	_declspec(selectany) _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	::setlocale(LC_ALL, ".utf8");
	int iRet = 0;

	CMainWindow mainWindow;
	bool bRet = mainWindow.create(hInstance, L"DxLibSpineViewer");
	if (bRet)
	{
		SDxLibInit dxLibInit(mainWindow.getHwnd());
		if (dxLibInit.iDxLibInitialised == -1)
		{
			::MessageBoxW(nullptr, L"Failed to setup DxLib.", L"Error", MB_ICONERROR);
			return iRet;
		}

		CDxLibImgui dxLibImgui("C:\\Windows\\Fonts\\yumin.ttf");
		if (!dxLibImgui.hasBeenInitialised())
		{
			::MessageBoxW(nullptr, L"Failed to setup ImGui.", L"Error", MB_ICONERROR);
			return iRet;
		}

		::ShowWindow(mainWindow.getHwnd(), nCmdShow);
		iRet = mainWindow.messageLoop();
	}

	return iRet;
}