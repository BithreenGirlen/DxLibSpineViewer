
#include <Windows.h>
#include <CommCtrl.h>

#include <algorithm>

#include "main_window.h"

#include "win_filesystem.h"
#include "win_dialogue.h"
#include "win_text.h"
#include "dxlib_image_encoder.h"
#include "dxlib_render_target_scope.h"
#include "json_minimal.h"
#include "text_utility.h"
#include "native-ui/window_menu.h"

#include "../runtime/spine_file_verifier.h"

#include "dxlib-imgui/dxlib_imgui.h"
#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct SuspendWmPaintScope
{
	SuspendWmPaintScope(HWND hWnd)
		:hTargetWnd(hWnd)
	{
		::ValidateRect(hTargetWnd, nullptr);
	}
	~SuspendWmPaintScope()
	{
		::InvalidateRect(hTargetWnd, nullptr, FALSE);
	}

	HWND hTargetWnd;
};

bool CMainWindow::create(HINSTANCE hInstance, const wchar_t* pwzWindowName)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszClassName = m_className;

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;
		const wchar_t* windowName = pwzWindowName == nullptr ? m_defaultWindowName : pwzWindowName;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		m_hWnd = ::CreateWindowW(m_className, windowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			return true;
		}
	}

	return false;
}

int CMainWindow::messageLoop()
{
	MSG msg{};
	/* Unlikely case though, dll for default Spine version is not loaded. */
	if (m_dxLibSpinePlayer.get() == nullptr)return 0;

	for (; msg.message != WM_QUIT;)
	{
		BOOL iRet = m_dxLibSpinePlayer.get()->hasSpineBeenLoaded() ?
			::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) :
			::GetMessageW(&msg, nullptr, 0, 0);
		if (iRet)
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}

		if (m_windowState.hasProcessedWmPaint)
		{
			m_windowState.hasProcessedWmPaint = false;
			continue;
		}

		tick();
	}

	return static_cast<int>(msg.wParam);
}
/* C CALLBACK */
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
		return pThis->handleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/* メッセージ処理 */
