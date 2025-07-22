

#include <Windows.h>
#include <CommCtrl.h>

#include <algorithm>
/* Not inclied to use this heavy STL */
//#include <regex>

#include "spine_manipulator_dialogue.h"
#include "dialogue_template.h"
#include "../win_text.h"

/// @brief global data for slot exclusion.
namespace slot_exclusion
{
	static std::string s_filter;
	static bool IsSlotToBeExcluded(const char* szSlotName, size_t nSlotNameLength)
	{
		if (s_filter.empty())return false;

		const char* pEnd = szSlotName + nSlotNameLength;
		return std::search(szSlotName, pEnd, s_filter.begin(), s_filter.end()) != pEnd;
		//return std::regex_search(szSlotName, pEnd, std::regex(s_filter));
	}
};

CSpineManipulatorDialogue::CSpineManipulatorDialogue()
{
	int iFontHeight = static_cast<int>(Constants::kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(iFontHeight, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"yumin");
}

CSpineManipulatorDialogue::~CSpineManipulatorDialogue()
{
	if (m_hFont != nullptr)
	{
		::DeleteObject(m_hFont);
	}
}

HWND CSpineManipulatorDialogue::Create(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, CDxLibSpinePlayer* pPlayer)
{
	CDialogueTemplate sWinDialogueTemplate;
	sWinDialogueTemplate.SetWindowSize(128, 320);
	sWinDialogueTemplate.MakeWindowResizable(true);
	std::vector<unsigned char> dialogueTemplate = sWinDialogueTemplate.Generate(pwzWindowName);

	m_pDxLibSpinePlayer = pPlayer;

	return ::CreateDialogIndirectParamA(hInstance, (LPCDLGTEMPLATE)dialogueTemplate.data(), hWndParent, (DLGPROC)DialogProc, (LPARAM)this);
}

bool CSpineManipulatorDialogue::HasSlotExclusionFilter() const
{
	return !slot_exclusion::s_filter.empty();
}

bool(*CSpineManipulatorDialogue::GetSlotExcludeCallback() const)(const char*, size_t)
{
	return &slot_exclusion::IsSlotToBeExcluded;
}

/*C CALLBACK*/
LRESULT CSpineManipulatorDialogue::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		::SetWindowLongPtr(hWnd, DWLP_USER, lParam);
	}

	CSpineManipulatorDialogue* pThis = reinterpret_cast<CSpineManipulatorDialogue*>(::GetWindowLongPtr(hWnd, DWLP_USER));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
