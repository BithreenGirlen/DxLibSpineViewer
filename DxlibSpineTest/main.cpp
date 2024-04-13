
#include "framework.h"
#include "main_window.h"

#ifdef _DEBUG
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
#ifdef _DEBUG
    SSpineDebug spineDebug;
#endif

    int iRet = 0;
    CMainWindow sWindow;
    bool bRet = sWindow.Create(hInstance, L"Dxlib spine");
    if (bRet)
    {
        ::ShowWindow(sWindow.GetHwnd(), nCmdShow);
        iRet = sWindow.MessageLoop();
    }

    return iRet;
}