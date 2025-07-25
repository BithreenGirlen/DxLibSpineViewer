
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
	MSG msg{};

	m_winclock.Restart();
	for (; msg.message != WM_QUIT;)
	{
		BOOL iRet = ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);
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
	Tick();
	m_hasProcessedWmPaint = true;

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
		case Menu::kSkeletonSetting:
			MenuOnSkeletonSetting();
			break;
		case Menu::kAtlasSetting:
			MenuOnAtlasSetting();
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
		case Menu::kSnapAsPNG:
			MenuOnSaveAsPng();
			break;
		case Menu::kSnapAsJPG:
			MenuOnSaveAsJpg();
			break;
		case Menu::kExportAsGif:
		case Menu::kExportAsVideo:
		case Menu::kExportAsPngs:
			MenuOnStartRecording(wmId);
			break;
		case Menu::kEndRecording:
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
		const auto PreparePupupMenu = [this](HMENU* hMenu)
			-> bool
			{
				if (hMenu == nullptr)return false;
				HMENU hPopupMenu = *hMenu;

				const auto& recorderState = m_dxLibRecorder.GetState();
				if (recorderState == CDxLibRecorder::EState::Idle)
				{
					int iRet = ::AppendMenuW(hPopupMenu, MF_STRING, Menu::kSnapAsPNG, L"Snap as PNG");
					if (iRet == 0)return false;
					iRet = ::AppendMenuW(hPopupMenu, MF_STRING, Menu::kSnapAsJPG, L"Snap as JPG");
					if (iRet == 0)return false;

					iRet = ::AppendMenuW(hPopupMenu, MF_SEPARATOR, 0, nullptr);
					if (iRet == 0)return false;
					iRet = ::AppendMenuW(hPopupMenu, MF_STRING, Menu::kExportAsGif, L"Export as GIF");
					if (iRet == 0)return false;
					iRet = ::AppendMenuW(hPopupMenu, MF_STRING, Menu::kExportAsVideo, L"Export as H264");
					if (iRet == 0)return false;

					if (m_exportSettingDialogue.IsToExportPerAnimation())
					{
						iRet = ::AppendMenuW(hPopupMenu, MF_SEPARATOR, 0, nullptr);
						if (iRet == 0)return false;
						iRet = ::AppendMenuW(hPopupMenu, MF_STRING, Menu::kExportAsPngs, L"Export as PNGs");
						if (iRet == 0)return false;
					}
				}
				else if (recorderState == CDxLibRecorder::EState::UnderRecording)
				{
					int iRet = ::AppendMenuW(hPopupMenu, MF_STRING, Menu::kEndRecording, L"End recording");
					if (iRet == 0)return false;
				}

				return true;
			};

		HMENU hPopupMenu = ::CreatePopupMenu();
		if (hPopupMenu != nullptr)
		{
			bool bRet = PreparePupupMenu(&hPopupMenu);
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

		StepUpRecording();

		m_winclock.Restart();

		DxLib::ScreenFlip();
	}
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
	HMENU hMenuFile = nullptr;
	HMENU hMenuTool = nullptr;
	HMENU hMenuWindow = nullptr;
	HMENU hMenuBar = nullptr;
	BOOL iRet = FALSE;

	if (m_hMenuBar != nullptr)return;

	hMenuFile = ::CreateMenu();
	if (hMenuFile == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kOpenFiles, "Open files");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuFile, MF_SEPARATOR, 0, nullptr);
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kOpenFolder, "Open folder");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kExtensionSetting, "Extension setting");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuFile, MF_SEPARATOR, 0, nullptr);
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kImportCocos, "Import Cocos");
	if (iRet == 0)goto failed;

	hMenuTool = ::CreateMenu();
	if (hMenuTool == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuTool, MF_STRING, Menu::kSkeletonSetting, "Exclude slot/ Mix anim./ Mix skin");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuTool, MF_STRING, Menu::kAtlasSetting, "Replace attachment");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuTool, MF_STRING, Menu::kAddEffectFile, "Add animation effect");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuTool, MF_STRING, Menu::kExportSetting, "Export setting");
	if (iRet == 0)goto failed;

	hMenuWindow = ::CreateMenu();
	if (hMenuWindow == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuWindow, MF_STRING, Menu::kSeeThroughImage, "Make window tranparent");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuWindow, MF_STRING, Menu::kAllowManualSizing, "Allow manual sizing");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuWindow, MF_STRING, Menu::kReverseZoomDirection, "Reverse zoom direction");
	if (iRet == 0)goto failed;

	hMenuBar = ::CreateMenu();
	if (hMenuBar == nullptr) goto failed;

	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFile), "File");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuTool), "Tool");
	if (iRet == 0)goto failed;
	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuWindow), "Window");
	if (iRet == 0)goto failed;

	iRet = ::SetMenu(m_hWnd, hMenuBar);
	if (iRet == 0)goto failed;

	m_hMenuBar = hMenuBar;

	return;