LRESULT CMainWindow::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))return 1;

	switch (uMsg)
	{
	case WM_CREATE:
		return onCreate(hWnd);
	case WM_DESTROY:
		return onDestroy();
	case WM_SIZE:
		return onSize(wParam, lParam);
	case WM_CLOSE:
		return onClose();
	case WM_PAINT:
		return onPaint();
	case WM_ERASEBKGND:
		return 1;
	case WM_KEYUP:
		return onKeyUp(wParam, lParam);
	case WM_COMMAND:
		return onCommand(wParam, lParam);
	case WM_MOUSEMOVE:
		return onMouseMove(wParam, lParam);
	case WM_MOUSEWHEEL:
		return onMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return onLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return onLButtonUp(wParam, lParam);
	case WM_RBUTTONUP:
		return onRButtonUp(wParam, lParam);
	case WM_MBUTTONUP:
		return onMButtonUp(wParam, lParam);
	default:
		break;
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/* WM_CREATE */
LRESULT CMainWindow::onCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	initialiseMenuBar();
	updateMenuItemState();

	m_spineToolDatum.pSpinePlayer = &m_dxLibSpinePlayer;

	return 0;
}
/* WM_DESTROY */
LRESULT CMainWindow::onDestroy()
{
	::PostQuitMessage(0);

	return 0;
}
/* WM_CLOSE */
LRESULT CMainWindow::onClose()
{
	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_className, m_hInstance);

	return 0;
}
/* WM_PAINT */
LRESULT CMainWindow::onPaint()
{
	if (!m_dxLibSpinePlayer.get()->hasSpineBeenLoaded())
	{
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(m_hWnd, &ps);
		::EndPaint(m_hWnd, &ps);
	}
	else
	{
		tick();
		m_windowState.hasProcessedWmPaint = true;
	}

	return 0;
}
/* WM_SIZE */
LRESULT CMainWindow::onSize(WPARAM wParam, LPARAM lParam)
{
	int clientWidth = LOWORD(lParam);
	int clientHeight = HIWORD(lParam);

	HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	if (hMonitor != nullptr)
	{
		MONITORINFO monitorInfo{};
		monitorInfo.cbSize = sizeof(MONITORINFO);
		BOOL iRet = ::GetMonitorInfoW(hMonitor, &monitorInfo);
		if (iRet)
		{
			int displayWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			int displayHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

			clientWidth = (std::min)(clientWidth, displayWidth);
			clientHeight = (std::min)(clientHeight, displayHeight);
		}
	}

	DxLib::SetGraphMode(clientWidth, clientHeight, 32);

	m_spineRenderTexture = DxLib::MakeScreen(clientWidth, clientHeight, 1);

	return 0;
}
/* WM_KEYUP */
LRESULT CMainWindow::onKeyUp(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureKeyboard)return 0;

	switch (wParam)
	{
	case VK_ESCAPE:
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		break;
	case VK_UP:
		openForeFolder();
		break;
	case VK_DOWN:
		openNextFolder();
		break;
	case '1':
	case '2':
	case '3':
	case '4':
		if (m_dxLibSpinePlayer.get()->hasSpineBeenLoaded())
		{
			ISpinePlayer::Physics physics = static_cast<ISpinePlayer::Physics>(wParam - '1');
			m_dxLibSpinePlayer.get()->setPhysicsAll(physics);
			m_spineToolDatum.toUpdatePhysicsSelectedItem = true;
		}
		break;
	case 'A':
		m_dxLibSpinePlayer.get()->togglePma();
		break;
	case 'B':
		m_dxLibSpinePlayer.get()->toggleBlendModeAdoption();
		break;
	case 'R':
		m_dxLibSpinePlayer.get()->setDrawOrder(!m_dxLibSpinePlayer.get()->isDrawOrderReversed());
		break;
	default:
		break;
	}
	return 0;
}
/* WM_COMMAND */
LRESULT CMainWindow::onCommand(WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int msgSource = LOWORD(lParam);
	if (msgSource == 0)
	{
		/* Menus */
		switch (id)
		{
		case Menu::kOpenFiles:
			menuOnOpenFiles();
			break;
		case Menu::kOpenFolder:
			menuOnOpenFolder();
			break;
		case Menu::kExtensionSetting:
			menuOnExtensionSetting();
			break;
		case Menu::kImportCocos:
			menuOnImportCocos();
			break;
		case Menu::kShowToolDialogue:
			menuOnShowToolDialogue();
			break;
		case Menu::kAddEffectFile:
			menuOnAddFile();
			break;
		case Menu::kFontSetting:
			menuOnFont();
			break;
		case Menu::kMakeWindowTransparent:
			menuOnMakeWindowTransparent();
			break;
		case Menu::kAllowDraggedResizing:
			menuOnAllowDraggedResizing();
			break;
		case Menu::kReverseZoomDirection:
			menuOnReverseZoomDirection();
			break;
		case Menu::kFitToCurrentFrame:
			menuOnFitToCurrentFrame();
			break;
		case Menu::kFitToDefaultSize:
			menuOnFitToDefaultSize();
			break;
		default: break;
		}
	}
	else
	{
		/* Controls */
	}

	return 0;
}
/* WM_MOUSEMOVE */
LRESULT CMainWindow::onMouseMove(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureMouse)return 0;

	WORD pressedKey = LOWORD(wParam);
	if (pressedKey == MK_LBUTTON)
	{
		if (m_mouseState.wasLeftCombined)return 0;

		POINT pt{};
		::GetCursorPos(&pt);
		if (m_mouseState.wasLeftDragged)
		{
			int deltaX = pt.x - m_mouseState.lastCursorPos.x;
			int deltaY = pt.y - m_mouseState.lastCursorPos.y;
			m_dxLibSpinePlayer.get()->addOffset(-deltaX, -deltaY);
		}

		m_mouseState.lastCursorPos = pt;
		m_mouseState.wasLeftDragged = true;
	}
	else if ((pressedKey & MK_LBUTTON) && (pressedKey & MK_RBUTTON))
	{
		if (m_mouseState.wasLeftCombined || m_mouseState.wasRightCombined)return 0;

		POINT pt{};
		::GetCursorPos(&pt);
		if (m_mouseState.wasRightDragged)
		{
			int deltaX = pt.x - m_mouseState.lastCursorPos.x;
			int deltaY = pt.y - m_mouseState.lastCursorPos.y;

			RECT windowRect{};
			::GetWindowRect(m_hWnd, &windowRect);
			::SetWindowPos(m_hWnd, nullptr, windowRect.left + deltaX, windowRect.top + deltaY, 0, 0, SWP_NOSIZE);
		}

		m_mouseState.lastCursorPos = pt;
		m_mouseState.wasRightDragged = true;
	}

	return 0;
}
/* WM_MOUSEWHEEL */
LRESULT CMainWindow::onMouseWheel(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureMouse)return 0;

	short scroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
	WORD pressedKey = LOWORD(wParam);
	if (pressedKey == MK_LBUTTON)
	{
		static constexpr float kTimeScaleDelta = 0.05f;
		const float scrollSign = scroll > 0 ? 1.f : -1.f;

		float timeScale = m_dxLibSpinePlayer.get()->getTimeScale() + kTimeScaleDelta * scrollSign;
		timeScale = (std::max)(timeScale, 0.f);
		m_dxLibSpinePlayer.get()->setTimeScale(timeScale);

		m_mouseState.wasLeftCombined = true;
	}
	else if (pressedKey == MK_RBUTTON)
	{
		m_dxLibSpinePlayer.get()->shiftSkin();

		m_mouseState.wasRightCombined = true;
	}
	else
	{
		if (m_dxLibSpinePlayer.get()->hasSpineBeenLoaded())
		{
			static constexpr float kScaleDelta = 0.025f;
			static constexpr float kMinScale = 0.15f;
			const float scrollSign = (scroll > 0) ^ m_windowStyle.isZoomReversed ? 1.f : -1.f;

			float skeletonScale = m_dxLibSpinePlayer.get()->getSkeletonScale() + kScaleDelta * scrollSign;
			skeletonScale = (std::max)(kMinScale, skeletonScale);
			m_dxLibSpinePlayer.get()->setSkeletonScale(skeletonScale);

			bool isWindowToBeResized = !(pressedKey & MK_CONTROL) && m_dxLibRecorder.getState() != CDxLibRecorder::EState::UnderRecording;
			if (isWindowToBeResized)
			{
				float canvasScale = m_dxLibSpinePlayer.get()->getCanvasScale() + kScaleDelta * scrollSign;
				canvasScale = (std::max)(kMinScale, canvasScale);
				m_dxLibSpinePlayer.get()->setCanvasScale(canvasScale);

				resizeWindow();
			}
		}
	}

	return 0;
}
/* WM_LBUTTONDOWN */
LRESULT CMainWindow::onLButtonDown(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureMouse)return 0;

	::GetCursorPos(&m_mouseState.lastCursorPos);

	m_mouseState.wasLeftPressed = true;

	return 0;
}
/* WM_LBUTTONUP */
LRESULT CMainWindow::onLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureMouse)return 0;

	if (m_mouseState.wasLeftCombined || m_mouseState.wasLeftDragged)
	{
		m_mouseState.wasLeftDragged = false;
		m_mouseState.wasLeftCombined = false;
		m_mouseState.wasLeftPressed = false;

		return 0;
	}

	WORD pressedKey = LOWORD(wParam);
	if (pressedKey == 0 && m_mouseState.wasLeftPressed)
	{
		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_mouseState.lastCursorPos.x - pt.x;
		int iY = m_mouseState.lastCursorPos.y - pt.y;

		if (iX == 0 && iY == 0)
		{
			const auto& recorderState = m_dxLibRecorder.getState();
			if (recorderState == CDxLibRecorder::EState::Idle || !m_spineToolDatum.toExportPerAnim)
			{
				m_dxLibSpinePlayer.get()->shiftAnimation();
			}
		}
	}

	m_mouseState.wasLeftPressed = false;

	return 0;
}
/* WM_RBUTTONUP */
LRESULT CMainWindow::onRButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureMouse)return 0;

	if (m_mouseState.wasRightCombined || m_mouseState.wasRightDragged)
	{
		m_mouseState.wasRightCombined = false;
		m_mouseState.wasRightDragged = false;

		return 0;
	}

	if (!m_dxLibSpinePlayer.get()->hasSpineBeenLoaded())return 0;

	WORD pressedKey = LOWORD(wParam);
	if (pressedKey == 0)
	{
		const auto& recorderState = m_dxLibRecorder.getState();

		window_menu::CContextMenu contextMenu;
		if (recorderState == CDxLibRecorder::EState::Idle)
		{
			contextMenu.addItems(
				{
					{PopupMenu::kSnapAsPNG, L"Snap as PNG"},
					{PopupMenu::kSnapAsJPG, L"Snap as JPG"},
					{},
					{PopupMenu::kExportAsGif, L"Export as GIF"},
					{PopupMenu::kExportAsVideo, L"Export as H264"}
				});

			if (m_spineToolDatum.toExportPerAnim)
			{
				contextMenu.addItems(
					{
						{},
						{PopupMenu::kExportAsPngs, L"Export as PNGs"},
						{PopupMenu::kExportAsJpgs, L"Export as JPGs"}
					});
			}
		}
		else if (recorderState == CDxLibRecorder::EState::UnderRecording)
		{
			contextMenu.addItems({ {PopupMenu::kEndRecording, L"End recording"} });
		}

		BOOL menuIndex = contextMenu.display(m_hWnd);
		if (menuIndex > 0)
		{
			switch (menuIndex)
			{
			case PopupMenu::kSnapAsPNG:
				saveRenderTextureAsPng();
				break;
			case PopupMenu::kSnapAsJPG:
				saveRenderTextureAsJpg();
				break;
			case PopupMenu::kExportAsGif:
			case PopupMenu::kExportAsVideo:
			case PopupMenu::kExportAsPngs:
			case PopupMenu::kExportAsJpgs:
				startRecording(menuIndex);
				break;
			case PopupMenu::kEndRecording:
				endRecording();
				break;
			default: break;
			}
		}
	}

	return 0;
}
/* WM_MBUTTONUP */
LRESULT CMainWindow::onMButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureMouse)return 0;

	WORD pressedKey = LOWORD(wParam);

	if (pressedKey == 0)
	{
		if (m_dxLibSpinePlayer.get()->hasSpineBeenLoaded() && m_dxLibRecorder.getState() != CDxLibRecorder::EState::UnderRecording)
		{
			m_dxLibSpinePlayer.get()->resetScale();
			resizeWindow();
		}
	}
	else if (pressedKey == MK_RBUTTON)
	{
		toggleWindowBorderStyle();

		m_mouseState.wasRightCombined = true;
	}

	return 0;
}

