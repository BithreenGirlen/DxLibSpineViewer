
#include <locale.h>

#include "framework.h"
#include "main_window.h"
#include "dxlib_init.h"

#if defined(DXLIB_SPINE_CPP)
static constexpr const wchar_t g_swzDefaultWindowTitle[] = L"DxLib spine-cpp";
	#if defined(_DEBUG)
		#pragma comment (lib, "spine-cpp-d.lib")
	#else 
		#pragma comment (lib, "spine-cpp.lib")
	#endif
#elif defined(DXLIB_SPINE_C)
static constexpr const wchar_t g_swzDefaultWindowTitle[] = L"DxLib spine-c";
	#if defined(_DEBUG)
		#pragma comment (lib, "spine-c-d.lib")
	#else 
		#pragma comment (lib, "spine-c.lib")
	#endif
#endif

#if defined(_DEBUG) && defined(DXLIB_SPINE_CPP)
#include <spine/Debug.h>
struct SSpineDebug
{
	SSpineDebug()
	{
		spine::SpineExtension::setInstance(&dbgExtension);
	}
	~SSpineDebug()
	{
		dbgExtension.reportLeaks();
	}
	spine::DebugExtension dbgExtension = spine::SpineExtension::getInstance();
};
static SSpineDebug g_spineDebug;
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	::setlocale(LC_ALL, ".utf8");
	int iRet = 0;

	CMainWindow mainWindow;
	bool bRet = mainWindow.Create(hInstance, g_swzDefaultWindowTitle);
	if (bRet)
	{
		SDxLibInit dxLibInit(mainWindow.GetHwnd());
		if (dxLibInit.iDxLibInitialised == -1)
		{
			::MessageBoxW(nullptr, L"Failed to setup DxLib.", L"Error", MB_ICONERROR);
			return iRet;
		}

		::ShowWindow(mainWindow.GetHwnd(), nCmdShow);
		iRet = mainWindow.MessageLoop();
	}

	return iRet;
}