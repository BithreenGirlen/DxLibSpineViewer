
#include <Windows.h>
#include <CommCtrl.h>

#include <algorithm>

#include "main_window.h"
#include "win_filesystem.h"
#include "win_dialogue.h"
#include "win_text.h"
#include "dxlib_image_encoder.h"
#include "cpu.h"

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
		const wchar_t* windowName = pwzWindowName == nullptr ? m_swzDefaultWindowName : pwzWindowName;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		m_hWnd = ::CreateWindowW(m_swzClassName, windowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			return true;
		}
		else
		{
			std::wstring wstrMessage = L"CreateWindowW failed; code: " + std::to_wstring(::GetLastError());
			::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
		}
	}
	else
	{
		std::wstring wstrMessage = L"RegisterClassExW failed; code: " + std::to_wstring(::GetLastError());
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
	case WM_SIZE:
		return OnSize(wParam, lParam);
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
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, lParam);
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

	if (m_bPlayReady && m_recoderState != RecorderState::InitialisingVideoStream)
	{
		DxLib::ClearDrawScreen();

		m_DxLibSpinePlayer.Redraw(m_fDelta);

		StepOnRecording();

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
LRESULT CMainWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	int iClientWidth = LOWORD(lParam);
	int iClientHeight = HIWORD(lParam);

	int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

	/* 
	* WM_SIZE seems to be called twice just after window style has been changed.
	* If SetGraphMode() were not so effective, store the previous size and compare here.
	*/
	int iGraphWidth = iClientWidth < iDesktopWidth ? iClientWidth : iDesktopWidth;
	int iGraphHeight = iClientHeight < iDesktopHeight ? iClientHeight : iDesktopHeight;
	DxLib::SetGraphMode(iGraphWidth,iGraphHeight,32);

	if (m_bManuallyResizable)
	{
		m_DxLibSpinePlayer.AdjustViewOffset();
	}

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
	case 'A':
		m_DxLibSpinePlayer.TogglePma();
		break;
	case 'B':
		m_DxLibSpinePlayer.ToggleBlendModeAdoption();
		break;
	case 'R':
		m_DxLibSpinePlayer.ToggleDrawOrder();
		break;
	case 'Z':
		m_DxLibSpinePlayer.ToggleDepthBufferValidity();
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
		case Menu::kSkeletonSetting:
			MenuOnSkeletonSetting();
			break;
		case Menu::kAtlasSetting:
			MenuOnAtlasSetting();
			break;
		case Menu::kAddEffectFile:
			MenuOnAddFile();
			break;
		case Menu::kSeeThroughImage:
			MenuOnSeeThroughImage();
			break;
		case Menu::kAllowManualSizing:
			MenuOnAllowManualSizing();
			break;
		case Menu::kMoveViewOnRelease:
			MenuOnMoveViewOnRelease();
			break;
		case Menu::kReverseZoomDirection:
			MenuonReverseZoomDirection();
			break;
		case Menu::kSnapAsPNG:
			MenuOnSaveAsPng();
			break;
		case Menu::kSnapAsJPG:
			MenuOnSaveAsJpg();
			break;
		case Menu::kStartStoringImages:
			MenuOnStartRecording(false);
			break;
		case Menu::kStartVideoRecording:
			MenuOnStartRecording(true);
			break;
		case Menu::kSaveAsGIF:
			MenuOnEndRecording();
			break;
		case Menu::kSaveAsPNGs:
			MenuOnEndRecording(false);
			break;
		case Menu::kEndVideoRecording:
			MenuOnEndRecording();
			break;
		}
	}
	else
	{
		/*Controls*/
	}

	return 0;
}
/*WM_MOUSEMOVE */
LRESULT CMainWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);
	if (usKey == MK_LBUTTON)
	{
		if (m_bLeftCombinated || m_bToPanOnce)return 0;

		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_cursorPos.x - pt.x;
		int iY = m_cursorPos.y - pt.y;

		m_DxLibSpinePlayer.MoveViewPoint(iX, iY);

		m_cursorPos = pt;
		m_bLeftDragged = true;
	}

	return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
	WORD usKey = LOWORD(wParam);

	if (usKey == MK_LBUTTON)
	{
		m_DxLibSpinePlayer.RescaleTime(iScroll > 0);

		m_bLeftCombinated = true;
	}
	else if (usKey == MK_RBUTTON)
	{
		m_DxLibSpinePlayer.ShiftSkin();

		m_bRightCombinated = true;
	}
	else
	{
		if (m_bPlayReady)
		{
			m_DxLibSpinePlayer.RescaleSkeleton((iScroll > 0) ^ m_bZoomReversed);

			bool bWindowToBeResized = !(usKey & MK_CONTROL) && m_recoderState != RecorderState::RecordingVideo;
			if (bWindowToBeResized)
			{
				m_DxLibSpinePlayer.RescaleCanvas((iScroll > 0) ^ m_bZoomReversed);
				ResizeWindow();
			}
		}
	}

	return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::GetCursorPos(&m_cursorPos);

	m_bLeftDowned = true;

	return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_bLeftCombinated || m_bLeftDragged)
	{
		m_bLeftDragged = false;
		m_bLeftCombinated = false;
		m_bLeftDowned = false;

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
		int iX = m_cursorPos.x - pt.x;
		int iY = m_cursorPos.y - pt.y;

		if (iX == 0 && iY == 0)
		{
			m_DxLibSpinePlayer.ShiftAnimation();
		}
		else
		{
			if (m_bToPanOnce)
			{
				m_DxLibSpinePlayer.MoveViewPoint(iX, iY);
			}
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
				if (m_recoderState == RecorderState::Idle)
				{
					int iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kSnapAsPNG, L"Snap as PNG");
					if (iRet == 0)return false;
					iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kSnapAsJPG, L"Snap as JPG");
					if (iRet == 0)return false;
					iRet = ::AppendMenu(hPopupMenu, MF_SEPARATOR, 0, nullptr);
					if (iRet == 0)return false;
					iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kStartStoringImages, L"Start image recording");
					if (iRet == 0)return false;
					if (cpu::IsIntel())
					{
						iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kStartVideoRecording, L"Start video recording");
						if (iRet == 0)return false;
					}
				}
				else if (m_recoderState == RecorderState::StoringImages)
				{
					int iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kSaveAsGIF, L"Save as GIF");
					if (iRet == 0)return false;
					iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kSaveAsPNGs, L"Save as PNGs");
					if (iRet == 0)return false;
				}
				else if (m_recoderState == RecorderState::RecordingVideo)
				{
					int iRet = ::AppendMenu(hPopupMenu, MF_STRING, Menu::kEndVideoRecording, L"End recording");
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
		if (m_bPlayReady)
		{
			m_DxLibSpinePlayer.ResetScale();

			ResizeWindow();
		}
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
	HMENU hMenuWindow = nullptr;
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

	iRet = ::AppendMenuA(hMenuImage, MF_STRING, Menu::kSkeletonSetting, "Manipulation");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuImage, MF_STRING, Menu::kAtlasSetting, "Re-attachment");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuImage, MF_STRING, Menu::kAddEffectFile, "Add effect");
	if (iRet == 0)goto failed;

	hMenuWindow = ::CreateMenu();
	if (hMenuWindow == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuWindow, MF_STRING, Menu::kSeeThroughImage, "Through-seen");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuWindow, MF_STRING, Menu::kAllowManualSizing, "Allow manual sizing");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuWindow, MF_STRING, Menu::kMoveViewOnRelease, "Move view on release");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuWindow, MF_STRING, Menu::kReverseZoomDirection, "Reverse zoom direction");
	if (iRet == 0)goto failed;

	hMenuBar = ::CreateMenu();
	if (hMenuBar == nullptr) goto failed;

	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFile), "File");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuImage), "Image");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuWindow), "Window");
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
	if (hMenuWindow != nullptr)
	{
		::DestroyMenu(hMenuWindow);
	}
	if (hMenuBar != nullptr)
	{
		::DestroyMenu(hMenuBar);
	}
}
/*フォルダ選択*/
void CMainWindow::MenuOnOpenFolder()
{
	if (m_recoderState != RecorderState::Idle)return;

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
	if (m_recoderState != RecorderState::Idle)return;

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

			m_bPlayReady = m_DxLibSpinePlayer.SetSpineFromFile(atlases, skels, m_SpineSettingDialogue.IsSkelBinary(wstrSkelFiles[0].c_str()));
			if (m_bPlayReady)
			{
				ResizeWindow();
			}
			else
			{
				::MessageBoxW(nullptr, L"Failed to load spine(s)", L"Error", MB_ICONERROR);
			}

			const auto ExtractFileName = [&wstrAtlasFiles]()
				-> std::wstring
				{
					const std::wstring& wstrFilePath = wstrAtlasFiles[0];
					size_t nPos = wstrFilePath.find_last_of(L"\\/");
					nPos = nPos == std::string::npos ? 0 : nPos + 1;

					size_t nPos2 = wstrFilePath.find(L".", nPos);
					if (nPos2 == std::wstring::npos)nPos2 = wstrFilePath.size();

					return wstrFilePath.substr(nPos, nPos2 - nPos);
				};
			ChangeWindowTitle(m_bPlayReady ? ExtractFileName().c_str() : nullptr);
		}
	}
}
/*ファイル追加*/
void CMainWindow::MenuOnAddFile()
{
	if (!m_bPlayReady || m_recoderState != RecorderState::Idle)return;

	std::wstring wstrAtlasFile = win_dialogue::SelectOpenFile(L"atlas file", L"", L"Select atlas file to add", m_hWnd);
	if (wstrAtlasFile.empty())return;

	std::wstring wstrSkeletonFile = win_dialogue::SelectOpenFile(L"skeleton file", L"", L"Select skeleton file to add", m_hWnd);
	if (wstrSkeletonFile.empty())return;

	std::string strAtlasFile = win_text::NarrowUtf8(wstrAtlasFile);
	std::string strSkeletonFile = win_text::NarrowUtf8(wstrSkeletonFile);
	bool bBinary = m_SpineSettingDialogue.IsSkelBinary(wstrSkeletonFile.c_str());

	m_DxLibSpinePlayer.AddSpineFromFile(strAtlasFile.c_str(), strSkeletonFile.c_str(), bBinary);
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
/*装着変更画面呼び出し*/
void CMainWindow::MenuOnAtlasSetting()
{
	if (m_SpineAtlasDialogue.GetHwnd() == nullptr)
	{
		HWND hWnd = m_SpineAtlasDialogue.Create(m_hInstance, m_hWnd, L"Spine re-attachment", &m_DxLibSpinePlayer);

		::ShowWindow(hWnd, SW_SHOWNORMAL);
	}
	else
	{
		::SetFocus(m_SpineAtlasDialogue.GetHwnd());
	}
}
/*透過*/
void CMainWindow::MenuOnSeeThroughImage()
{
	if (!m_bPlayReady)return;

	HMENU hMenuBar = ::GetMenu(m_hWnd);
	if (hMenuBar != nullptr)
	{
		HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kWindow);
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
/*手動寸法変更許可切り替え*/
void CMainWindow::MenuOnAllowManualSizing()
{
	HMENU hMenuBar = ::GetMenu(m_hWnd);
	if (hMenuBar != nullptr)
	{
		HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kWindow);
		if (hMenu != nullptr)
		{
			bool bAllowed = m_bPlayReady && !m_bManuallyResizable && m_recoderState != RecorderState::RecordingVideo;
			DWORD ulRet = ::CheckMenuItem(hMenu, Menu::kAllowManualSizing, bAllowed ? MF_CHECKED : MF_UNCHECKED);
			if (ulRet != (DWORD)-1)
			{
				m_bManuallyResizable = bAllowed;
				UpdateWindowResizableAttribute();
			}
		}
	}
}
/*視点移動法切り替え*/
void CMainWindow::MenuOnMoveViewOnRelease()
{
	HMENU hMenuBar = ::GetMenu(m_hWnd);
	if (hMenuBar != nullptr)
	{
		HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kWindow);
		if (hMenu != nullptr)
		{
			DWORD ulRet = ::CheckMenuItem(hMenu, Menu::kMoveViewOnRelease, m_bToPanOnce ? MF_UNCHECKED : MF_CHECKED);
			if (ulRet != (DWORD)-1)
			{
				m_bToPanOnce ^= true;
			}
		}
	}
}