void CMainWindow::tick()
{
	const auto& recorderState = m_dxLibRecorder.getState();
	if (m_dxLibSpinePlayer.get()->hasSpineBeenLoaded() && recorderState != CDxLibRecorder::EState::InitialisingVideoStream)
	{
		CDxLibImgui::NewFrame();

		float fDelta = m_winclock.getElapsedTime();
		if (recorderState == CDxLibRecorder::EState::UnderRecording)
		{
			fDelta = m_dxLibRecorder.hasFrames() ? 1.f / m_dxLibRecorder.getFps() : 0.f;
		}
		m_dxLibSpinePlayer.get()->update(fDelta);

		DxLib::ClearDrawScreen();

		if (!m_spineRenderTexture.empty())
		{
			{
				DxLibRenderTargetScope dxLibRenderTarget(m_spineRenderTexture.get());
				m_dxLibSpinePlayer.get()->draw();
			}
			DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
			DxLib::DrawGraph(0, 0, m_spineRenderTexture.get(), TRUE);
		}

		stepRecording();

		imGuiSpineToolDialogue();

		m_winclock.restart();

		CDxLibImgui::Render();
		CDxLibImgui::UpdateAndRenderViewPorts();

		DxLib::ScreenFlip();
	}
}
/* 操作欄作成 */
void CMainWindow::initialiseMenuBar()
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
				}).get()
			},
			{0, L"Tool", window_menu::MenuBuilder(
				{
					{Menu::kShowToolDialogue, L"Show tool dialogue"},
					{Menu::kAddEffectFile, L"Add animation effect"},
					{Menu::kFontSetting, L"Font"}
				}).get()
			},
			{0, L"Window", window_menu::MenuBuilder(
				{
					{Menu::kMakeWindowTransparent, L"Make tranparent"},
					{Menu::kAllowDraggedResizing, L"Allow dragged resizing"},
					{Menu::kReverseZoomDirection, L"Reverse zoom direction"},
					{0, L"Base size", window_menu::MenuBuilder(
						{
							{Menu::kFitToCurrentFrame, L"Fit to current frame"},
							{Menu::kFitToDefaultSize, L"Reset to the default"}
						}).get()
					},
				}).get()
			}
		}
	).get();

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

