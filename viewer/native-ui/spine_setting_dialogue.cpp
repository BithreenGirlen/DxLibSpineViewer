/* Dialog-box like behaviour input window */

#include <Windows.h>
#include <CommCtrl.h>

#include <vector>

#include "spine_setting_dialogue.h"

CSpineSettingDialogue::CSpineSettingDialogue()
{
	int fontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(fontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Yu mincho");
}

CSpineSettingDialogue::~CSpineSettingDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

bool CSpineSettingDialogue::open(HINSTANCE hInstance, HWND hWnd, const wchar_t* windowName)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszClassName = m_className;

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;

		UINT dpi = ::GetDpiForSystem();
		int windowWidth = ::MulDiv(160, dpi, USER_DEFAULT_SCREEN_DPI);
		int windowHeight = ::MulDiv(160, dpi, USER_DEFAULT_SCREEN_DPI);

		RECT rect{};
		::GetClientRect(hWnd, &rect);
		POINT parentClientPos{ rect.left, rect.top };
		::ClientToScreen(hWnd, &parentClientPos);

		m_hWnd = ::CreateWindowW(m_className, windowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			parentClientPos.x, parentClientPos.y, windowWidth, windowHeight, hWnd, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			messageLoop();
			return true;
		}
	}

	return false;
}

void CSpineSettingDialogue::multiplyAlphaOnLoading(bool toMultiply)
{
	m_toMultiplyAlphaOnLoading = toMultiply;
}

bool CSpineSettingDialogue::isToMultiplyAlphaOnLoading() const noexcept
{
	return m_toMultiplyAlphaOnLoading;
}

int CSpineSettingDialogue::messageLoop()
{
	MSG msg;

	for (;;)
	{
		BOOL iRet = ::GetMessageW(&msg, 0, 0, 0);
		if (iRet > 0)
		{
			if (!::IsDialogMessageW(m_hWnd, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
		}
		else if (iRet == 0)
		{
			return static_cast<int>(msg.wParam);
		}
		else
		{
			return -1;
		}
	}

	return 0;
}
/* C CALLBACK */
LRESULT CSpineSettingDialogue::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSpineSettingDialogue* pThis = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = reinterpret_cast<CSpineSettingDialogue*>(pCreateStruct->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}

	pThis = reinterpret_cast<CSpineSettingDialogue*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (pThis != nullptr)
	{
		return pThis->handleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/* メッセージ処理 */
LRESULT CSpineSettingDialogue::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return onCreate(hWnd);
	case WM_DESTROY:
		return onDestroy();
	case WM_CLOSE:
		return onClose();
	case WM_PAINT:
		return onPaint();
	case WM_SIZE:
		return onSize();
	case WM_COMMAND:
		return onCommand(wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/* WM_CREATE */
LRESULT CSpineSettingDialogue::onCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	::ShowWindow(hWnd, SW_NORMAL);
	::EnableWindow(::GetWindow(m_hWnd, GW_OWNER), FALSE);

	m_atlasStatic.create(L"Atlas", m_hWnd);
	m_atlasEdit.create(m_atlasExtension.c_str(), m_hWnd);

	m_skelStatic.create(L"Skeleton", m_hWnd);
	m_skelEdit.create(m_skelExtension.c_str(), m_hWnd);

	m_pmaButton.create(L"PMA on loading", m_hWnd, reinterpret_cast<HMENU>(Controls::kPmaButton), true);
	m_pmaButton.setCheckBox(m_toMultiplyAlphaOnLoading);

	const auto SetFontCallback = [](HWND hWnd, LPARAM lParam)
		-> BOOL
		{
			::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
			/* TRUE: 続行, FALSE: 終了 */
			return TRUE;
		};

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return 0;
}
/* WM_DESTROY */
LRESULT CSpineSettingDialogue::onDestroy()
{
	::PostQuitMessage(0);

	return 0;
}
/* WM_CLOSE */
LRESULT CSpineSettingDialogue::onClose()
{
	storeInputs();

	HWND hOwnerWnd = ::GetWindow(m_hWnd, GW_OWNER);
	::EnableWindow(hOwnerWnd, TRUE);
	::BringWindowToTop(hOwnerWnd);

	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_className, m_hInstance);

	return 0;
}
/* WM_PAINT */
LRESULT CSpineSettingDialogue::onPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/* WM_SIZE */
LRESULT CSpineSettingDialogue::onSize()
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);

	long clientWidth = rect.right - rect.left;
	long clientHeight = rect.bottom - rect.top;

	long spaceX = clientWidth / 12;
	long spaceY = clientHeight / 48;

	long fontHeight = static_cast<long>(Constants::kFontSize * ::GetDpiForWindow(m_hWnd) / 96.f);

	long x = spaceX;
	long y = spaceY * 2;
	long w = clientWidth * 3 / 4;
	long h = fontHeight;
	::MoveWindow(m_atlasStatic.getHwnd(), x, y, w, h, TRUE);

	y += h;
	h = fontHeight + spaceY * 2;
	::MoveWindow(m_atlasEdit.getHwnd(), x, y, w, h, TRUE);

	y += h + spaceY * 4;
	h = fontHeight;
	::MoveWindow(m_skelStatic.getHwnd(), x, y, w, h, TRUE);

	y += h;
	h = fontHeight + spaceY * 2;
	::MoveWindow(m_skelEdit.getHwnd(), x, y, w, h, TRUE);

	y += h;
	::MoveWindow(m_pmaButton.getHwnd(), x, y, w, h, TRUE);

	return 0;
}
/* WM_COMMAND */
LRESULT CSpineSettingDialogue::onCommand(WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int msgSource = LOWORD(lParam);
	if (msgSource == 0)
	{
		/* Menus */
	}
	else
	{
		/* Controls */

		WORD notificationCode = HIWORD(wParam);
		if (notificationCode == CBN_SELCHANGE)
		{
			/* Notification code */
		}
	}

	return 0;
}

/* 入力値格納 */
void CSpineSettingDialogue::storeInputs()
{
	m_atlasExtension.assign(m_atlasEdit.getText());
	m_skelExtension.assign(m_skelEdit.getText());
	m_toMultiplyAlphaOnLoading = m_pmaButton.isChecked();
}