failed:
	std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
	win_dialogue::ShowErrorMessageValidatingOwnerWindow(wstrMessage.c_str(), m_hWnd);

	if (hMenuFile != nullptr)
	{
		::DestroyMenu(hMenuFile);
	}
	if (hMenuTool != nullptr)
	{
		::DestroyMenu(hMenuTool);
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

	bool bLastState = m_dxLibSpinePlayer.HasSpineBeenLoaded();
	bRet = m_dxLibSpinePlayer.LoadSpineFromMemory(atlasData, textureDirectories, skeletonData, isBinarySkel);
	if (bRet)
	{
		ResizeWindow();
		ChangeWindowTitle(wstrSelectedFilePath.c_str());
	}
	else
	{
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"Failed to load spine from Cocos import file", m_hWnd);
		ChangeWindowTitle(nullptr);
	}
	if (bLastState != bRet)UpdateMenuItemState();
}
/*骨組み操作画面呼び出し*/
void CMainWindow::MenuOnSkeletonSetting()
{
	if (m_spineManipulatorDialogue.GetHwnd() == nullptr)
	{
		HWND hWnd = m_spineManipulatorDialogue.Create(m_hInstance, m_hWnd, L"Spine manipulation", &m_dxLibSpinePlayer);

		::ShowWindow(hWnd, SW_SHOWNORMAL);
	}
	else
	{
		::SetFocus(m_spineManipulatorDialogue.GetHwnd());
	}
}
/*装着変更画面呼び出し*/
void CMainWindow::MenuOnAtlasSetting()
{
	if (m_spineAtlasDialogue.GetHwnd() == nullptr)
	{
		HWND hWnd = m_spineAtlasDialogue.Create(m_hInstance, m_hWnd, L"Spine re-attachment", &m_dxLibSpinePlayer);

		::ShowWindow(hWnd, SW_SHOWNORMAL);
	}
	else
	{
		::SetFocus(m_spineAtlasDialogue.GetHwnd());
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
	bool result = SetMenuCheckState(MenuBar::kWindow, Menu::kSeeThroughImage, !m_isTransparentWindow);
	if (result)
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
	bool result = SetMenuCheckState(MenuBar::kWindow, Menu::kAllowManualSizing, isResizingAllowed);
	if (result)
	{
		m_isManuallyResizable = isResizingAllowed;
		UpdateWindowResizableAttribute();
	}
}
/*拡縮方向反転*/
void CMainWindow::MenuOnReverseZoomDirection()
{
	bool result = SetMenuCheckState(MenuBar::kWindow, Menu::kReverseZoomDirection, !m_isZoomDirectionReversed);
	if (result)
	{
		m_isZoomDirectionReversed ^= true;
	}
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
	wstrFilePath += FormatAnimationTime(fTrackTime) + L".jpg";

	CDxLibImageEncoder::SaveScreenAsJpg(wstrFilePath.c_str());
}
/*PNGとして保存*/
void CMainWindow::MenuOnSaveAsPng()
{
	if (!m_dxLibSpinePlayer.HasSpineBeenLoaded())return;

	std::wstring wstrFilePath = BuildExportFilePath();
	float fTrackTime = 0.f;
	m_dxLibSpinePlayer.GetCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	wstrFilePath += FormatAnimationTime(fTrackTime) + L".png";

	CDxLibImageEncoder::SaveScreenAsPng(wstrFilePath.c_str());
}
/*記録開始*/
void CMainWindow::MenuOnStartRecording(int menuKind)
{
	if (!m_dxLibSpinePlayer.HasSpineBeenLoaded())return;

	CDxLibRecorder::EOutputType outputType = CDxLibRecorder::EOutputType::Unknown;
	switch (menuKind)
	{
	case Menu::kExportAsGif:
		outputType = CDxLibRecorder::EOutputType::Gif;
		break;
	case Menu::kExportAsVideo:
		outputType = CDxLibRecorder::EOutputType::Video;
		break;
	case Menu::kExportAsPngs:
		outputType = CDxLibRecorder::EOutputType::Pngs;
		break;
	default:
		break;
	}

	if (outputType == CDxLibRecorder::EOutputType::Unknown)return;

	unsigned short fps = menuKind == Menu::kExportAsVideo ?
		m_exportSettingDialogue.GetVideoFps() :
		m_exportSettingDialogue.GetImageFps();

	bool bRet = m_dxLibRecorder.Start(outputType, fps);
	if (!bRet)return;

	/* Disable manual resizing once video recording has started. */
	MenuOnAllowManualSizing();

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

		const auto& outputType = m_dxLibRecorder.GetOutputType();
		switch (outputType)
		{
		case CDxLibRecorder::EOutputType::Gif:
			wstrFilePath += L".gif";
			break;
		case CDxLibRecorder::EOutputType::Video:
			wstrFilePath += L".mp4";
			break;
		default:
			break;
		}

		m_dxLibRecorder.End(wstrFilePath.c_str());
	}
}
/*表題変更*/
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
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME);
		::SetMenu(m_hWnd, m_hMenuBar);
	}

	ResizeWindow();
}

