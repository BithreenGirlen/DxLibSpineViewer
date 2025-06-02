

#include "export_setting_dialogue.h"

CExportSettingDialogue::CExportSettingDialogue()
{
	int iFontHeight = static_cast<int>(kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(iFontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"yumin");
}

CExportSettingDialogue::~CExportSettingDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

bool CExportSettingDialogue::Open(HINSTANCE hInstance,HWND hOwnerWnd, const wchar_t* windowName)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszClassName = m_className;

	if (::RegisterClassExW(&wcex))
	{
		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(160, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(160, uiDpi, USER_DEFAULT_SCREEN_DPI);

		RECT rect{};
		::GetClientRect(hOwnerWnd, &rect);
		POINT parentClientPos{ rect.left, rect.top };
		::ClientToScreen(hOwnerWnd, &parentClientPos);

		m_hWnd = ::CreateWindowW(m_className, windowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			parentClientPos.x, parentClientPos.y, iWindowWidth, iWindowHeight, hOwnerWnd, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			MessageLoop();
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

int CExportSettingDialogue::MessageLoop()
{
	MSG msg{};

	for (;;)
	{
		BOOL iRet = ::GetMessageW(&msg, 0, 0, 0);
		if (iRet > 0)
		{
			if (!::IsDialogMessage(m_hWnd, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
		}
		else if (iRet == 0)
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

LRESULT CExportSettingDialogue::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CExportSettingDialogue* pThis = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = reinterpret_cast<CExportSettingDialogue*>(pCreateStruct->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}

	pThis = reinterpret_cast<CExportSettingDialogue*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CExportSettingDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	case WM_SIZE:
		return OnSize();
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CExportSettingDialogue::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	::ShowWindow(hWnd, SW_NORMAL);
	::EnableWindow(::GetWindow(m_hWnd, GW_OWNER), FALSE);

	m_imageFpsStatic.Create(L"Imgae FPS", m_hWnd);
	m_imageFpsSpin.Create(m_hWnd, 15, 60);
	m_imageFpsSpin.SetValue(m_imageFps);

	m_videoFpsStatic.Create(L"Video FPS", m_hWnd);
	m_videoFpsSpin.Create(m_hWnd, 15, 60);
	m_videoFpsSpin.SetValue(m_videoFps);

	m_exportMethodButton.Create(L"Export per anim.", m_hWnd, reinterpret_cast<HMENU>(Controls::kExportMethod), true);
	m_exportMethodButton.SetCheckBox(m_isToExportPerAnimation);

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return 0;
}
/*WM_DESTROY*/
LRESULT CExportSettingDialogue::OnDestroy()
{
	::PostQuitMessage(0);

	return 0;
}
/*WM_CLOSE*/
LRESULT CExportSettingDialogue::OnClose()
{
	GetInputs();

	HWND hOwnerWnd = ::GetWindow(m_hWnd, GW_OWNER);
	::EnableWindow(hOwnerWnd, TRUE);
	::BringWindowToTop(hOwnerWnd);

	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_className, ::GetModuleHandle(nullptr));

	return 0;
}

LRESULT CExportSettingDialogue::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	::EndPaint(m_hWnd, &ps);

	return 0;
}

LRESULT CExportSettingDialogue::OnSize()
{
	ResizeControls();

	return 0;
}

LRESULT CExportSettingDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmKind = LOWORD(lParam);
	if (wmKind == 0)
	{
		/*Menus*/
	}
	else
	{
		/*Controls*/
		switch (wmId)
		{
		case Controls::kExportMethod:
			m_exportMethodButton.SetCheckBox(!m_exportMethodButton.IsChecked());
			break;
		}
	}

	return 0;
}

BOOL CExportSettingDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	return TRUE;
}

void CExportSettingDialogue::ResizeControls()
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);

	long clientWidth = rect.right - rect.left;
	long clientHeight = rect.bottom - rect.top;

	long spaceX = clientWidth / 24;
	long spaceY = clientHeight / 48;

	long fontHeight = static_cast<long>(kFontSize * ::GetDpiForWindow(m_hWnd) / 96.f);

	long x = spaceX;
	long y = spaceY * 2;
	long w = clientWidth * 3 / 5;
	long h = fontHeight;
	::MoveWindow(m_imageFpsStatic.GetHwnd(), x, y, w, h, TRUE);

	y += h + spaceY;
	w = fontHeight * 2;
	m_imageFpsSpin.AdjustPosition(x, y, w, h);

	y += h + spaceY;
	w = clientWidth * 3 / 5;
	::MoveWindow(m_videoFpsStatic.GetHwnd(), x, y, w, h, TRUE);

	y += h + spaceY;
	w = fontHeight * 2;
	m_videoFpsSpin.AdjustPosition(x, y, w, h);

	y += h + spaceY;
	w = clientWidth - spaceX * 2;
	::MoveWindow(m_exportMethodButton.GetHwnd(), x, y, w, h, TRUE);
}

void CExportSettingDialogue::GetInputs()
{
	m_imageFps = static_cast<unsigned short>(m_imageFpsSpin.GetValue());
	m_videoFps = static_cast<unsigned short>(m_videoFpsSpin.GetValue());
	m_isToExportPerAnimation = m_exportMethodButton.IsChecked();
}

