
#include <Windows.h>
#include <CommCtrl.h>

#include <algorithm>

#include "main_window.h"
#include "win_filesystem.h"
#include "win_dialogue.h"
#include "win_text.h"
#include "dxlib_image_encoder.h"
#include "json_minimal.h"
#include "text_utility.h"
#include "native-ui/window_menu.h"


CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName, HICON hIcon)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszClassName = m_swzClassName;
	if (hIcon != nullptr)
	{
		wcex.hIcon = hIcon;
		wcex.hIconSm = hIcon;
	}

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
	}

	return false;
}

int CMainWindow::MessageLoop()
{
	MSG msg{};

	for (; msg.message != WM_QUIT;)
	{
		BOOL iRet = m_dxLibSpinePlayer.HasSpineBeenLoaded() ?
			::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) :
			::GetMessageW(&msg, nullptr, 0, 0);
		if (iRet)
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}

		if (m_hasProcessedWmPaint)
		{
			m_hasProcessedWmPaint = false;
			continue;
		}

		Tick();
	}

	return static_cast<int>(msg.wParam);
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
	UpdateMenuItemState();

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
	if (!m_dxLibSpinePlayer.HasSpineBeenLoaded())
	{
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(m_hWnd, &ps);
		::EndPaint(m_hWnd, &ps);
	}
	else
	{
		Tick();
		m_hasProcessedWmPaint = true;
	}

	return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	int iClientWidth = LOWORD(lParam);
	int iClientHeight = HIWORD(lParam);

	int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

	int iGraphWidth = iClientWidth < iDesktopWidth ? iClientWidth : iDesktopWidth;
	int iGraphHeight = iClientHeight < iDesktopHeight ? iClientHeight : iDesktopHeight;
	DxLib::SetGraphMode(iGraphWidth, iGraphHeight, 32);

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
		m_dxLibSpinePlayer.TogglePma();
		break;
	case 'B':
		m_dxLibSpinePlayer.ToggleBlendModeAdoption();
		break;
	case 'R':
		m_dxLibSpinePlayer.SetDrawOrder(!m_dxLibSpinePlayer.IsDrawOrderReversed());
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
		case Menu::kOpenFiles:
			MenuOnOpenFiles();
			break;
		case Menu::kOpenFolder:
			MenuOnOpenFolder();
			break;
		case Menu::kExtensionSetting:
			MenuOnExtensionSetting();
			break;
		case Menu::kImportCocos:
			MenuOnImportCocos();
			break;
		case Menu::kSpineTool:
			MenuOnSpineTool();
			break;
		case Menu::kAddEffectFile:
			MenuOnAddFile();
			break;
		case Menu::kExportSetting:
			MenuOnExportSetting();
			break;
		case Menu::kSeeThroughImage:
			MenuOnMakeWindowTransparent();
			break;
		case Menu::kAllowManualSizing:
			MenuOnAllowManualSizing();
			break;
		case Menu::kReverseZoomDirection:
			MenuOnReverseZoomDirection();
			break;
		case Menu::kFitToManualSize:
			MenuOnFiToManualSize();
			break;
		case Menu::kFitToDefaultSize:
			MenuOnFitToDefaultSize();
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
		if (m_wasLeftCombinated)return 0;

		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_lastCursorPos.x - pt.x;
		int iY = m_lastCursorPos.y - pt.y;

		if (m_hasLeftBeenDragged)
		{
			m_dxLibSpinePlayer.MoveViewPoint(iX, iY);
		}

		m_lastCursorPos = pt;
		m_hasLeftBeenDragged = true;
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
		m_dxLibSpinePlayer.RescaleTime(iScroll > 0);

		m_wasLeftCombinated = true;
	}
	else if (usKey == MK_RBUTTON)
	{
		m_dxLibSpinePlayer.ShiftSkin();

		m_wasRightCombinated = true;
	}
	else
	{
		if (m_dxLibSpinePlayer.HasSpineBeenLoaded())
		{
			m_dxLibSpinePlayer.RescaleSkeleton((iScroll > 0) ^ m_isZoomDirectionReversed);

			bool bWindowToBeResized = !(usKey & MK_CONTROL) && m_dxLibRecorder.GetState() != CDxLibRecorder::EState::UnderRecording;
			if (bWindowToBeResized)
			{
				m_dxLibSpinePlayer.RescaleCanvas((iScroll > 0) ^ m_isZoomDirectionReversed);
				ResizeWindow();
			}
		}
	}

	return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::GetCursorPos(&m_lastCursorPos);

	m_wasLeftPressed = true;

	return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_wasLeftCombinated || m_hasLeftBeenDragged)
	{
		m_hasLeftBeenDragged = false;
		m_wasLeftCombinated = false;
		m_wasLeftPressed = false;

		return 0;
	}

	WORD usKey = LOWORD(wParam);

	if (usKey == MK_RBUTTON && m_isFramelessWindow)
	{
		::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
		INPUT input{};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_DOWN;
		::SendInput(1, &input, sizeof(input));

		m_wasRightCombinated = true;
	}

	if (usKey == 0 && m_wasLeftPressed)
	{
		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_lastCursorPos.x - pt.x;
		int iY = m_lastCursorPos.y - pt.y;

		if (iX == 0 && iY == 0)
		{
			const auto& recorderState = m_dxLibRecorder.GetState();
			if (recorderState == CDxLibRecorder::EState::Idle || !m_exportSettingDialogue.IsToExportPerAnimation())
			{
				m_dxLibSpinePlayer.ShiftAnimation();
			}
		}
	}

	m_wasLeftPressed = false;

	return 0;
}
/*WM_RBUTTONUP*/
LRESULT CMainWindow::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_wasRightCombinated)
	{
		m_wasRightCombinated = false;
		return 0;
	}

	if (!m_dxLibSpinePlayer.HasSpineBeenLoaded())return 0;

	WORD usKey = LOWORD(wParam);
	if (usKey == 0)
	{
		const auto& recorderState = m_dxLibRecorder.GetState();

		window_menu::CContextMenu contextMenu;
		if (recorderState == CDxLibRecorder::EState::Idle)
		{
			contextMenu.AddItems(
				{
					{PopupMenu::kSnapAsPNG, L"Snap as PNG"},
					{PopupMenu::kSnapAsJPG, L"Snap as JPG"},
					{},
					{PopupMenu::kExportAsGif, L"Export as GIF"},
					{PopupMenu::kExportAsVideo, L"Export as H264"}
				});

			if (m_exportSettingDialogue.IsToExportPerAnimation())
			{
				contextMenu.AddItems(
					{
						{},
						{PopupMenu::kExportAsPngs, L"Export as PNGs"},
						{PopupMenu::kExportAsJpgs, L"Export as JPGs"}
					});
			}
		}
		else if (recorderState == CDxLibRecorder::EState::UnderRecording)
		{
			contextMenu.AddItems({ {PopupMenu::kEndRecording, L"End recording"} });
		}

		BOOL menuIndex = contextMenu.Display(m_hWnd);
		if (menuIndex > 0)
		{
			switch (menuIndex)
			{
			case PopupMenu::kSnapAsPNG:
				MenuOnSaveAsPng();
				break;
			case PopupMenu::kSnapAsJPG:
				MenuOnSaveAsJpg();
				break;
			case PopupMenu::kExportAsGif:
			case PopupMenu::kExportAsVideo:
			case PopupMenu::kExportAsPngs:
			case PopupMenu::kExportAsJpgs:
				MenuOnStartRecording(menuIndex);
				break;
			case PopupMenu::kEndRecording:
				MenuOnEndRecording();
				break;
			default: break;
			}
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
		if (m_dxLibSpinePlayer.HasSpineBeenLoaded() && m_dxLibRecorder.GetState() != CDxLibRecorder::EState::UnderRecording)
		{
			m_dxLibSpinePlayer.ResetScale();
			ResizeWindow();
		}
	}
	else if (usKey == MK_RBUTTON)
	{
		ToggleWindowFrameStyle();

		m_wasRightCombinated = true;
	}

	return 0;
}

