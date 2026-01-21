/* Dialog-box like behaviour input window */

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
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszClassName = m_swzClassName;

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(160, uiDpi, USER_DEFAULT_SCREEN_DPI);
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
	}

	return false;
}

bool CSpineSettingDialogue::IsSkelBinary(const wchar_t* pwzFileName) const
{
	switch (m_skeletonFormat)
	{
	case ESkeletonFormat::Automatic:
		return IsLikelyBinary(pwzFileName == nullptr ? m_wstrSkelExtension : pwzFileName);
	case ESkeletonFormat::Binary:
		return true;
	case ESkeletonFormat::Text:
		return false;
	default:
		break;
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
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/* メッセージ処理 */
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
/* WM_CREATE */
LRESULT CSpineSettingDialogue::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	::ShowWindow(hWnd, SW_NORMAL);
	::EnableWindow(::GetWindow(m_hWnd, GW_OWNER), FALSE);

	m_atlasStatic.Create(L"Atlas", m_hWnd);
	m_atlasEdit.Create(m_wstrAtlasExtension.c_str(), m_hWnd);

	m_skelStatic.Create(L"Skeleton", m_hWnd);
	m_skelEdit.Create(m_wstrSkelExtension.c_str(), m_hWnd);

	m_binarySkelStatic.Create(L"Format: ", m_hWnd);
	m_binarySkelComboBox.Create(m_hWnd);
	m_binarySkelComboBox.Setup({ L"Auto", L"Binary", L"Text" });

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return 0;
}
/* WM_DESTROY */
LRESULT CSpineSettingDialogue::OnDestroy()
{
	::PostQuitMessage(0);

	return 0;
}
/* WM_CLOSE */
LRESULT CSpineSettingDialogue::OnClose()
{
	GetInputs();

	HWND hOwnerWnd = ::GetWindow(m_hWnd, GW_OWNER);
	::EnableWindow(hOwnerWnd, TRUE);
	::BringWindowToTop(hOwnerWnd);

	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_swzClassName, m_hInstance);

	return 0;
}
/* WM_PAINT */
LRESULT CSpineSettingDialogue::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/* WM_SIZE */
LRESULT CSpineSettingDialogue::OnSize()
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
	long h = fontHeight + spaceY;
	::MoveWindow(m_atlasStatic.GetHwnd(), x, y, w, h, TRUE);

	y += h;
	::MoveWindow(m_atlasEdit.GetHwnd(), x, y, w, h, TRUE);

	y += h + spaceY * 4;
	::MoveWindow(m_skelStatic.GetHwnd(), x, y, w, h, TRUE);

	y += h;
	::MoveWindow(m_skelEdit.GetHwnd(), x, y, w, h, TRUE);

	y += h;
	w = clientWidth * 1/ 3;
	::MoveWindow(m_binarySkelStatic.GetHwnd(), x, y + spaceY, w, h, TRUE);

	x += w;
	w = clientWidth - x - spaceX;
	::MoveWindow(m_binarySkelComboBox.GetHwnd(), x, y, w, h, TRUE);

	return 0;
}
/*WM_COMMAND*/
LRESULT CSpineSettingDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmKind = LOWORD(lParam);
	if (wmKind == 0)
	{
		/* Menus */
	}
	else
	{
		/* Controls */

		WORD usCode = HIWORD(wParam);
		if (usCode == CBN_SELCHANGE)
		{
			/* Notification code */
			if (reinterpret_cast<HWND>(lParam) == m_binarySkelComboBox.GetHwnd())
			{
				OnSkeletonFormatSelect();
			}
		}
	}

	return 0;
}
/* EnumChildWindows CALLBACK */
BOOL CSpineSettingDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	/* TRUE: 続行, FALSE: 終了 */
	return TRUE;
}

void CSpineSettingDialogue::OnSkeletonFormatSelect()
{
	int iIndex = m_binarySkelComboBox.GetSelectedItemIndex();
	if (iIndex != -1)
	{
		m_skeletonFormat = static_cast<ESkeletonFormat>(iIndex);
	}
}
/* 入力値取得 */
void CSpineSettingDialogue::GetInputs()
{
	m_wstrAtlasExtension.assign(m_atlasEdit.GetText());
	m_wstrSkelExtension.assign(m_skelEdit.GetText());
}

bool CSpineSettingDialogue::IsLikelyBinary(const std::wstring& wstrFileName) const
{
	constexpr const wchar_t* const binaryCandidates[] =
	{
		L".skel", L".bin", L".bytes"
	};

	size_t nPos = wstrFileName.find_last_of(L"\\/");
	nPos = nPos == std::wstring::npos ? 0 : nPos + 1;

	for (const auto& binaryCandidate : binaryCandidates)
	{
		if (wcsstr(&wstrFileName[nPos], binaryCandidate) != nullptr)
		{
			return true;
		}
	}

	return false;
}
