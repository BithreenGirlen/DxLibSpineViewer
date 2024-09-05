
#include <Windows.h>
#include <CommCtrl.h>


#include "main_window.h"
#include "win_filesystem.h"
#include "win_dialogue.h"
#include "win_text.h"

CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    //wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_APP));
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
    //wcex.lpszMenuName = MAKEINTRESOURCE(IDI_ICON_APP);
    wcex.lpszClassName = m_swzClassName;
    //wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_APP));

    if (::RegisterClassExW(&wcex))
    {
        m_hInstance = hInstance;

        m_hWnd = ::CreateWindowW(m_swzClassName, pwzWindowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, nullptr, nullptr, hInstance, this);
        if (m_hWnd != nullptr)
        {
            return true;
        }
        else
        {
            std::wstring wstrMessage = L"CreateWindowExW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
        }
    }
    else
    {
        std::wstring wstrMessage = L"RegisterClassW failed; code: " + std::to_wstring(::GetLastError());
        ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    }

    return false;
}

int CMainWindow::MessageLoop()
{
    MSG msg;

    for (;;)
    {
        BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
        if (bRet > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        else if (bRet == 0)
        {
            /*ループ終了*/
            return static_cast<int>(msg.wParam);
        }
        else
        {
            /*ループ異常*/
            std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
            return -1;
        }
    }
    return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pThis != nullptr)
    {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return OnCreate(hWnd);
    case WM_DESTROY:
        return OnDestroy();
    case WM_CLOSE:
        return OnClose();
    case WM_PAINT:
        return OnPaint();
    case WM_ERASEBKGND:
        return 1;
    case WM_KEYUP:
        return OnKeyUp(wParam, lParam);
    case WM_COMMAND:
        return OnCommand(wParam, lParam);
    case WM_MOUSEWHEEL:
        return OnMouseWheel(wParam, lParam);
    case WM_LBUTTONDOWN:
        return OnLButtonDown(wParam, lParam);
    case WM_LBUTTONUP:
        return OnLButtonUp(wParam, lParam);
    case WM_MBUTTONUP:
        return OnMButtonUp(wParam, lParam);
    default:
        break;
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    InitialiseMenuBar();

    bool bRet = m_DxLibSpinePlayer.SetupDxLib(m_hWnd);
    if (!bRet)
    {
        ::MessageBoxW(nullptr, L"Failed to setup DxLib.", L"Error", MB_ICONERROR);
    }

    UpdateDrawingInterval();

    return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
    ::PostQuitMessage(0);

    return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_swzClassName, m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);

    m_DxLibSpinePlayer.Redraw(m_fDelta);

    ::EndPaint(m_hWnd, &ps);

    return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
    return 0;
}
/*WM_KEYUP*/
LRESULT CMainWindow::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case VK_ESCAPE:
        ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        break;
    case VK_UP:
        KeyUpOnForeFolder();
        break;
    case VK_DOWN:
        KeyUpOnNextFolder();
        break;
    case 0x41: // Key A
        m_DxLibSpinePlayer.SwitchPma();
        break;
    case 0x42: // Key B
        m_DxLibSpinePlayer.SwitchBlendModeAdoption();
        break;
    case 0x5a: // Key Z
        m_DxLibSpinePlayer.SwitchDepthBufferValidity();
        break;
    default:
        break;
    }
    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
    int wmId = LOWORD(wParam);
    int iControlWnd = LOWORD(lParam);
    if (iControlWnd == 0)
    {
        /*Menus*/
        switch (wmId)
        {
        case Menu::kOpenFolder:
            MenuOnOpenFolder();
            break;
        case Menu::kFileSetting:
            MenuOnFileSetting();
            break;
        case Menu::kSelectFiles:
            MenuOnSelectFiles();
            break;
        case Menu::kSeeThroughImage:
            MenuOnSeeThroughImage();
            break;
        case Menu::kSkeletonSetting:
            MenuOnSkeletonSetting();
            break;
        }
    }
    else
    {
        /*Controls*/
    }

    return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
    WORD wKey = LOWORD(wParam);

    if (wKey == 0)
    {
        m_DxLibSpinePlayer.RescaleSkeleton(iScroll > 0);
    }

    if (wKey == MK_LBUTTON)
    {
        m_DxLibSpinePlayer.RescaleTime(iScroll > 0);
        m_bSpeedHavingChanged = true;
    }

    if (wKey == MK_RBUTTON)
    {
        m_DxLibSpinePlayer.ShiftSkin();
    }

    return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    ::GetCursorPos(&m_CursorPos);

    m_bLeftDowned = true;

    return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (m_bSpeedHavingChanged)
    {
        m_bSpeedHavingChanged = false;
        return 0;
    }

    WORD usKey = LOWORD(wParam);

    if (usKey == MK_RBUTTON && m_bBarHidden)
    {
        ::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_DOWN;
        ::SendInput(1, &input, sizeof(input));
    }

    if (usKey == 0 && m_bLeftDowned)
    {
        POINT pt{};
        ::GetCursorPos(&pt);
        int iX = m_CursorPos.x - pt.x;
        int iY = m_CursorPos.y - pt.y;

        if (iX == 0 && iY == 0)
        {
            m_DxLibSpinePlayer.ShiftAnimation();
        }
        else
        {
            m_DxLibSpinePlayer.MoveViewPoint(iX, iY);
        }
    }

    m_bLeftDowned = false;

    return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
    WORD usKey = LOWORD(wParam);

    if (usKey == 0)
    {
        m_DxLibSpinePlayer.ResetScale();
    }

    if (usKey == MK_RBUTTON)
    {
        SwitchWindowMode();
    }

    return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
    HMENU hMenuFile = nullptr;
    HMENU hMenuImage = nullptr;
    HMENU hMenuBar = nullptr;
    BOOL iRet = FALSE;

    if (m_hMenuBar != nullptr)return;

    hMenuFile = ::CreateMenu();
    if (hMenuFile == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kOpenFolder, "Open folder");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kFileSetting, "Setting");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kSelectFiles, "Select files");
    if (iRet == 0)goto failed;

    hMenuImage = ::CreateMenu();
    if (hMenuImage == nullptr)goto failed;

    iRet = ::AppendMenuA(hMenuImage, MF_STRING, Menu::kSeeThroughImage, "Through-seen");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuImage, MF_STRING, Menu::kSkeletonSetting, "Manipulation");
    if (iRet == 0)goto failed;

    hMenuBar = ::CreateMenu();
    if (hMenuBar == nullptr) goto failed;

    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFile), "File");
    if (iRet == 0)goto failed;
    iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuImage), "Image");
    if (iRet == 0)goto failed;

    iRet = ::SetMenu(m_hWnd, hMenuBar);
    if (iRet == 0)goto failed;

    m_hMenuBar = hMenuBar;

    return;