void CMainWindow::menuOnOpenFiles()
{
	if (m_dxLibRecorder.getState() != CDxLibRecorder::EState::Idle)return;

	SuspendWmPaintScope suspenWmPaintScope(m_hWnd);

	std::vector<std::wstring> atlasFilePaths = win_dialogue::SelectOpenFiles(L"atlas files", L"*.atlas;*.atlas.txt", L"Select atlas files", m_hWnd, true);
	if (!atlasFilePaths.empty())
	{
		std::vector<std::wstring> skelFilePaths = win_dialogue::SelectOpenFiles(L"skeleton files", L"*.skel;*.bin;*.json;*.txt", L"Select skeleton files", m_hWnd, true);
		if (!skelFilePaths.empty())
		{
			if (atlasFilePaths.size() != skelFilePaths.size())
			{
				win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"The number of atlas and skeleton files should be the same.", m_hWnd);
				return;
			}

			clearFolderPaths();

			std::sort(atlasFilePaths.begin(), atlasFilePaths.end());
			std::sort(skelFilePaths.begin(), skelFilePaths.end());

			std::vector<std::string> atlasData;
			std::vector<std::string> skelData;
			for (const auto& atlasFilePath : atlasFilePaths)
			{
				atlasData.emplace_back(win_filesystem::LoadFileAsString(atlasFilePath.c_str()));
			}

			for (const auto& skelFilePath : skelFilePaths)
			{
				skelData.emplace_back(win_filesystem::LoadFileAsString(skelFilePath.c_str()));
			}

			const std::wstring& selectedAtlasPath = atlasFilePaths[0];
			size_t nPos1 = selectedAtlasPath.find_last_of(L"\\/");
			if (nPos1 == std::wstring::npos)nPos1 = 0;
			else ++nPos1;

			size_t nPos2 = selectedAtlasPath.find(L".", nPos1);
			if (nPos2 == std::wstring::npos)nPos2 = selectedAtlasPath.size();

			std::string textureDirectory = win_text::NarrowUtf8(&selectedAtlasPath[0], nPos1);
			std::vector<std::string> textureDirectories(atlasData.size(), textureDirectory);

			std::wstring windowName = selectedAtlasPath.substr(nPos1, nPos2 - nPos1);
			loadSpinesFromMemory(atlasData, textureDirectories, skelData, windowName.c_str());
		}
	}
}

void CMainWindow::menuOnOpenFolder()
{
	if (m_dxLibRecorder.getState() != CDxLibRecorder::EState::Idle)return;

	SuspendWmPaintScope suspenWmPaintScope(m_hWnd);

	std::wstring selectedFolderPath = win_dialogue::SelectFolder(nullptr, m_hWnd);
	if (!selectedFolderPath.empty())
	{
		bool bRet = loadSpineFilesInFolder(selectedFolderPath);
		if (bRet)
		{
			clearFolderPaths();
			win_filesystem::GetFilePathListAndIndex(selectedFolderPath, nullptr, m_folderPaths, &m_nFolderPathIndex);
		}
	}
}

void CMainWindow::menuOnExtensionSetting()
{
	m_spineSettingDialogue.open(::GetModuleHandleW(nullptr), m_hWnd, L"Extensions");
}