void CMainWindow::Tick()
{
	const auto& recorderState = m_dxLibRecorder.GetState();
	if (m_dxLibSpinePlayer.HasSpineBeenLoaded() && recorderState != CDxLibRecorder::EState::InitialisingVideoStream)
	{
		DxLib::ClearDrawScreen();

		float fDelta = m_winclock.GetElapsedTime();
		if (recorderState == CDxLibRecorder::EState::UnderRecording)
		{
			fDelta = m_dxLibRecorder.HasFrames() ? 1.f / m_dxLibRecorder.GetFps() : 0.f;
		}

		m_dxLibSpinePlayer.Update(fDelta);
		m_dxLibSpinePlayer.Redraw();

		StepRecording();

		m_winclock.Restart();

		DxLib::ScreenFlip();
	}
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
	if (::IsMenu(m_hMenuBar))return;

	HMENU hMenu = window_menu::MenuBuilder(
		{
			{0, L"File", window_menu::MenuBuilder(
				{
					{Menu::kOpenFiles, L"Open Files"},
					{},
					{Menu::kOpenFolder, L"Open folder"},
					{Menu::kExtensionSetting, L"Extension setting"},
					{},
					{Menu::kImportCocos, L"Import Cocos"},
				}).Get()
			},
			{0, L"Tool", window_menu::MenuBuilder(
				{
					{Menu::kSpineTool, L"Spine tool"},
					{Menu::kAddEffectFile, L"Add animation effect"},
					{Menu::kExportSetting, L"Export setting"}
				}).Get()
			},
			{0, L"Window", window_menu::MenuBuilder(
				{
					{Menu::kSeeThroughImage, L"Make tranparent"},
					{Menu::kAllowManualSizing, L"Allow manual sizing"},
					{Menu::kReverseZoomDirection, L"Reverse zoom direction"},
					{0, L"Base size", window_menu::MenuBuilder(
						{
							{Menu::kFitToManualSize, L"Fit to current frame"},
							{Menu::kFitToDefaultSize, L"Reset to the default"}
						}).Get()
					},
				}).Get()
			}
		}
	).Get();

	if (::IsMenu(hMenu))
	{
		if (::SetMenu(m_hWnd, hMenu) != 0)
		{
			m_hMenuBar = hMenu;
		}
		else
		{
			::DestroyMenu(hMenu);
		}
	}
}
/*ファイル選択*/
void CMainWindow::MenuOnOpenFiles()
{
	if (m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	std::vector<std::wstring> wstrAtlasFiles = win_dialogue::SelectOpenFiles(L"atlas files", L"*.atlas;*.atlas.txt", L"Select atlas files", m_hWnd, true);
	if (!wstrAtlasFiles.empty())
	{
		std::vector<std::wstring> wstrSkelFiles = win_dialogue::SelectOpenFiles(L"skeleton files", L"*.skel;*.bin;*.json;*.txt", L"Select skeleton files", m_hWnd, true);
		if (!wstrSkelFiles.empty())
		{
			if (wstrAtlasFiles.size() != wstrSkelFiles.size())
			{
				win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"The number of atlas and skeleton files should be the same.", m_hWnd);
				return;
			}

			ClearFolderPathList();

			std::sort(wstrAtlasFiles.begin(), wstrAtlasFiles.end());
			std::sort(wstrSkelFiles.begin(), wstrSkelFiles.end());

			std::vector<std::string> atlasPaths;
			std::vector<std::string> skelPaths;

			for (const auto& atlas : wstrAtlasFiles)
			{
				atlasPaths.emplace_back(win_text::NarrowUtf8(atlas));
			}

			for (const auto& skel : wstrSkelFiles)
			{
				skelPaths.emplace_back(win_text::NarrowUtf8(skel));
			}

			const auto ExtractFileName = [](const std::wstring& wstrFilePath)
				-> std::wstring
				{
					size_t nPos = wstrFilePath.find_last_of(L"\\/");
					nPos = nPos == std::string::npos ? 0 : nPos + 1;

					size_t nPos2 = wstrFilePath.find(L".", nPos);
					if (nPos2 == std::wstring::npos)nPos2 = wstrFilePath.size();

					return wstrFilePath.substr(nPos, nPos2 - nPos);
				};

			bool isBinarySkel = m_spineSettingDialogue.IsSkelBinary(wstrSkelFiles[0].c_str());
			const auto& atlasFileName = ExtractFileName(wstrSkelFiles[0]);
			LoadSpineFiles(atlasPaths, skelPaths, isBinarySkel, atlasFileName.c_str());
		}
	}
}
/*フォルダ選択*/
void CMainWindow::MenuOnOpenFolder()
{
	if (m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	std::wstring wstrPickedupFolderPath = win_dialogue::SelectWorkFolder(m_hWnd);
	if (!wstrPickedupFolderPath.empty())
	{
		bool bRet = LoadSpineFilesInFolder(wstrPickedupFolderPath.c_str());
		if (bRet)
		{
			ClearFolderPathList();
			win_filesystem::GetFilePathListAndIndex(wstrPickedupFolderPath.c_str(), nullptr, m_folders, &m_nFolderIndex);
		}
	}
}
/*取り込みファイル設定*/
void CMainWindow::MenuOnExtensionSetting()
{
	m_spineSettingDialogue.Open(::GetModuleHandleA(nullptr), m_hWnd, L"Extensions");
}

void CMainWindow::MenuOnImportCocos()
{
	std::wstring wstrSelectedFilePath = win_dialogue::SelectOpenFile(L"Import file", L"*.json", L"Select Cocos import file", m_hWnd);
	if (wstrSelectedFilePath.empty())return;

	std::string strFile = win_filesystem::LoadFileAsString(wstrSelectedFilePath.c_str());
	if (strFile.empty())return;

	size_t nPos = wstrSelectedFilePath.find_last_of(L"\\/");
	if (nPos == std::wstring::npos)return;

	std::vector<std::string> textureDirectories;
	textureDirectories.emplace_back(win_text::NarrowUtf8(wstrSelectedFilePath.substr(0, nPos + 1)));

	std::vector<std::string> atlasData;
	std::vector<std::string> skeletonData;

	constexpr size_t atlasIndices[] = { 5, 0, 2 };
	constexpr size_t nAtlasDepth = sizeof(atlasIndices) / sizeof(atlasIndices)[0];

	constexpr size_t skeletonIndices[] = { 5, 0, 4 };
	constexpr size_t nSkeltonDepth = sizeof(skeletonIndices) / sizeof(skeletonIndices)[0];

	char* p1 = &strFile[0];
	char* p2 = nullptr;
	bool bRet = json_minimal::ExtractArrayValueByIndices(p1, atlasIndices, nAtlasDepth, &p2);
	if (bRet)
	{
		atlasData.push_back(p2);
		free(p2);
		text_utility::ReplaceAll(atlasData[0], "\\r", "\r");
		text_utility::ReplaceAll(atlasData[0], "\\n", "\n");
		text_utility::ReplaceAll(atlasData[0], "\"", "");
	}

	bool isBinarySkel = false;
	bRet = json_minimal::ExtractArrayValueByIndices(p1, skeletonIndices, nSkeltonDepth, &p2);
	if (bRet)
	{
		skeletonData.push_back(p2);
		free(p2);
	}
	else
	{
		/* This is thought to be combination with binary skeleton file. */
		std::wstring wstrBinarySkelFilePath = win_dialogue::SelectOpenFile(L"Import file", L"*.bin;*.skel", L"Select binary skel", m_hWnd);
		if (wstrBinarySkelFilePath.empty())return;

		std::string strBinarySkeletonFile = win_filesystem::LoadFileAsString(wstrBinarySkelFilePath.c_str());
		if (strBinarySkeletonFile.empty())return;

		skeletonData.push_back(std::move(strBinarySkeletonFile));
		isBinarySkel = true;
	}

	bool hadLoaded = m_dxLibSpinePlayer.HasSpineBeenLoaded();
	bRet = m_dxLibSpinePlayer.LoadSpineFromMemory(atlasData, textureDirectories, skeletonData, isBinarySkel);
	PostSpineLoading(hadLoaded, bRet, wstrSelectedFilePath.c_str());
}
/*骨組み操作画面呼び出し*/
void CMainWindow::MenuOnSpineTool()
{
	if (m_spineToolDialogue.GetHwnd() == nullptr)
	{
		HWND hWnd = m_spineToolDialogue.Create(m_hInstance, m_hWnd, L"Spine tool", &m_dxLibSpinePlayer);

		::ShowWindow(hWnd, SW_SHOWNORMAL);
	}
	else
	{
		::SetFocus(m_spineToolDialogue.GetHwnd());
	}
}
/*ファイル追加*/
void CMainWindow::MenuOnAddFile()
{
	if (m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	std::wstring wstrAtlasFile = win_dialogue::SelectOpenFile(L"atlas file", L"*.atlas;*.atlas.txt", L"Select atlas file to add", m_hWnd, true);
	if (wstrAtlasFile.empty())return;

	std::wstring wstrSkeletonFile = win_dialogue::SelectOpenFile(L"skeleton file", L"*.skel;*.bin;*.json;*.txt", L"Select skeleton file to add", m_hWnd, true);
	if (wstrSkeletonFile.empty())return;

	std::string strAtlasFile = win_text::NarrowUtf8(wstrAtlasFile);
	std::string strSkeletonFile = win_text::NarrowUtf8(wstrSkeletonFile);
	bool bBinary = m_spineSettingDialogue.IsSkelBinary(wstrSkeletonFile.c_str());

	m_dxLibSpinePlayer.AddSpineFromFile(strAtlasFile.c_str(), strSkeletonFile.c_str(), bBinary);
}

void CMainWindow::MenuOnExportSetting()
{
	if (m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	m_exportSettingDialogue.Open(::GetModuleHandleA(nullptr), m_hWnd, L"Export setting");
}
/*透過*/
void CMainWindow::MenuOnMakeWindowTransparent()
{
	bool bRet = window_menu::SetMenuCheckState(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), Menu::kSeeThroughImage, !m_isTransparentWindow);
	if (bRet)
	{
		m_isTransparentWindow ^= true;
		LONG lStyleEx = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

		if (m_isTransparentWindow)
		{
			::SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyleEx | WS_EX_LAYERED);
			::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
			::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
		else
		{
			::SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyleEx & ~WS_EX_LAYERED);
			::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
			::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
	}
}
/*手動寸法変更許可切り替え*/
void CMainWindow::MenuOnAllowManualSizing()
{
	bool isResizingAllowed = !m_isManuallyResizable && m_dxLibRecorder.GetState() != CDxLibRecorder::EState::UnderRecording;
	bool bRet = window_menu::SetMenuCheckState(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), Menu::kAllowManualSizing, isResizingAllowed);
	if (bRet)
	{
		m_isManuallyResizable = isResizingAllowed;
		UpdateWindowResizableAttribute();
	}
}
/*拡縮方向反転*/
void CMainWindow::MenuOnReverseZoomDirection()
{
	bool bRet = window_menu::SetMenuCheckState(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), Menu::kReverseZoomDirection, !m_isZoomDirectionReversed);
	if (bRet)
	{
		m_isZoomDirectionReversed ^= true;
	}
}
/* 現在の描画先の大きさに合わせる */
void CMainWindow::MenuOnFiToManualSize()
{
	int iScreenWidth = 0;
	int iScreenHeight = 0;
	DxLib::GetDrawScreenSize(&iScreenWidth, &iScreenHeight);

	const float fSkeletonScale = m_dxLibSpinePlayer.GetSkeletonScale();
	float fBaseWidth = iScreenWidth / fSkeletonScale;
	float fBaseHeight = iScreenHeight / fSkeletonScale;

	m_dxLibSpinePlayer.SetBaseSize(fBaseWidth, fBaseHeight);
	ResizeWindow();
}
/* ファイル情報に合わせる */
void CMainWindow::MenuOnFitToDefaultSize()
{
	m_dxLibSpinePlayer.ResetBaseSize();
	ResizeWindow();
}
/*次のフォルダに移動*/
void CMainWindow::KeyUpOnNextFolder()
{
	if (m_folders.empty() || m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	++m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = 0;
	LoadSpineFilesInFolder(m_folders[m_nFolderIndex].c_str());
}
/*前のフォルダに移動*/
void CMainWindow::KeyUpOnForeFolder()
{
	if (m_folders.empty() || m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	--m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = m_folders.size() - 1;
	LoadSpineFilesInFolder(m_folders[m_nFolderIndex].c_str());
}
/*JPGとして保存*/
void CMainWindow::MenuOnSaveAsJpg()
{
	if (!m_dxLibSpinePlayer.HasSpineBeenLoaded())return;

	std::wstring wstrFilePath = BuildExportFilePath();
	float fTrackTime = 0.f;
	m_dxLibSpinePlayer.GetCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	wstrFilePath += FormatAnimationTime(fTrackTime).append(L".jpg");

	CDxLibImageEncoder::SaveScreenAsJpg(wstrFilePath.c_str());
}
/*PNGとして保存*/
void CMainWindow::MenuOnSaveAsPng()
{
	if (!m_dxLibSpinePlayer.HasSpineBeenLoaded())return;

	std::wstring wstrFilePath = BuildExportFilePath();
	float fTrackTime = 0.f;
	m_dxLibSpinePlayer.GetCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	wstrFilePath += FormatAnimationTime(fTrackTime).append(L".png");

	CDxLibImageEncoder::SaveScreenAsPng(wstrFilePath.c_str());
}
/*記録開始*/
void CMainWindow::MenuOnStartRecording(int menuKind)
{
	if (!m_dxLibSpinePlayer.HasSpineBeenLoaded())return;

	CDxLibRecorder::EOutputType outputType = CDxLibRecorder::EOutputType::Unknown;
	switch (menuKind)
	{
	case PopupMenu::kExportAsGif:
		outputType = CDxLibRecorder::EOutputType::Gif;
		break;
	case PopupMenu::kExportAsVideo:
		outputType = CDxLibRecorder::EOutputType::Video;
		break;
	case PopupMenu::kExportAsPngs:
		outputType = CDxLibRecorder::EOutputType::Pngs;
		break;
	case PopupMenu::kExportAsJpgs:
		outputType = CDxLibRecorder::EOutputType::Jpgs;
	default:
		break;
	}

	if (outputType == CDxLibRecorder::EOutputType::Unknown)return;

	unsigned short fps = menuKind == PopupMenu::kExportAsVideo ?
		m_exportSettingDialogue.GetVideoFps() :
		m_exportSettingDialogue.GetImageFps();

	bool bRet = m_dxLibRecorder.Start(outputType, fps);
	if (!bRet)return;

	/* Disable manual resizing once video recording has started. */
	if (outputType == CDxLibRecorder::EOutputType::Video)
	{
		MenuOnAllowManualSizing();
	}

	if (m_exportSettingDialogue.IsToExportPerAnimation())
	{
		m_dxLibSpinePlayer.RestartAnimation();
	}
}
/*記録終了*/
void CMainWindow::MenuOnEndRecording()
{
	if (m_dxLibRecorder.GetState() == CDxLibRecorder::EState::UnderRecording)
	{
		std::wstring wstrFilePath = BuildExportFilePath();
		m_dxLibRecorder.End(wstrFilePath.c_str());
	}
}
/*表題変更*/
void CMainWindow::ChangeWindowTitle(const wchar_t* pwzTitle)
{
	const wchar_t* pwzName = pwzTitle;
	if (pwzName != nullptr)
	{
		for (;;)
		{
			const wchar_t* pPos = wcspbrk(pwzName, L"\\/");
			if (pPos == nullptr)break;
			pwzName = pPos + 1;
		}
	}

	::SetWindowTextW(m_hWnd, (pwzName == nullptr || *pwzName == L'\0') ? m_swzDefaultWindowName : pwzName);
}
/*表題取得*/
std::wstring CMainWindow::GetWindowTitle() const
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
void CMainWindow::ToggleWindowFrameStyle()
{
	if (!m_dxLibSpinePlayer.HasSpineBeenLoaded() || m_dxLibRecorder.GetState() == CDxLibRecorder::EState::UnderRecording)return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_isFramelessWindow ^= true;

	if (m_isFramelessWindow)
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU & ~WS_THICKFRAME);
		::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		::SetMenu(m_hWnd, nullptr);
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU | (m_isManuallyResizable ? WS_THICKFRAME : 0));
		::SetMenu(m_hWnd, m_hMenuBar);
	}

	ResizeWindow();
}

