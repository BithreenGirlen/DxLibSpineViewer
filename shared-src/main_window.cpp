
#include <Windows.h>
#include <CommCtrl.h>

#include <algorithm>

#include "main_window.h"
#include "win_filesystem.h"
#include "win_dialogue.h"
#include "win_text.h"
#include "dxlib_image_encoder.h"

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
        if (pwzWindowName != nullptr)m_wstrWindowName = pwzWindowName;

        UINT uiDpi = ::GetDpiForSystem();
        int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
        int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

        m_hWnd = ::CreateWindowW(m_swzClassName, pwzWindowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
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
    case WM_RBUTTONUP:
        return OnRButtonUp(wParam, lParam);
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

    m_DxLibSpinePlayer.SetRenderWindow(m_hWnd);

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

    if (m_bPlayReady)
    {
        DxLib::ClearDrawScreen();

        m_DxLibSpinePlayer.Redraw(m_fDelta);

        if (m_bUnderRecording)
        {
            static int iCount = 0;
            ++iCount;
            constexpr int iRecordingInterval = 8;
            //int iInterval = static_cast<int>(::ceilf(1 / m_fDelta / 60.f));
            if (iCount > iRecordingInterval)
            {
                int iWidth = 0;
                int iHeight = 0;
                int iStride = 0;
                std::vector<unsigned char> pixels;
                bool bRet = CDxLibImageEncoder::GetScreenPixels(&iWidth, &iHeight, &iStride, pixels, m_hWnd);
                if (bRet)
                {
                    SImageFrame s;
                    s.uiWidth = iWidth;
                    s.uiHeight = iHeight;
                    s.iStride = iStride;
                    s.pixels = std::move(pixels);
                    m_imageFrames.push_back(std::move(s));
                    float fTrackTime = 0.f;
                    std::wstring wstrFrameName = win_text::WidenUtf8(m_DxLibSpinePlayer.GetCurrentAnimationNameWithTrackTime(&fTrackTime));
                    wstrFrameName += L"_" + std::to_wstring(fTrackTime);
                    m_imageFrameNames.push_back(std::move(wstrFrameName));
                }

                iCount = 0;
            }
        }

        DxLib::ScreenFlip();

        if (m_hWnd != nullptr)
        {
            ::InvalidateRect(m_hWnd, nullptr, FALSE);
        }
    }

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
    static std::vector<SImageFrame> imageFrames;
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
    case 'A':
        m_DxLibSpinePlayer.SwitchPma();
        break;
    case 'B':
        m_DxLibSpinePlayer.SwitchBlendModeAdoption();
        break;
    case 'R':
        m_DxLibSpinePlayer.SwitchDrawOrder();
        break;
    case 'Z':
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
    int wmKind = LOWORD(lParam);
    if (wmKind == 0)
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
        case Menu::kSnapAsPNG:
            MenuOnSaveAsPng();
            break;
        case Menu::kStartRecording:
            MenuOnStartRecording();
            break;
        case Menu::kSaveAsGIF:
            MenuOnEndRecording();
            break;
        case Menu::kSaveAsPNGs:
            MenuOnEndRecording(false);
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
    WORD usKey = LOWORD(wParam);

    if (usKey == 0)
    {
        m_DxLibSpinePlayer.RescaleSkeleton(iScroll > 0);
    }

    if (usKey == MK_LBUTTON)
    {
        m_DxLibSpinePlayer.RescaleTime(iScroll > 0);
        m_bSpeedHavingChanged = true;
    }

    if (usKey == MK_RBUTTON)
    {
        m_DxLibSpinePlayer.ShiftSkin();

        m_bRightCombinated = true;
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

        m_bRightCombinated = true;
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
/*WM_RBUTTONUP*/
LRESULT CMainWindow::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (m_bRightCombinated)
    {
        m_bRightCombinated = false;
        return 0;
    }
    WORD usKey = LOWORD(wParam);

    if (usKey == 0 && m_bPlayReady)
    {
        const auto PreparePupupMenu = [this](HMENU *hMenu)
            -> bool
            {
                if (hMenu == nullptr)return false;

                HMENU hPopupMenu = ::CreatePopupMenu();
                if (hPopupMenu == nullptr)return false;

                *hMenu = hPopupMenu;
                if (!m_bUnderRecording)
                {
                    int iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kSnapAsPNG, L"Snap as PNG");
                    if (iRet == 0)return false;
                    iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kStartRecording, L"Start recording");
                    if (iRet == 0)return false;
                }
                else
                {
                    int iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kSaveAsGIF, L"Save as GIF");
                    if (iRet == 0)return false;
                    iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kSaveAsPNGs, L"Save as PNGs");
                    if (iRet == 0)return false;
                }

                return true;
            };

        HMENU hPopupMenu = nullptr;
        bool bRet = PreparePupupMenu(&hPopupMenu);
        if (hPopupMenu != nullptr)
        {
            if (bRet)
            {
                POINT point{};
                ::GetCursorPos(&point);
                ::TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, 0, m_hWnd, nullptr);
            }
            ::DestroyMenu(hPopupMenu);
        }
    }

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

        m_bRightCombinated = true;
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
    if (m_bUnderRecording)return;

    std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder(m_hWnd);
    if (!wstrPickedFolder.empty())
    {
        bool bRet = SetupResources(wstrPickedFolder.c_str());
        if (bRet)
        {
            ClearFolderInfo();
            win_filesystem::GetFilePathListAndIndex(wstrPickedFolder.c_str(), nullptr, m_folders, &m_nFolderIndex);
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
    std::vector<std::wstring> wstrAtlasFiles = win_dialogue::SelectOpenFiles(L"atlas files", L"", L"Select atlas files", m_hWnd);
    if (!wstrAtlasFiles.empty())
    {
        std::vector<std::wstring> wstrSkelFiles = win_dialogue::SelectOpenFiles(L"skeleton files", L"", L"Select skeleton files", m_hWnd);
        if (!wstrSkelFiles.empty())
        {
            if (wstrAtlasFiles.size() != wstrSkelFiles.size())
            {
                ::MessageBoxW(nullptr, L"The number of atlas and skeleton files should be the same.", L"Error", MB_ICONERROR);
                return;
            }

            std::sort(wstrAtlasFiles.begin(), wstrAtlasFiles.end());
            std::sort(wstrSkelFiles.begin(), wstrSkelFiles.end());

            ClearFolderInfo();
            std::vector<std::string> atlases;
            std::vector<std::string> skels;

            for (const auto& atlas : wstrAtlasFiles)
            {
                atlases.push_back(win_text::NarrowUtf8(atlas));
            }

            for (const auto& skel : wstrSkelFiles)
            {
                skels.push_back(win_text::NarrowUtf8(skel));
            }

            m_bPlayReady = m_DxLibSpinePlayer.SetSpineFromFile(atlases, skels, m_SpineSettingDialogue.IsSkelBinary());
            if (!m_bPlayReady)
            {
                ::MessageBoxW(nullptr, L"Failed to load spine(s)", L"Error", MB_ICONERROR);
            }

            const auto ExtractFileName = [&wstrAtlasFiles]()
                -> std::wstring
                {
                    const std::wstring& wstrFilePath = wstrAtlasFiles.at(0);
                    size_t nPos = wstrFilePath.find_last_of(L"\\/");
                    nPos = nPos == std::string::npos ? 0 : nPos + 1;

                    size_t nPos2 = wstrFilePath.find(L".", nPos);
                    if (nPos2 == std::wstring::npos)nPos2 = wstrFilePath.size() - 1;

                    return wstrFilePath.substr(nPos, nPos2 - nPos);
                };
            ChangeWindowTitle(m_bPlayReady ? ExtractFileName().c_str() : nullptr);
        }
    }
}
/*透過*/
void CMainWindow::MenuOnSeeThroughImage()
{
    if (!m_bPlayReady)return;

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
/*PNGとして保存*/
void CMainWindow::MenuOnSaveAsPng()
{
    if (!m_bPlayReady)return;

    std::wstring wstrFilePath = win_filesystem::CreateWorkFolder(GerWindowTitle());
    float fTrackTime = 0.f;
    wstrFilePath += win_text::WidenUtf8(m_DxLibSpinePlayer.GetCurrentAnimationNameWithTrackTime(&fTrackTime));
    wstrFilePath += L"_" + std::to_wstring(fTrackTime);
    wstrFilePath += L".png";

    CDxLibImageEncoder::SaveScreenAsPng(wstrFilePath, m_hWnd);
}
/*記録開始*/
void CMainWindow::MenuOnStartRecording()
{
    if (!m_bPlayReady)return;

    m_bUnderRecording = true;
}
/*記録終了*/
void CMainWindow::MenuOnEndRecording(bool bAsGif)
{
    m_bUnderRecording = false;
    if (bAsGif)
    {
        std::wstring wstrFilePath = win_filesystem::CreateWorkFolder(GerWindowTitle());
        wstrFilePath += win_text::WidenUtf8(m_DxLibSpinePlayer.GetCurrentAnimationNameWithTrackTime());
        wstrFilePath += L".gif";
        win_image::SaveImagesAsGif(wstrFilePath.c_str(), m_imageFrames);
    }
    else
    {
        std::wstring wstrFolderPath = win_filesystem::CreateWorkFolder(GerWindowTitle());
        for (size_t i = 0; i < m_imageFrames.size() && i < m_imageFrameNames.size(); ++i)
        {
            std::wstring wstrFilePath = wstrFolderPath;
            wstrFilePath += m_imageFrameNames.at(i);
            wstrFilePath += L".png";
            win_image::SaveImageAsPng(wstrFilePath.c_str(), &m_imageFrames.at(i));
        }
    }
    m_imageFrames.clear();
    m_imageFrameNames.clear();
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
    if (m_folders.empty() || m_bUnderRecording)return;

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

    ::SetWindowTextW(m_hWnd, wstr.empty() ? m_wstrWindowName.c_str() : wstr.c_str());
}
/*標題取得*/
std::wstring CMainWindow::GerWindowTitle()
{
    for (int iSize = 256; iSize <= 1024; iSize *= 2)
    {
        std::vector<wchar_t> vBuffer(iSize, L'\0');
        int iLen = ::GetWindowTextW(m_hWnd, vBuffer.data(), static_cast<int>(vBuffer.size()));
        if (iLen < iSize - 1)
        {
            return vBuffer.data();
        }
    }
    return std::wstring();
}
/*表示形式変更*/
void CMainWindow::SwitchWindowMode()
{
    if (!m_bPlayReady)return;

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

    const std::wstring& wstrAtlasExt = m_SpineSettingDialogue.GetAtlasExtension();
    const std::wstring& wstrSkelExt = m_SpineSettingDialogue.GetSkelExtension();
    bool bIsBinary = m_SpineSettingDialogue.IsSkelBinary();

    enum EExtensionRelation{ kExclusive = 0, kAltasInclusive, kSkelInclusive };
    const auto GeExtensionRelation = [&wstrAtlasExt, wstrSkelExt]()
        -> int
        {
            if (wstrAtlasExt.find(wstrSkelExt) != std::wstring::npos)
            {
                return kAltasInclusive;
            }
            else if (wstrSkelExt.find(wstrAtlasExt) != std::wstring::npos)
            {
                return kSkelInclusive;
            }
            return kExclusive;
        };

    bool bRet = false;
    std::vector<std::string> atlases;
    std::vector<std::string> skels;

    std::vector<std::wstring> temps;
    int iExtensionRelation = GeExtensionRelation();
    if (iExtensionRelation != kExclusive)
    {
        const std::wstring& wstrInclusive = iExtensionRelation == kAltasInclusive ? wstrAtlasExt : wstrSkelExt;
        const std::wstring& wstrContained = iExtensionRelation == kAltasInclusive ? wstrSkelExt : wstrAtlasExt;
        bRet = win_filesystem::CreateFilePathList(pwzFolderPath, wstrContained.c_str(), temps);
        if (bRet)
        {
            const std::wstring wstrDif = wstrInclusive.substr(0, wstrInclusive.size() - wstrContained.size());
            auto& inclusive = iExtensionRelation == kAltasInclusive ? atlases : skels;
            auto& contained = iExtensionRelation == kAltasInclusive ? skels : atlases;

            for (const auto& temp : temps)
            {
                if (temp.find(wstrDif) != std::wstring::npos)
                {
                    inclusive.push_back(win_text::NarrowUtf8(temp));
                }
                else
                {
                    contained.push_back(win_text::NarrowUtf8(temp));
                }
            }
        }
    }
    else
    {
        bRet = win_filesystem::CreateFilePathList(pwzFolderPath, wstrAtlasExt.c_str(), temps);
        if (bRet)
        {
            for (const std::wstring& temp : temps)
            {
                atlases.push_back(win_text::NarrowUtf8(temp));
            }
            temps.clear();
            bRet = win_filesystem::CreateFilePathList(pwzFolderPath, wstrSkelExt.c_str(), temps);
            if (bRet)
            {
                for (const std::wstring& temp : temps)
                {
                    skels.push_back(win_text::NarrowUtf8(temp));
                }
            }
        }
    }
    if (bRet)
    {
        bRet = m_DxLibSpinePlayer.SetSpineFromFile(atlases, skels, bIsBinary);
        ChangeWindowTitle(bRet ? pwzFolderPath : nullptr);
        m_bPlayReady = bRet;
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