void CMainWindow::menuOnImportCocos()
{
	SuspendWmPaintScope suspenWmPaintScope(m_hWnd);

	std::wstring selectedFilePath = win_dialogue::SelectOpenFile(L"Import file", L"*.json", L"Select Cocos import file", m_hWnd);
	if (selectedFilePath.empty())return;

	std::string selectedFile = win_filesystem::LoadFileAsString(selectedFilePath.c_str());
	if (selectedFile.empty())return;

	size_t nPos = selectedFilePath.find_last_of(L"\\/");
	if (nPos == std::wstring::npos)nPos = selectedFilePath.size();
	else ++nPos;

	std::vector<std::string> textureDirectories;
	textureDirectories.emplace_back(win_text::NarrowUtf8(&selectedFilePath[0], nPos));

	std::vector<std::string> atlasData;
	std::vector<std::string> skeletonData;

	static constexpr size_t atlasIndices[] = { 5, 0, 2 };
	static constexpr size_t nAtlasDepth = sizeof(atlasIndices) / sizeof(atlasIndices)[0];

	static constexpr size_t skeletonIndices[] = { 5, 0, 4 };
	static constexpr size_t nSkeltonDepth = sizeof(skeletonIndices) / sizeof(skeletonIndices)[0];

	const char* p = &selectedFile[0];
	const char* pStart = nullptr, * pEnd = nullptr;
	bool bRet = json_minimal::FindArrayValueByIndices(p, atlasIndices, nAtlasDepth, &pStart, &pEnd);
	if (bRet)
	{
		if (*pStart == '"' && *(pEnd - 1) == '"')
		{
			++pStart;
			--pEnd;
		}
		std::string atlasDatum(pStart, pEnd);
		text_utility::UnescapeInPlace(atlasDatum);
		atlasData.push_back(std::move(atlasDatum));
	}
	else
	{
		/*
		* Calling ShowErrorMessageValidatingOwnerWindow() in the scope of SuspendWmPaintScope 
		* results in unnecessary ::ValidateRect() calls though,
		* keep it untouched becasue the error reporting is unlikely case.
		*/
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"The selected json seems not to contain atlas file.", m_hWnd);
		return;
	}

	bRet = json_minimal::FindArrayValueByIndices(p, skeletonIndices, nSkeltonDepth, &pStart, &pEnd);
	if (bRet)
	{
		skeletonData.emplace_back(pStart, pEnd);
	}
	else
	{
		/* This is thought to be combination with binary skeleton file. */
		std::wstring binarySkelFilePath = win_dialogue::SelectOpenFile(L"Import file", L"*.bin;*.skel", L"Select binary skeleton", m_hWnd);
		if (binarySkelFilePath.empty())return;

		std::string binarySkeletonFile = win_filesystem::LoadFileAsString(binarySkelFilePath.c_str());
		if (binarySkeletonFile.empty())return;

		skeletonData.push_back(std::move(binarySkeletonFile));
	}

	loadSpinesFromMemory(atlasData, textureDirectories, skeletonData, selectedFilePath.c_str());
}

void CMainWindow::menuOnShowToolDialogue()
{
	m_toShowSpineTool = true;
}

void CMainWindow::menuOnAddFile()
{
	if (m_dxLibRecorder.getState() != CDxLibRecorder::EState::Idle)return;

	SuspendWmPaintScope suspenWmPaintScope(m_hWnd);

	std::wstring selectedAtlasFilePath = win_dialogue::SelectOpenFile(L"atlas file", L"*.atlas;*.atlas.txt", L"Select atlas file to add", m_hWnd, true);
	if (selectedAtlasFilePath.empty())return;

	std::wstring selectedSkeletonFilePath = win_dialogue::SelectOpenFile(L"skeleton file", L"*.skel;*.bin;*.json;*.txt", L"Select skeleton file to add", m_hWnd, true);
	if (selectedSkeletonFilePath.empty())return;

	/* Todo: This will load file twice. */
	std::string skeletonFileDatum = win_filesystem::LoadFileAsString(selectedSkeletonFilePath.c_str());
	using namespace spine_file_verifier;
	SkeletonMetadata skeletonMetaData = VerifySkeletonFileData(reinterpret_cast<const unsigned char*>(skeletonFileDatum.data()), skeletonFileDatum.size());
	if (skeletonMetaData.skeletonFormat == SkeletonFormat::Neither)
	{
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"This seems not to be valid Spine skeleton file.", m_hWnd);
		return;
	}

	long long versionIndex = m_dxLibSpinePlayer.findVersionIndex(reinterpret_cast<const char*>(skeletonMetaData.version));
	if (static_cast<uint8_t>(versionIndex) != m_dxLibSpinePlayer.versionIndexInUse())
	{
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"The file to be added should have the same Spine version as that of being loaded.", m_hWnd);
		return;
	}

	bool isBinarySkel = skeletonMetaData.skeletonFormat == SkeletonFormat::Binary;
	std::string atlasFilePath = win_text::NarrowUtf8(selectedAtlasFilePath);
	std::string skeletonFilePath = win_text::NarrowUtf8(selectedSkeletonFilePath);
	m_dxLibSpinePlayer.get()->addSpineFromFile(atlasFilePath.c_str(), skeletonFilePath.c_str(), isBinarySkel);
}

void CMainWindow::menuOnFont()
{
	if (m_fontSettingDialogue.getHwnd() == nullptr)
	{
		HWND hWnd = m_fontSettingDialogue.open(::GetModuleHandleW(nullptr), m_hWnd, L"Font setting");
		::SendMessage(hWnd, WM_SETICON, ICON_SMALL, ::GetClassLongPtr(m_hWnd, GCLP_HICON));
		::ShowWindow(hWnd, SW_SHOWNORMAL);
	}
	else
	{
		::SetFocus(m_fontSettingDialogue.getHwnd());
	}
}
/* 透過 */
void CMainWindow::menuOnMakeWindowTransparent()
{
	bool bRet = window_menu::SetMenuCheckState(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), Menu::kMakeWindowTransparent, !m_windowStyle.isTransparent);
	if (bRet)
	{
		m_windowStyle.isTransparent ^= true;
		LONG lStyleEx = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

		if (m_windowStyle.isTransparent)
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
/* 手動寸法変更許可切り替え */
void CMainWindow::menuOnAllowDraggedResizing()
{
	bool isResizingAllowed = !m_windowStyle.isResizable && m_dxLibRecorder.getState() != CDxLibRecorder::EState::UnderRecording;
	bool bRet = window_menu::SetMenuCheckState(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), Menu::kAllowDraggedResizing, isResizingAllowed);
	if (bRet)
	{
		m_windowStyle.isResizable = isResizingAllowed;
		updateWindowResizableStyle();
	}
}