/*メッセージ処理*/
LRESULT CSpineManipulatorDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return OnInit(hWnd);
	case WM_SIZE:
		return OnSize();
	case WM_CLOSE:
		return OnClose();
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk(wParam, lParam);
	default:
		break;
	}
	return FALSE;
}
/*WM_INITDIALOG*/
LRESULT CSpineManipulatorDialogue::OnInit(HWND hWnd)
{
	m_hWnd = hWnd;

	m_slotEdit.Create(win_text::WidenUtf8(slot_exclusion::s_filter).c_str(), m_hWnd);
	m_slotEdit.SetHint(L"Partial slot name to exclude");

	const std::vector<std::wstring> slotColumnNames = { L"Slots to exclude" };
	const std::vector<std::wstring> skinColumnNames = { L"Skins to mix" };
	const std::vector<std::wstring> animationColumnNames = { L"Animations to mix" };

	m_slotListView.Create(m_hWnd, slotColumnNames, true);
	m_skinListView.Create(m_hWnd, skinColumnNames, true);
	m_animationListView.Create(m_hWnd, animationColumnNames, true);

	m_applyButton.Create(L"Apply", m_hWnd, reinterpret_cast<HMENU>(Controls::kApplyButton));

	ResizeControls();
	/* If the dialogue was created with DS_MODALFRAME style, it cannot be resized even if its style has been changed to WS_THICKFRAME */
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_THICKFRAME | DS_MODALFRAME);

	if (m_pDxLibSpinePlayer != nullptr)
	{
		std::vector<std::wstring> wstrTemps;

		const auto ConvertAndSort = 
			[&wstrTemps](const std::vector<std::string>& strTemps)
			-> void
			{
				if (wstrTemps.empty()) wstrTemps.reserve(strTemps.size());
				else wstrTemps.clear();
				for (const auto& temp : strTemps)
				{
					wstrTemps.push_back(win_text::WidenUtf8(temp));
				}
				std::sort(wstrTemps.begin(), wstrTemps.end());
			};

		ConvertAndSort(m_pDxLibSpinePlayer->GetSlotNames());
		m_slotListView.CreateSingleList(wstrTemps);

		ConvertAndSort(m_pDxLibSpinePlayer->GetSkinNames());
		m_skinListView.CreateSingleList(wstrTemps);

		ConvertAndSort(m_pDxLibSpinePlayer->GetAnimationNames());
		m_animationListView.CreateSingleList(wstrTemps);
	}

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

	return TRUE;
}
/*WM_CLOSE*/
LRESULT CSpineManipulatorDialogue::OnClose()
{
	::DestroyWindow(m_hWnd);
	m_hWnd = nullptr;
	return 0;
}
/*WM_SIZE*/
LRESULT CSpineManipulatorDialogue::OnSize()
{
	ResizeControls();
	return 0;
}
/*WM_COMMAND*/
LRESULT CSpineManipulatorDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
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
		case Controls::kApplyButton:
			OnApplyButton();
			break;
		default:
			break;
		}
	}

	return 0;
}
/* WM_LBUTTONDBLCLK */
LRESULT CSpineManipulatorDialogue::OnLButtonDblClk(WPARAM wParam, LPARAM lParam)
{
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	if (lStyle & DS_MODALFRAME)
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~DS_MODALFRAME | WS_THICKFRAME);
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_THICKFRAME | DS_MODALFRAME);
	}
	return 0;
}
/*EnumChildWindows CALLBACK*/
BOOL CSpineManipulatorDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	/*TRUE: 続行, FALSE: 終了*/
	return TRUE;
}
/*適用ボタン押下*/
void CSpineManipulatorDialogue::OnApplyButton()
{
	if (m_pDxLibSpinePlayer != nullptr)
	{
		std::vector<std::string> strCheckedItems;
		std::vector<std::wstring> wstrCheckedItems;
		const auto ConvertCheckedItems = [&strCheckedItems, &wstrCheckedItems]()
			-> void
			{
				strCheckedItems.clear();
				for (const auto& wstrCheckedItem : wstrCheckedItems)
				{
					strCheckedItems.push_back(win_text::NarrowUtf8(wstrCheckedItem));
				}
			};

		const std::wstring wstrSlotExclusionRegex =  m_slotEdit.GetText();
		slot_exclusion::s_filter = win_text::NarrowUtf8(wstrSlotExclusionRegex);
		if (!slot_exclusion::s_filter.empty())
		{
			m_pDxLibSpinePlayer->SetSlotExcludeCallback(&slot_exclusion::IsSlotToBeExcluded);
		}
		else
		{
			m_pDxLibSpinePlayer->SetSlotExcludeCallback(nullptr);

			wstrCheckedItems = m_slotListView.PickupCheckedItems();
			ConvertCheckedItems();
			m_pDxLibSpinePlayer->SetSlotsToExclude(strCheckedItems);
		}

		wstrCheckedItems = m_skinListView.PickupCheckedItems();
		ConvertCheckedItems();
		if (!strCheckedItems.empty())
		{
			m_pDxLibSpinePlayer->MixSkins(strCheckedItems);
		}

		wstrCheckedItems = m_animationListView.PickupCheckedItems();
		ConvertCheckedItems();
		m_pDxLibSpinePlayer->MixAnimations(strCheckedItems);
	}
}
/*寸法・位置調整*/
LRESULT CSpineManipulatorDialogue::ResizeControls()
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);

	long clientWidth = rect.right - rect.left;
	long clientHeight = rect.bottom - rect.top;

	long spaceX = clientWidth / 96;
	long spaceY = clientHeight / 96;

	long fontHeight = static_cast<long>(Constants::kFontSize * ::GetDpiForWindow(m_hWnd) / 96.f);

	long x = spaceX;
	long y = spaceY;
	long w = clientWidth - spaceX * 2;
	long h = fontHeight + spaceY;

	::MoveWindow(m_slotEdit.GetHwnd(), x, y, w, h, TRUE);

	y += h + spaceY;
	h = clientHeight * 11 / 40;
	::MoveWindow(m_slotListView.GetHwnd(), x, y, w, h, TRUE);
	m_slotListView.AdjustWidth();

	y += h + spaceY;
	::MoveWindow(m_skinListView.GetHwnd(), x, y, w, h, TRUE);
	m_skinListView.AdjustWidth();

	y += h + spaceY;
	::MoveWindow(m_animationListView.GetHwnd(), x, y, w, h, TRUE);
	m_animationListView.AdjustWidth();

	y += h + spaceY;
	w = clientWidth / 4;
	h = clientHeight / 16;
	::MoveWindow(m_applyButton.GetHwnd(), x, y, w, h, TRUE);

	return 0;
}