void CMainWindow::UpdateMenuItemState()
{
	constexpr const unsigned int toolMenuIndices[] = { Menu::kSpineTool, Menu::kAddEffectFile, Menu::kExportSetting };
	constexpr const unsigned int windowMenuIndices[] = { Menu::kSeeThroughImage, Menu::kAllowManualSizing, Menu::kReverseZoomDirection, Menu::kFitToManualSize, Menu::kFitToDefaultSize };

	bool toEnable = m_dxLibSpinePlayer.HasSpineBeenLoaded();

	window_menu::EnableMenuItems(window_menu::GetMenuInBar(m_hWnd, MenuBar::kTool), toolMenuIndices, toEnable);
	window_menu::EnableMenuItems(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), windowMenuIndices, toEnable);
}
/*描画素材設定*/
bool CMainWindow::LoadSpineFilesInFolder(const wchar_t* pwzFolderPath)
{
	if (pwzFolderPath == nullptr)return false;

	std::vector<std::string> atlasPaths;
	std::vector<std::string> skelPaths;

	const std::wstring& wstrAtlasExt = m_spineSettingDialogue.GetAtlasExtension();
	const std::wstring& wstrSkelExt = m_spineSettingDialogue.GetSkelExtension();
	bool isBinarySkel = m_spineSettingDialogue.IsSkelBinary();

	bool isAtlasLonger = wstrAtlasExt.size() > wstrSkelExt.size();

	const std::wstring& wstrLongerExtesion = isAtlasLonger ? wstrAtlasExt : wstrSkelExt;
	const std::wstring& wstrShorterExtension = isAtlasLonger ? wstrSkelExt : wstrAtlasExt;

	std::vector<std::string>& strLongerPaths = isAtlasLonger ? atlasPaths : skelPaths;
	std::vector<std::string>& strShorterPaths = isAtlasLonger ? skelPaths : atlasPaths;

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
			strLongerPaths.emplace_back(win_text::NarrowUtf8(filePath));
		}
		else if (EndsWith(wstrShorterExtension))
		{
			strShorterPaths.emplace_back(win_text::NarrowUtf8(filePath));
		}
	}

	return LoadSpineFiles(atlasPaths, skelPaths, isBinarySkel, pwzFolderPath);
}
/* ファイル取り込み */
bool CMainWindow::LoadSpineFiles(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel, const wchar_t* windowName)
{
	bool hadLoaded = m_dxLibSpinePlayer.HasSpineBeenLoaded();
	bool hasLoaded = m_dxLibSpinePlayer.LoadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
	PostSpineLoading(hadLoaded, hasLoaded, windowName);
	return hasLoaded;
}
/*階層情報消去*/
void CMainWindow::ClearFolderPathList()
{
	m_folders.clear();
	m_nFolderIndex = 0;
}
/* 読み込み後処理*/
void CMainWindow::PostSpineLoading(bool hadLoaded, bool hasLoaded, const wchar_t* windowName)
{
	if (hasLoaded)
	{
		ResizeWindow();
		ChangeWindowTitle(windowName);

		if (CSpineSlotTab::HasSlotExclusionFilter())
		{
			m_dxLibSpinePlayer.SetSlotExcludeCallback(CSpineSlotTab::GetSlotExcludeCallback());
		}

		if (m_spineToolDialogue.GetHwnd() != nullptr)
		{
			m_spineToolDialogue.OnRefresh();
		}

		m_winclock.Restart();
	}
	else
	{
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"Failed to load Spine(s)", m_hWnd);
		ChangeWindowTitle(nullptr);
	}
	if (hadLoaded != hasLoaded)UpdateMenuItemState();
}

