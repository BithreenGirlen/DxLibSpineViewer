/*Dialog-box like behaviour input window*/

#include <Windows.h>
#include <CommCtrl.h>

#include <vector>

#include "spine_setting_dialogue.h"

CSpineSettingDialogue::CSpineSettingDialogue()
{
	int iFontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(iFontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"yumin");
}

CSpineSettingDialogue::~CSpineSettingDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

bool CSpineSettingDialogue::Open(HINSTANCE hInstance, HWND hWnd, const wchar_t* pwzWindowName)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	//wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_APP));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	//wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_ICON_APP);
	wcex.lpszClassName = m_swzClassName;
	//wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_APP));

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;
		m_hParentWnd = hWnd; // Stores here to avoid calling GetParent() everytime

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(100, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(160, uiDpi, USER_DEFAULT_SCREEN_DPI);

		RECT rect{};
		::GetClientRect(hWnd, &rect);
		POINT parentClientPos{ rect.left, rect.top };
		::ClientToScreen(hWnd, &parentClientPos);

		m_hWnd = ::CreateWindowW(m_swzClassName, pwzWindowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			parentClientPos.x, parentClientPos.y, iWindowWidth, iWindowHeight, hWnd, nullptr, hInstance, this);
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

int CSpineSettingDialogue::MessageLoop()
{
	MSG msg;

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
/*C CALLBACK*/
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
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CSpineSettingDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
LRESULT CSpineSettingDialogue::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	::ShowWindow(hWnd, SW_NORMAL);
	::EnableWindow(m_hParentWnd, FALSE);

	m_hAtlasStatic = ::CreateWindowEx(0, WC_STATIC, L"Atlas", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, m_hWnd, nullptr, m_hInstance, nullptr);
	m_hAtlasEdit = ::CreateWindowEx(0, WC_EDIT, m_wstrAtlasExtension.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, m_hWnd, nullptr, ::GetModuleHandle(NULL), nullptr);

	m_hSkelStatic = ::CreateWindowEx(0, WC_STATIC, L"Skeleton", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, m_hWnd, nullptr, m_hInstance, nullptr);
	m_hSkelEdit = ::CreateWindowEx(0, WC_EDIT, m_wstrSkelExtension.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, m_hWnd, nullptr, ::GetModuleHandle(NULL), nullptr);
	
	m_BinarySkelCheckButton.Create(L"Binary", m_hWnd, reinterpret_cast<HMENU>(Controls::kCheckButton), true);
	m_BinarySkelCheckButton.SetCheckBox(m_bBinarySkel);

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return 0;
}
/*WM_DESTROY*/
LRESULT CSpineSettingDialogue::OnDestroy()
{
	::PostQuitMessage(0);
	return 0;
}
/*WM_CLOSE*/
LRESULT CSpineSettingDialogue::OnClose()
{
	GetInputs();

	::EnableWindow(m_hParentWnd, TRUE);
	::BringWindowToTop(m_hParentWnd);

	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_swzClassName, m_hInstance);

	return 0;
}
/*WM_PAINT*/
LRESULT CSpineSettingDialogue::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/*WM_SIZE*/
LRESULT CSpineSettingDialogue::OnSize()
{
	long clientWidth, clientHeight;
	GetClientAreaSize(&clientWidth, &clientHeight);
	long x_space = clientWidth / 12;
	long y_space = clientHeight / 48;

	long lFontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForWindow(m_hWnd) / 96.f);

	long x = x_space;
	long y = y_space * 4;
	long w = clientWidth * 3 / 4;
	long h = lFontHeight + y_space;
	if (m_hAtlasStatic != nullptr)
	{
		::MoveWindow(m_hAtlasStatic, x, y, w, h, TRUE);
	}
	y += h;
	if (m_hAtlasEdit != nullptr)
	{
		::MoveWindow(m_hAtlasEdit, x, y, w, h, TRUE);
	}
	y += h + y_space * 4;
	if (m_hSkelStatic != nullptr)
	{
		::MoveWindow(m_hSkelStatic, x, y, w, h, TRUE);
	}
	y += h;
	if (m_hSkelEdit != nullptr)
	{
		::MoveWindow(m_hSkelEdit, x, y, w, h, TRUE);
	}
	y += h;
	if (m_BinarySkelCheckButton.GetHwnd() != nullptr)
	{
		::MoveWindow(m_BinarySkelCheckButton.GetHwnd(), x, y, w, h, TRUE);
	}

	return 0;
}
/*WM_COMMAND*/
LRESULT CSpineSettingDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
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
		case Controls::kCheckButton:
			OnCheckButton();
			break;
		}
	}

	return 0;
}
/*描画領域の大きさ取得*/
void CSpineSettingDialogue::GetClientAreaSize(long* width, long* height)
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);
	*width = rect.right - rect.left;
	*height = rect.bottom - rect.top;
}
/*EnumChildWindows CALLBACK*/
BOOL CSpineSettingDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	/*TRUE: 続行, FALSE: 終了*/
	return TRUE;
}

void CSpineSettingDialogue::OnCheckButton()
{
	bool bRet = m_BinarySkelCheckButton.IsChecked();
	m_BinarySkelCheckButton.SetCheckBox(!bRet);
}
/*入力欄文字列取得*/
std::wstring CSpineSettingDialogue::GetEditBoxText(HWND hWnd)
{
	int iLen = ::GetWindowTextLength(hWnd); // 終端を含まない
	if (iLen > 0)
	{
		std::vector<wchar_t> vBuffer(iLen + 1LL, L'\0');
		LRESULT lResult = ::SendMessage(hWnd, WM_GETTEXT, static_cast<WPARAM>(vBuffer.size()), reinterpret_cast<LPARAM>(vBuffer.data()));
		return vBuffer.data();
	}

	return std::wstring();
}
/*入力欄文字列設定*/
bool CSpineSettingDialogue::SetEditBoxText(HWND hWnd, const std::wstring& wstr)
{
	LRESULT lResult = ::SendMessage(hWnd, WM_SETTEXT, wstr.size(), reinterpret_cast<LPARAM>(wstr.data()));
	return lResult == TRUE;
}
/*入力値取得*/
void CSpineSettingDialogue::GetInputs()
{
	std::wstring wstrTemp;
	wstrTemp = GetEditBoxText(m_hAtlasEdit);
	if (!wstrTemp.empty())
	{
		m_wstrAtlasExtension = wstrTemp;
	}
	wstrTemp.clear();
	wstrTemp = GetEditBoxText(m_hSkelEdit);
	if (!wstrTemp.empty())
	{
		m_wstrSkelExtension = wstrTemp;
	}
	m_bBinarySkel = m_BinarySkelCheckButton.IsChecked();
}