void CMainWindow::menuOnReverseZoomDirection()
{
	bool bRet = window_menu::SetMenuCheckState(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), Menu::kReverseZoomDirection, !m_windowStyle.isZoomReversed);
	if (bRet)
	{
		m_windowStyle.isZoomReversed ^= true;
	}
}

void CMainWindow::menuOnFitToCurrentFrame()
{
	if (m_spineRenderTexture.empty())return;

	int iScreenWidth = 0;
	int iScreenHeight = 0;
	DxLib::GetGraphSize(m_spineRenderTexture.get(), &iScreenWidth, &iScreenHeight);

	const float fSkeletonScale = m_dxLibSpinePlayer.get()->getSkeletonScale();
	const float fBaseWidth = iScreenWidth / fSkeletonScale;
	const float fBaseHeight = iScreenHeight / fSkeletonScale;

	m_dxLibSpinePlayer.get()->setBaseSize(fBaseWidth, fBaseHeight);
	resizeWindow();
}

void CMainWindow::menuOnFitToDefaultSize()
{
	m_dxLibSpinePlayer.get()->resetBaseSize();
	resizeWindow();
}

void CMainWindow::clearFolderPaths()
{
	m_folderPaths.clear();
	m_nFolderPathIndex = 0;
}

void CMainWindow::openForeFolder()
{
	if (m_folderPaths.empty() || m_dxLibRecorder.getState() != CDxLibRecorder::EState::Idle)return;

	--m_nFolderPathIndex;
	if (m_nFolderPathIndex >= m_folderPaths.size())m_nFolderPathIndex = m_folderPaths.size() - 1;
	loadSpineFilesInFolder(m_folderPaths[m_nFolderPathIndex]);
}

void CMainWindow::openNextFolder()
{
	if (m_folderPaths.empty() || m_dxLibRecorder.getState() != CDxLibRecorder::EState::Idle)return;

	++m_nFolderPathIndex;
	if (m_nFolderPathIndex >= m_folderPaths.size())m_nFolderPathIndex = 0;
	loadSpineFilesInFolder(m_folderPaths[m_nFolderPathIndex]);
}

void CMainWindow::saveRenderTextureAsJpg()
{
	if (!m_dxLibSpinePlayer.get()->hasSpineBeenLoaded())return;

	std::wstring filePath = buildExportFilePath();
	float fTrackTime = 0.f;
	m_dxLibSpinePlayer.get()->getCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	filePath += formatAnimationTime(fTrackTime);
	filePath += L".jpg";

	dxlib_image_encoder::SaveRenderTextureAsJpg(m_spineRenderTexture.get(), filePath.c_str());
}

void CMainWindow::saveRenderTextureAsPng()
{
	if (!m_dxLibSpinePlayer.get()->hasSpineBeenLoaded())return;

	std::wstring filePath = buildExportFilePath();
	float fTrackTime = 0.f;
	m_dxLibSpinePlayer.get()->getCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	filePath += formatAnimationTime(fTrackTime);
	filePath += L".png";

	dxlib_image_encoder::SaveRenderTextureAsPng(m_spineRenderTexture.get(), filePath.c_str());
}

void CMainWindow::startRecording(int menuKind)
{
	if (!m_dxLibSpinePlayer.get()->hasSpineBeenLoaded())return;

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

	unsigned short fps = outputType == CDxLibRecorder::EOutputType::Video ?
		m_spineToolDatum.iVideoFps :
		m_spineToolDatum.iImageFps;

	if (m_dxLibSpinePlayer.get()->isPaused())m_dxLibSpinePlayer.get()->togglePause();
	if (!m_dxLibSpinePlayer.get()->isVisible())m_dxLibSpinePlayer.get()->toggleVisibility();

	bool bRet = m_dxLibRecorder.start(outputType, fps);
	if (!bRet)return;

	/* Disable manual resizing once video recording has started. */
	if (outputType == CDxLibRecorder::EOutputType::Video)
	{
		menuOnAllowDraggedResizing();
	}

	if (m_spineToolDatum.toExportPerAnim)
	{
		m_dxLibSpinePlayer.get()->restartAnimation();
	}
}

void CMainWindow::stepRecording()
{
	const auto& recorderState = m_dxLibRecorder.getState();
	if (recorderState == CDxLibRecorder::EState::UnderRecording)
	{
		float fTrack = 0.f;
		float fEnd = 0.f;
		m_dxLibSpinePlayer.get()->getCurrentAnimationTime(&fTrack, nullptr, nullptr, &fEnd);

		if (m_spineToolDatum.toExportPerAnim)
		{
			if (::isgreater(fTrack, fEnd))
			{
				endRecording();
			}
		}

		const auto& outputType = m_dxLibRecorder.getOutputType();
		if (outputType == CDxLibRecorder::EOutputType::Pngs || outputType == CDxLibRecorder::EOutputType::Jpgs)
		{
			wchar_t* frameName = formatAnimationTime(fTrack);
			m_dxLibRecorder.commitFrame(m_spineRenderTexture.get(), frameName);
		}
		else
		{
			m_dxLibRecorder.commitFrame(m_spineRenderTexture.get());
		}
	}
}

void CMainWindow::endRecording()
{
	if (m_dxLibRecorder.getState() == CDxLibRecorder::EState::UnderRecording)
	{
		std::wstring filePath = buildExportFilePath();
		m_dxLibRecorder.end(filePath.c_str());
	}
}