failed:
    std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
    ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    if (hMenuFile != nullptr)
    {
        ::DestroyMenu(hMenuFile);
    }
    if (hMenuImage != nullptr)
    {
        ::DestroyMenu(hMenuImage);
    }
    if (hMenuBar != nullptr)
    {
        ::DestroyMenu(hMenuBar);
    }
}
/*フォルダ選択*/
void CMainWindow::MenuOnOpenFolder()
{
    std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder(m_hWnd);
    if (!wstrPickedFolder.empty())
    {
        bool bRet = SetupResources(wstrPickedFolder.c_str());
        if (bRet)
        {
            ClearFolderInfo();
            win_filesystem::GetFolderListAndIndex(wstrPickedFolder.c_str(), m_folders, &m_nFolderIndex);
        }
    }
}
/*取り込みファイル設定*/
void CMainWindow::MenuOnFileSetting()
{
    m_SpineSettingDialogue.Open(::GetModuleHandleA(nullptr), m_hWnd, L"Extensions");
}
/*ファイル選択*/
void CMainWindow::MenuOnSelectFiles()
{
    std::wstring wstrAtlasFile = win_dialogue::SelectOpenFile(L"atlas file", L"", L"Select atlas file", m_hWnd);
    if (!wstrAtlasFile.empty())
    {
        std::wstring wstrSkelFile = win_dialogue::SelectOpenFile(L"skeleton file", L"", L"Select skeleton file", m_hWnd);
        if (!wstrSkelFile.empty())
        {
            ClearFolderInfo();
            std::vector<std::string> atlases;
            std::vector<std::string> skels;

            atlases.push_back(win_text::NarrowANSI(wstrAtlasFile));
            skels.push_back(win_text::NarrowANSI(wstrSkelFile));

            bool bRet = m_DxLibSpinePlayer.SetSpineFromFile(atlases, skels, m_SpineSettingDialogue.IsSkelBinary());
            if (!bRet)
            {
                ::MessageBoxW(nullptr, L"Failed to load spine", L"Error", MB_ICONERROR);
            }
        }
    }
}
/*透過*/
void CMainWindow::MenuOnSeeThroughImage()
{
    HMENU hMenuBar = ::GetMenu(m_hWnd);
    if (hMenuBar != nullptr)
    {
        HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kImage);
        if (hMenu != nullptr)
        {
            m_bTransparent ^= true;

            LONG lStyleEx = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

            if (m_bTransparent)
            {
                ::SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyleEx | WS_EX_LAYERED);
                ::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
                ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            }
            else
            {
                ::SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyleEx & ~WS_EX_LAYERED);
                ::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
                ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            }

            ::CheckMenuItem(hMenu, Menu::kSeeThroughImage, m_bTransparent ? MF_CHECKED : MF_UNCHECKED);
        }
    }
}
/*骨組み操作画面呼び出し*/
void CMainWindow::MenuOnSkeletonSetting()
{
    if (m_SpineManipulatorDialogue.GetHwnd() == nullptr)
    {
        HWND hWnd = m_SpineManipulatorDialogue.Create(m_hInstance, m_hWnd, L"Spine manipulation", &m_DxLibSpinePlayer);

        ::ShowWindow(hWnd, SW_SHOWNORMAL);
    }
    else
    {
        ::SetFocus(m_SpineManipulatorDialogue.GetHwnd());
    }
}
/*次のフォルダに移動*/
void CMainWindow::KeyUpOnNextFolder()
{
    if (m_folders.empty())return;

    ++m_nFolderIndex;
    if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = 0;
    SetupResources(m_folders.at(m_nFolderIndex).c_str());
}
/*前のフォルダに移動*/
void CMainWindow::KeyUpOnForeFolder()
{
    if (m_folders.empty())return;

    --m_nFolderIndex;
    if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = m_folders.size() - 1;
    SetupResources(m_folders.at(m_nFolderIndex).c_str());
}
/*標題変更*/
void CMainWindow::ChangeWindowTitle(const wchar_t* pwzTitle)
{
    std::wstring wstr;
    if (pwzTitle != nullptr)
    {
        std::wstring wstrTitle = pwzTitle;
        size_t nPos = wstrTitle.find_last_of(L"\\/");
        wstr = nPos == std::wstring::npos ? wstrTitle : wstrTitle.substr(nPos + 1);
    }

    ::SetWindowTextW(m_hWnd, wstr.c_str());
}
/*表示形式変更*/
void CMainWindow::SwitchWindowMode()
{
    RECT rect;
    ::GetWindowRect(m_hWnd, &rect);
    LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    m_bBarHidden ^= true;

    if (m_bBarHidden)
    {
        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU);
        ::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
        ::SetMenu(m_hWnd, nullptr);
    }
    else
    {
        ::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU);
        ::SetMenu(m_hWnd, m_hMenuBar);
    }

    m_DxLibSpinePlayer.OnStyleChanged();
}
/*描画素材設定*/
bool CMainWindow::SetupResources(const wchar_t* pwzFolderPath)
{
    if (pwzFolderPath == nullptr)return false;

    std::vector<std::string> atlases;
    std::vector<std::string> skels;

    const std::wstring& wstrAtlasExt = m_SpineSettingDialogue.GetAtlasExtension();
    const std::wstring& wstrSkelExt = m_SpineSettingDialogue.GetSkelExtension();
    bool bIsBinary = m_SpineSettingDialogue.IsSkelBinary();

    std::vector<std::wstring> temps;
    bool bRet = win_filesystem::CreateFilePathList(pwzFolderPath, wstrAtlasExt.c_str(), temps);
    if (bRet)
    {
        for (const std::wstring& temp : temps)
        {
            atlases.push_back(win_text::NarrowANSI(temp));
        }
        temps.clear();
        bRet = win_filesystem::CreateFilePathList(pwzFolderPath, wstrSkelExt.c_str(), temps);
        if (bRet)
        {
            for (const std::wstring& temp : temps)
            {
                skels.push_back(win_text::NarrowANSI(temp));
            }

            bRet = m_DxLibSpinePlayer.SetSpineFromFile(atlases, skels, bIsBinary);
        }
    }
    if (!bRet)
    {
        ::MessageBoxW(nullptr, L"Failed to load spine(s)", L"Error", MB_ICONERROR);
    }
    return bRet;
}
/*階層構成情報消去*/
void CMainWindow::ClearFolderInfo()
{
    m_folders.clear();
    m_nFolderIndex = 0;
}
/*描画間隔更新*/
void CMainWindow::UpdateDrawingInterval()
{
    DEVMODE sDevMode{};
    ::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &sDevMode);
    m_fDelta = 1 / static_cast<float>(sDevMode.dmDisplayFrequency);
}
