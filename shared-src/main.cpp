
#include "framework.h"
#include "main_window.h"
#include "dxlib_init.h"

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
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
#if defined(_DEBUG) && defined(DXLIB_SPINE_CPP)
    static SSpineDebug spineDebug;
#endif

    int iRet = 0;
    CMainWindow sWindow;
    bool bRet = sWindow.Create(hInstance, L"Dxlib spine");
    if (bRet)
    {
        SDxLibInit sDxLibInit(sWindow.GetHwnd());
        if (sDxLibInit.iDxLibInitialised == -1)
        {
            ::MessageBoxW(nullptr, L"Failed to setup DxLib.", L"Error", MB_ICONERROR);
            return iRet;
        }

        ::ShowWindow(sWindow.GetHwnd(), nCmdShow);
        iRet = sWindow.MessageLoop();
    }

    return iRet;
}