std::wstring CMainWindow::BuildExportFilePath()
{
	std::wstring wstrFilePath = win_filesystem::CreateWorkFolder(GetWindowTitle());
	wstrFilePath += win_text::WidenUtf8(m_dxLibSpinePlayer.GetCurrentAnimationName());

	return wstrFilePath;
}
/*時間整形*/
std::wstring CMainWindow::FormatAnimationTime(float fAnimationTime)
{
	wchar_t swBuffer[16]{};
	swprintf_s(swBuffer, L"_%.3f", fAnimationTime);

	return swBuffer;
}
/*記録逓進*/
void CMainWindow::StepRecording()
{
	const auto& recorderState = m_dxLibRecorder.GetState();
	if (recorderState == CDxLibRecorder::EState::UnderRecording)
	{
		float fTrack = 0.f;
		float fEnd = 0.f;
		m_dxLibSpinePlayer.GetCurrentAnimationTime(&fTrack, nullptr, nullptr, &fEnd);

		if (m_exportSettingDialogue.IsToExportPerAnimation())
		{
			if (::isgreater(fTrack, fEnd))
			{
				MenuOnEndRecording();
			}
		}

		const auto& outputType = m_dxLibRecorder.GetOutputType();
		if (outputType == CDxLibRecorder::EOutputType::Pngs || outputType == CDxLibRecorder::EOutputType::Jpgs)
		{
			std::wstring wstrFrameName = FormatAnimationTime(fTrack);
			m_dxLibRecorder.CaptureFrame(wstrFrameName.c_str());
		}
		else
		{
			m_dxLibRecorder.CaptureFrame();
		}
	}
}
/*寸法手動変更可否属性更新*/
void CMainWindow::UpdateWindowResizableAttribute()
{
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	::SetWindowLong(m_hWnd, GWL_STYLE, (m_dxLibSpinePlayer.HasSpineBeenLoaded() && m_isManuallyResizable) ? (lStyle | WS_THICKFRAME) : (lStyle & ~WS_THICKFRAME));
}
/*窓寸法変更*/
void CMainWindow::ResizeWindow()
{
	FPoint2 fBaseSize = m_dxLibSpinePlayer.GetBaseSize();
	float fScale = m_dxLibSpinePlayer.GetCanvasScale();

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	int iX = static_cast<int>(fBaseSize.x * fScale);
	int iY = static_cast<int>(fBaseSize.y * fScale);

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
}