void CMainWindow::MenuonReverseZoomDirection()
{
	HMENU hMenuBar = ::GetMenu(m_hWnd);
	if (hMenuBar != nullptr)
	{
		HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kWindow);
		if (hMenu != nullptr)
		{
			DWORD ulRet = ::CheckMenuItem(hMenu, Menu::kReverseZoomDirection, m_bZoomReversed ? MF_UNCHECKED : MF_CHECKED);
			if (ulRet != (DWORD)-1)
			{
				m_bZoomReversed ^= true;
			}
		}
	}
}
/*次のフォルダに移動*/
void CMainWindow::KeyUpOnNextFolder()
{
	if (m_folders.empty() || m_recoderState != RecorderState::Idle)return;

	++m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = 0;
	SetupResources(m_folders[m_nFolderIndex].c_str());
}
/*前のフォルダに移動*/
void CMainWindow::KeyUpOnForeFolder()
{
	if (m_folders.empty() || m_recoderState != RecorderState::Idle)return;

	--m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = m_folders.size() - 1;
	SetupResources(m_folders[m_nFolderIndex].c_str());
}
/*JPGとして保存*/
void CMainWindow::MenuOnSaveAsJpg()
{
	if (!m_bPlayReady)return;

	std::wstring wstrFilePath = win_filesystem::CreateWorkFolder(GetWindowTitle());
	float fTrackTime = 0.f;
	wstrFilePath += win_text::WidenUtf8(m_DxLibSpinePlayer.GetCurrentAnimationNameWithTrackTime(&fTrackTime));
	wstrFilePath += L"_" + std::to_wstring(fTrackTime);
	wstrFilePath += L".jpg";

	CDxLibImageEncoder::SaveScreenAsJpg(wstrFilePath.c_str());
}
/*PNGとして保存*/
void CMainWindow::MenuOnSaveAsPng()
{
	if (!m_bPlayReady)return;

	std::wstring wstrFilePath = win_filesystem::CreateWorkFolder(GetWindowTitle());
	float fTrackTime = 0.f;
	wstrFilePath += win_text::WidenUtf8(m_DxLibSpinePlayer.GetCurrentAnimationNameWithTrackTime(&fTrackTime));
	wstrFilePath += L"_" + std::to_wstring(fTrackTime);
	wstrFilePath += L".png";

	CDxLibImageEncoder::SaveScreenAsPng(wstrFilePath.c_str());
}
/*記録開始*/
void CMainWindow::MenuOnStartRecording(bool bAsVideo)
{
	if (!m_bPlayReady)return;

	if (bAsVideo)
	{
		int iWidth = 0;
		int iHeight = 0;
		DxLib::GetScreenState(&iWidth, &iHeight, nullptr);
		std::wstring wstrFilePath = win_filesystem::CreateWorkFolder(GetWindowTitle());
		wstrFilePath += win_text::WidenUtf8(m_DxLibSpinePlayer.GetCurrentAnimationNameWithTrackTime());
		wstrFilePath += L".mp4";
		/* Initialising input media types takes time. In the meantime, pause rendering. */
		m_recoderState = RecorderState::InitialisingVideoStream;
		bool bRet = m_MfVideoEncoder.Start(wstrFilePath.c_str(), iWidth, iHeight, 60);
		if (bRet)
		{
			m_recoderState = RecorderState::RecordingVideo;

			/* Disable manual resizing once video recording has started. */
			MenuOnAllowManualSizing();
		}
		else
		{
			m_recoderState = RecorderState::Idle;
		}
		/* Resume rendering. */
		if (m_hWnd != nullptr)
		{
			::InvalidateRect(m_hWnd, nullptr, FALSE);
		}
	}
	else
	{
		m_recoderState = RecorderState::StoringImages;
	}
}
/*記録終了*/
void CMainWindow::MenuOnEndRecording(bool bAsGif)
{
	if (m_recoderState == RecorderState::RecordingVideo)
	{
		m_MfVideoEncoder.End();
	}
	else
	{
		if (bAsGif)
		{
			std::wstring wstrFilePath = win_filesystem::CreateWorkFolder(GetWindowTitle());
			wstrFilePath += win_text::WidenUtf8(m_DxLibSpinePlayer.GetCurrentAnimationNameWithTrackTime());
			wstrFilePath += L".gif";
			win_image::SaveImagesAsGif(wstrFilePath.c_str(), m_imageFrames);
		}
		else
		{
			std::wstring wstrFolderPath = win_filesystem::CreateWorkFolder(GetWindowTitle());
			for (size_t i = 0; i < m_imageFrames.size() && i < m_imageFrameNames.size(); ++i)
			{
				std::wstring wstrFilePath = wstrFolderPath;
				wstrFilePath += m_imageFrameNames[i];
				wstrFilePath += L".png";
				win_image::SaveImageAsPng(wstrFilePath.c_str(), &m_imageFrames[i]);
			}
		}
		m_imageFrames.clear();
		m_imageFrameNames.clear();
	}

	m_recoderState = RecorderState::Idle;
	m_iFrameCount = 0;
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

	::SetWindowTextW(m_hWnd, wstr.empty() ? m_swzDefaultWindowName : wstr.c_str());
}
/*標題取得*/
std::wstring CMainWindow::GetWindowTitle()
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
	if (!m_bPlayReady || m_recoderState == RecorderState::RecordingVideo)return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_bBarHidden ^= true;

	if (m_bBarHidden)
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU & ~WS_THICKFRAME);
		::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		::SetMenu(m_hWnd, nullptr);
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME);
		::SetMenu(m_hWnd, m_hMenuBar);
	}

	ResizeWindow();
}
/*描画素材設定*/
bool CMainWindow::SetupResources(const wchar_t* pwzFolderPath)
{
	if (pwzFolderPath == nullptr)return false;

	std::vector<std::string> atlasPaths;
	std::vector<std::string> skelPaths;

	const std::wstring& wstrAtlasExt = m_SpineSettingDialogue.GetAtlasExtension();
	const std::wstring& wstrSkelExt = m_SpineSettingDialogue.GetSkelExtension();
	bool bIsBinary = m_SpineSettingDialogue.IsSkelBinary();

	bool bAtlasLonger = wstrAtlasExt.size() > wstrSkelExt.size();

	const std::wstring& wstrLongerExtesion = bAtlasLonger ? wstrAtlasExt : wstrSkelExt;
	const std::wstring& wstrShorterExtension = bAtlasLonger ? wstrSkelExt : wstrAtlasExt;

	std::vector<std::string>& strLongerPaths = bAtlasLonger ? atlasPaths : skelPaths;
	std::vector<std::string>& strShorterPaths = bAtlasLonger ? skelPaths : atlasPaths;

	std::vector<std::wstring> wstrFilePaths;
	win_filesystem::CreateFilePathList(pwzFolderPath, L"*", wstrFilePaths);

	for (const auto& filePath : wstrFilePaths)
	{
		const auto EndsWith = [&filePath](const std::wstring& str)
			-> bool
			{
				if (filePath.size() < str.size()) return false;
				return std::equal(str.rbegin(), str.rend(), filePath.rbegin());
			};

		if (EndsWith(wstrLongerExtesion))
		{
			strLongerPaths.push_back(win_text::NarrowUtf8(filePath));
		}
		else if (EndsWith(wstrShorterExtension))
		{
			strShorterPaths.push_back(win_text::NarrowUtf8(filePath));
		}
	}

	m_bPlayReady = m_DxLibSpinePlayer.SetSpineFromFile(atlasPaths, skelPaths, bIsBinary);
	ChangeWindowTitle(m_bPlayReady ? pwzFolderPath : nullptr);
	if (m_bPlayReady)
	{
		ResizeWindow();
	}
	else
	{
		::MessageBoxW(nullptr, L"Failed to load spine(s)", L"Error", MB_ICONERROR);
	}

	return m_bPlayReady;
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
/*記録逓進*/
void CMainWindow::StepOnRecording()
{
	if (m_recoderState == RecorderState::Idle)return;

	/*
	* WM_PAINT comes at intervals of display refresh rate.
	* For example, if refresh rate is 120Hz, iInterval will be 2.
	* The condition "m_iFrameCount >= iInterval" results in 60fps for video, and 15fps for images.
	* Slower refresh rate such as 40Hz is not taken into consideration here.
	*/
	int iInterval = static_cast<int>(::ceilf(1 / m_fDelta / 60.f));
	if (m_recoderState == RecorderState::StoringImages)
	{
		++m_iFrameCount;
		iInterval *= 4;
		if (m_iFrameCount >= iInterval)
		{
			int iWidth = 0;
			int iHeight = 0;
			int iStride = 0;
			std::vector<unsigned char> pixels;
			bool bRet = CDxLibImageEncoder::GetScreenPixels(&iWidth, &iHeight, &iStride, pixels);
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

			m_iFrameCount = 0;
		}
	}
	else if (m_recoderState == RecorderState::RecordingVideo)
	{
		++m_iFrameCount;
		if (m_iFrameCount >= iInterval)
		{
			int iWidth = 0;
			int iHeight = 0;
			int iStride = 0;
			std::vector<unsigned char> pixels;
			bool bRet = CDxLibImageEncoder::GetScreenPixels(&iWidth, &iHeight, &iStride, pixels, false);
			if (bRet)
			{
				m_MfVideoEncoder.AddCpuFrame(pixels.data(), static_cast<unsigned long>(pixels.size()), false);
			}

			m_iFrameCount = 0;
		}
	}
}
/*寸法手動変更可否属性更新*/
void CMainWindow::UpdateWindowResizableAttribute()
{
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	::SetWindowLong(m_hWnd, GWL_STYLE, (m_bPlayReady && m_bManuallyResizable) ? (lStyle | WS_THICKFRAME) : (lStyle & ~WS_THICKFRAME));
}
/*窓寸法変更*/
void CMainWindow::ResizeWindow()
{
	float fWidth = 0.f;
	float fHeight = 0.f;
	m_DxLibSpinePlayer.GetBaseSize(&fWidth, &fHeight);
	float fScale = m_DxLibSpinePlayer.GetCanvasScale();

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	int iX = static_cast<int>(fWidth * fScale);
	int iY = static_cast<int>(fHeight * fScale);

	rect.right = iX + rect.left;
	rect.bottom = iY + rect.top;

	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	const auto IsWidowBarHidden = [&lStyle]()
		-> bool
		{
			return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
		};

	::AdjustWindowRect(&rect, lStyle, IsWidowBarHidden() ? FALSE : TRUE);
	::SetWindowPos(m_hWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);

	m_DxLibSpinePlayer.AdjustViewOffset();
}