void CMainWindow::changeWindowTitle(const wchar_t* windowTitle)
{
	const wchar_t* truncatedWindowTitle = windowTitle;
	if (truncatedWindowTitle != nullptr)
	{
		for (;;)
		{
			const wchar_t* pPos = wcspbrk(truncatedWindowTitle, L"\\/");
			if (pPos == nullptr)break;
			truncatedWindowTitle = pPos + 1;
		}
	}

	::SetWindowTextW(m_hWnd, truncatedWindowTitle == nullptr ? m_defaultWindowName : truncatedWindowTitle);
}

std::wstring CMainWindow::getWindowTitle() const
{
	int iLen = ::GetWindowTextLengthW(m_hWnd);
	if (iLen == 0)return {};

	++iLen;
	std::wstring windowTitle(iLen, L'\0');
	int iWritten = ::GetWindowTextW(m_hWnd, &windowTitle[0], iLen);
	windowTitle.resize(iWritten);

	return windowTitle;
}

int CMainWindow::getWindowTitleToBuffer(wchar_t* dst, size_t dstSize) const
{
	int iLen = ::GetWindowTextLengthW(m_hWnd);
	++iLen;
	if (dstSize < iLen)return 0;

	return ::GetWindowTextW(m_hWnd, dst, iLen);
}

void CMainWindow::toggleWindowBorderStyle()
{
	if (!m_dxLibSpinePlayer.get()->hasSpineBeenLoaded() || m_dxLibRecorder.getState() == CDxLibRecorder::EState::UnderRecording)return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_windowStyle.isBorderless ^= true;

	if (m_windowStyle.isBorderless)
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU & ~WS_THICKFRAME);
		::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		::SetMenu(m_hWnd, nullptr);
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU | (m_windowStyle.isResizable ? WS_THICKFRAME : 0));
		::SetMenu(m_hWnd, m_hMenuBar);
	}

	resizeWindow();
}

void CMainWindow::updateMenuItemState()
{
	constexpr const unsigned int toolMenuIndices[] = { Menu::kShowToolDialogue, Menu::kAddEffectFile };
	constexpr const unsigned int windowIndices[] = { Menu::kMakeWindowTransparent, Menu::kAllowDraggedResizing, Menu::kReverseZoomDirection, Menu::kFitToCurrentFrame, Menu::kFitToDefaultSize };

	bool toEnable = m_dxLibSpinePlayer.get() == nullptr ? false : m_dxLibSpinePlayer.get()->hasSpineBeenLoaded();

	window_menu::EnableMenuItems(window_menu::GetMenuInBar(m_hWnd, MenuBar::kTool), toolMenuIndices, toEnable);
	window_menu::EnableMenuItems(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), windowIndices, toEnable);
}

void CMainWindow::updateWindowResizableStyle()
{
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	::SetWindowLong(m_hWnd, GWL_STYLE, (m_dxLibSpinePlayer.get()->hasSpineBeenLoaded() && m_windowStyle.isResizable) ? (lStyle | WS_THICKFRAME) : (lStyle & ~WS_THICKFRAME));
}