bool CMainWindow::SetMenuCheckState(unsigned int uiMenuIndex, unsigned int uiItemIndex, bool checked) const
{
	HMENU hMenuBar = ::GetMenu(m_hWnd);
	if (hMenuBar != nullptr)
	{
		HMENU hMenu = ::GetSubMenu(hMenuBar, uiMenuIndex);
		if (hMenu != nullptr)
		{
			DWORD ulRet = ::CheckMenuItem(hMenu, uiItemIndex, checked ? MF_CHECKED : MF_UNCHECKED);
			return ulRet != (DWORD)-1;
		}
	}

	return false;
}

void CMainWindow::UpdateMenuItemState()
{
	/* Temporal measure. */
	constexpr const unsigned int toolMenuIndices[] = { Menu::kSkeletonSetting, Menu::kAtlasSetting, Menu::kAddEffectFile, Menu::kExportSetting };
	constexpr const unsigned int windowIndices[] = { Menu::kSeeThroughImage, Menu::kAllowManualSizing, Menu::kReverseZoomDirection };

	UINT uiState = m_dxLibSpinePlayer.HasSpineBeenLoaded() ? MF_ENABLED : MF_GRAYED;

	HMENU hMenuBar = ::GetMenu(m_hWnd);
	if (hMenuBar != nullptr)
	{
		HMENU hMenu = ::GetSubMenu(hMenuBar, MenuBar::kTool);
		if (hMenu != nullptr)
		{
			for (unsigned int i = 0; i < sizeof(toolMenuIndices) / sizeof(toolMenuIndices[0]); ++i)
			{
				::EnableMenuItem(hMenu, toolMenuIndices[i], uiState);
			}
		}

		hMenu = ::GetSubMenu(hMenuBar, MenuBar::kWindow);
		if (hMenu != nullptr)
		{
			for (unsigned int i = 0; i < sizeof(windowIndices) / sizeof(windowIndices[0]); ++i)
			{
				::EnableMenuItem(hMenu, windowIndices[i], uiState);
			}
		}
	}
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
	bool bLastState = m_dxLibSpinePlayer.HasSpineBeenLoaded();
	bool bRet = m_dxLibSpinePlayer.LoadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
	if (bRet)
	{
		ResizeWindow();
		ChangeWindowTitle(windowName);

		if (m_spineManipulatorDialogue.HasSlotExclusionFilter())
		{
			m_dxLibSpinePlayer.SetSlotExcludeCallback(m_spineManipulatorDialogue.GetSlotExcludeCallback());
		}
		m_winclock.Restart();
	}
	else
	{
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"Failed to load spine(s)", m_hWnd);
		ChangeWindowTitle(nullptr);
	}
	if (bLastState != bRet)UpdateMenuItemState();

	return bRet;
}
/*階層情報消去*/
void CMainWindow::ClearFolderPathList()
{
	m_folders.clear();
	m_nFolderIndex = 0;
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
void CMainWindow::StepUpRecording()
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

		if (m_dxLibRecorder.GetOutputType() == CDxLibRecorder::EOutputType::Pngs)
		{
			std::wstring wstrFrameName = win_text::WidenUtf8(m_dxLibSpinePlayer.GetCurrentAnimationName());
			wstrFrameName += FormatAnimationTime(fTrack);

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
