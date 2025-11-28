
#include "spine_tool_dialogue.h"

#include "dialogue_template.h"

CSpineToolDialogue::CSpineToolDialogue()
{
	int iFontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(iFontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Yu mincho");
}

CSpineToolDialogue::~CSpineToolDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

HWND CSpineToolDialogue::Create(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, CDxLibSpinePlayer* pPlayer)
{
	CDialogueTemplate sWinDialogueTemplate;
	sWinDialogueTemplate.MakeWindowResizable(true);
	sWinDialogueTemplate.SetWindowSize(BaseSize::kWidth, BaseSize::kHeight);
	std::vector<unsigned char> dialogueTemplate = sWinDialogueTemplate.Generate(pwzWindowName);

	m_pDxLibSpinePlayer = pPlayer;

	//m_tabs.clear();
	//m_tabs.push_back(std::make_unique<CSpineAnimationTab>());
	//m_tabs.push_back(std::make_unique<CSpineSlotTab>());

	return ::CreateDialogIndirectParamA(hInstance, (LPCDLGTEMPLATE)dialogueTemplate.data(), hWndParent, (DLGPROC)DialogProc, (LPARAM)this);
}

/*C CALLBACK*/
LRESULT CSpineToolDialogue::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		::SetWindowLongPtr(hWnd, DWLP_USER, lParam);
	}

	CSpineToolDialogue* pThis = reinterpret_cast<CSpineToolDialogue*>(::GetWindowLongPtr(hWnd, DWLP_USER));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
/*メッセージ処理*/
LRESULT CSpineToolDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return OnInit(hWnd);
	case WM_SIZE:
		return OnSize();
	case WM_CLOSE:
		return OnClose();
	case WM_NOTIFY:
		return OnNotify(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	default:
		break;
	}
	return FALSE;
}
/*WM_INITDIALOG*/
LRESULT CSpineToolDialogue::OnInit(HWND hWnd)
{
	m_hWnd = hWnd;

	m_tab.Create(m_hWnd);
	for (const auto& tabName : m_tabNames)
	{
		m_tab.Add(tabName);
	}

	ResizeControls();
	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return TRUE;
}
/*WM_CLOSE*/
LRESULT CSpineToolDialogue::OnClose()
{
	if (m_spineAnimationTab.GetHwnd() != nullptr)
	{
		::SendMessage(m_spineAnimationTab.GetHwnd(), WM_CLOSE, 0, 0);
	}

	if (m_spineSkinTab.GetHwnd() != nullptr)
	{
		::SendMessage(m_spineSkinTab.GetHwnd(), WM_CLOSE, 0, 0);
	}

	if (m_spineSlotTab.GetHwnd() != nullptr)
	{
		::SendMessage(m_spineSlotTab.GetHwnd(), WM_CLOSE, 0, 0);
	}

	::DestroyWindow(m_hWnd);
	m_hWnd = nullptr;
	return 0;
}
/*WM_SIZE*/
LRESULT CSpineToolDialogue::OnSize()
{
	ResizeControls();
	return 0;
}
/* WM_NOTIFY */
LRESULT CSpineToolDialogue::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pNmhdr = reinterpret_cast<LPNMHDR>(lParam);
	if (pNmhdr != nullptr)
	{
		if (pNmhdr->code == TCN_SELCHANGE)
		{
			int index = m_tab.GetSelectedTabIndex();
			if (m_hLastTab != nullptr)
			{
				::ShowWindow(m_hLastTab, SW_HIDE);
			}
			HWND hWnd = nullptr;
			switch (index)
			{
			case Tab::Animation:
				hWnd = m_spineAnimationTab.GetHwnd();
				if (hWnd == nullptr)
				{
					hWnd = m_spineAnimationTab.Create(::GetModuleHandle(nullptr), m_hWnd, GenerateTabPageDialogueTemplate(L"Animation").data(), m_hFont, m_pDxLibSpinePlayer);
				}
				break;
			case Tab::Skin:
				hWnd = m_spineSkinTab.GetHwnd();
				if (hWnd == nullptr)
				{
					hWnd = m_spineSkinTab.Create(::GetModuleHandle(nullptr), m_hWnd, GenerateTabPageDialogueTemplate(L"Skin").data(), m_hFont, m_pDxLibSpinePlayer);
				}
				break;
			case Tab::Slot:
				hWnd = m_spineSlotTab.GetHwnd();
				if (hWnd == nullptr)
				{
					hWnd = m_spineSlotTab.Create(::GetModuleHandle(nullptr), m_hWnd, GenerateTabPageDialogueTemplate(L"Slot").data(), m_hFont, m_pDxLibSpinePlayer);
				}
				break;
			default:
				break;
			}

			if (hWnd != nullptr)
			{
				::ShowWindow(hWnd, SW_SHOWNORMAL);
				m_hLastTab = hWnd;
				ResizeControls();
			}
		}
	}
	return 0;
}
/*WM_COMMAND*/
LRESULT CSpineToolDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
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
	}

	return 0;
}
/*EnumChildWindows CALLBACK*/
BOOL CSpineToolDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	/*TRUE: 続行, FALSE: 終了*/
	return TRUE;
}
/*寸法・位置調整*/
LRESULT CSpineToolDialogue::ResizeControls()
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);

	long clientWidth = rect.right - rect.left;
	long clientHeight = rect.bottom - rect.top;

	long fontHeight = static_cast<long>(Constants::kFontSize * ::GetDpiForWindow(m_hWnd) / 96.f);

	long x = 0;
	long y = 0;
	long w = clientWidth;
	long h = static_cast<long>(fontHeight * 1.5);

	::MoveWindow(m_tab.GetHwnd(), x, y, w, h, TRUE);
	m_tab.Adjust();

	if (m_hLastTab != nullptr)
	{
		int tabHeight = m_tab.GetItemHeight();
		::MoveWindow(m_hLastTab, rect.left, rect.top + tabHeight, clientWidth, clientHeight - tabHeight, TRUE);
		::InvalidateRect(m_hLastTab, nullptr, TRUE);
	}

	return 0;
}

std::vector<unsigned char> CSpineToolDialogue::GenerateTabPageDialogueTemplate(const wchar_t* windowName)
{
	CDialogueTemplate sWinDialogueTemplate;
	sWinDialogueTemplate.SetWindowSize(BaseSize::kWidth, BaseSize::kHeight);
	sWinDialogueTemplate.MakeWindowChild(true);
	return sWinDialogueTemplate.Generate(windowName);
}