void CMainWindow::resizeWindow()
{
	DxLib::FLOAT2 fBaseSize = m_dxLibSpinePlayer.get()->getBaseSize();
	float fScale = m_dxLibSpinePlayer.get()->getCanvasScale();

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	int iX = static_cast<int>(fBaseSize.u * fScale);
	int iY = static_cast<int>(fBaseSize.v * fScale);

	int monitorWidth = (std::numeric_limits<int32_t>::max)();
	int monitorHeight = (std::numeric_limits<int32_t>::max)();
	HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	if (hMonitor != nullptr)
	{
		MONITORINFO monitorInfo{ sizeof(MONITORINFO) };
		BOOL iRet = ::GetMonitorInfoW(hMonitor, &monitorInfo);
		if (iRet)
		{
			monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
		}
	}

	iX = (std::min)(iX, monitorWidth);
	iY = (std::min)(iY, monitorHeight);

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

bool CMainWindow::loadSpineFilesInFolder(const std::wstring& folderPath)
{
	std::vector<std::string> atlasData;
	std::vector<std::string> skelData;

	const std::wstring& atlasExtention = m_spineSettingDialogue.getAtlasExtension();
	const std::wstring& skelExtention = m_spineSettingDialogue.getSkelExtension();

	bool isAtlasLonger = atlasExtention.size() > skelExtention.size();

	const std::wstring& longerExtesion = isAtlasLonger ? atlasExtention : skelExtention;
	const std::wstring& shorterExtension = isAtlasLonger ? skelExtention : atlasExtention;

	std::vector<std::string>& longerPathData = isAtlasLonger ? atlasData : skelData;
	std::vector<std::string>& shorterPathData = isAtlasLonger ? skelData : atlasData;

	std::vector<std::wstring> filePaths;
	win_filesystem::CreateFilePathList(folderPath.c_str(), L"*", filePaths);

	for (const auto& filePath : filePaths)
	{
		const auto EndsWith = [&filePath](const std::wstring& str)
			-> bool
			{
				if (filePath.size() < str.size()) return false;
				return std::equal(str.rbegin(), str.rend(), filePath.rbegin());
			};

		if (EndsWith(longerExtesion))
		{
			longerPathData.emplace_back(win_filesystem::LoadFileAsString(filePath.c_str()));
		}
		else if (EndsWith(shorterExtension))
		{
			shorterPathData.emplace_back(win_filesystem::LoadFileAsString(filePath.c_str()));
		}
	}

	std::string textureDirectory = win_text::NarrowUtf8(folderPath);
	std::vector<std::string> textureDirectories(atlasData.size(), textureDirectory);

	return loadSpinesFromMemory(atlasData, textureDirectories, skelData, folderPath.c_str());
}

bool CMainWindow::loadSpineFiles(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel, const wchar_t* windowName)
{
	bool hadLoaded = m_dxLibSpinePlayer.get()->hasSpineBeenLoaded();
	bool hasLoaded = m_dxLibSpinePlayer.get()->loadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
	postSpineLoading(hadLoaded, hasLoaded, windowName);

	return hasLoaded;
}

bool CMainWindow::loadSpinesFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skelData, const wchar_t* windowName)
{
	if (skelData.empty())return false;

	const auto& skelDatum = skelData[0];
	using namespace spine_file_verifier;
	SkeletonMetadata skeletonMetaData = VerifySkeletonFileData(reinterpret_cast<const unsigned char*>(skelDatum.data()), skelDatum.size());
	if (skeletonMetaData.skeletonFormat == SkeletonFormat::Neither)
	{
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"The format of skeleton seems not to be valid one.", m_hWnd);
		return false;
	}

	long long versionIndex = m_dxLibSpinePlayer.findVersionIndex(reinterpret_cast<const char*>(skeletonMetaData.version));
	if (versionIndex == static_cast<long long>(CSpinePlayerDynamic::ESpineVersionIndex::NotImplemented))
	{
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"The runtime for this version is not implemented.", m_hWnd);
		return false;
	}
	if (m_dxLibSpinePlayer.getByIndex(versionIndex) == nullptr)
	{
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"The dll for this version is not loaded.", m_hWnd);
		return false;
	}
	m_dxLibSpinePlayer.setPlayerToUse(versionIndex);
	m_dxLibSpinePlayer.get()->enableConversionToPmaOnLoading(m_spineSettingDialogue.isToMultiplyAlphaOnLoading());

	bool isBinarySkel = skeletonMetaData.skeletonFormat == SkeletonFormat::Binary;
	bool hadLoaded = m_dxLibSpinePlayer.get()->hasSpineBeenLoaded();
	bool hasLoaded = m_dxLibSpinePlayer.get()->loadSpineFromMemory(atlasData, textureDirectories, skelData, isBinarySkel);
	postSpineLoading(hadLoaded, hasLoaded, windowName);

	return hasLoaded;
}

void CMainWindow::postSpineLoading(bool hadLoaded, bool hasLoaded, const wchar_t* windowName)
{
	if (hasLoaded)
	{
		resizeWindow();
		changeWindowTitle(windowName);

		if (spine_tool_dialogue::HasSlotExclusionFilter())
		{
			m_dxLibSpinePlayer.get()->setSlotExcludeCallback(spine_tool_dialogue::GetSlotExcludeCallback());
		}

		m_winclock.restart();
	}
	else
	{
		win_dialogue::ShowErrorMessageValidatingOwnerWindow(L"Failed to load Spine(s)", m_hWnd);
		changeWindowTitle(nullptr);
	}
	if (hadLoaded != hasLoaded)updateMenuItemState();
	m_spineToolDatum.hasJustBeenLoaded = hasLoaded;
}

std::wstring CMainWindow::buildExportFilePath()
{
	wchar_t nameBuffer[256]{};
	static constexpr size_t bufferSize = sizeof(nameBuffer) / sizeof(wchar_t) - 1;
	int windowTitleLength = getWindowTitleToBuffer(nameBuffer, bufferSize);

	std::wstring filePath = win_filesystem::CreateSubDirectory(nameBuffer, windowTitleLength);

	const char* pzAnimationName = m_dxLibSpinePlayer.get()->getCurrentAnimationName();
	if (pzAnimationName != nullptr)
	{
		const size_t animationNameLength = ::strlen(pzAnimationName);
		int utf16Length = win_text::WidenUtf8InBuffer(pzAnimationName, animationNameLength, nameBuffer, bufferSize);
		nameBuffer[utf16Length] = L'\0';
		filePath.append(nameBuffer, utf16Length);
	}

	return filePath;
}

wchar_t* CMainWindow::formatAnimationTime(float fAnimationTime, int* length)
{
	static wchar_t s_animationTime[16]{};
	int nWritten = swprintf_s(s_animationTime, L"_%.3f", fAnimationTime);
	if (length != nullptr)*length = nWritten;

	return s_animationTime;
}

void CMainWindow::imGuiSpineToolDialogue()
{
	if (!m_dxLibSpinePlayer.get()->hasSpineBeenLoaded())return;

	if (!m_toShowSpineTool)return;

	if (!m_spineRenderTexture.empty())
	{
		DxLib::GetGraphSize(m_spineRenderTexture.get(), &m_spineToolDatum.iTextureWidth, &m_spineToolDatum.iTextureHeight);
	}

	spine_tool_dialogue::Display(m_spineToolDatum, &m_toShowSpineTool);
	if (m_spineToolDatum.isWindowToBeResized)
	{
		resizeWindow();

		m_spineToolDatum.isWindowToBeResized = false;
	}
	if (m_spineToolDatum.hasJustBeenLoaded)
	{
		m_spineToolDatum.hasJustBeenLoaded = false;
	}
}
