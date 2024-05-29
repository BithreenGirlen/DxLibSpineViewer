

#include <Windows.h>
#include <CommCtrl.h>

#include <algorithm>

#include "spine_manipulator_dialogue.h"
#include "win_text.h"


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
	if (pwzWindowName != nullptr)m_wstrWindowName = pwzWindowName;
	if (pPlayer != nullptr)m_pDxLibSpinePlayer = pPlayer;
	return ::CreateDialogIndirectParamA(hInstance, (LPCDLGTEMPLATE)m_DialogueTemplate, hWndParent, (DLGPROC)DialogProc, (LPARAM)this);
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
	case WM_CLOSE:
		return OnClose();
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	default:
		break;
	}
	return FALSE;
}
/*WM_INITDIALOG*/
LRESULT CSpineManipulatorDialogue::OnInit(HWND hWnd)
{
	m_hWnd = hWnd;

	::SetWindowText(m_hWnd, m_wstrWindowName.c_str());

	CreateListView(&m_hSlotListView, m_slotColumnNames);
	CreateListView(&m_hSkinListView, m_skinColumnNames);
	CreateListView(&m_hAnimationListView, m_animationColumnNames);

	n_hApplyButton = ::CreateWindowExW(0, WC_BUTTONW, L"Apply", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(Controls::kApplyButton), ::GetModuleHandle(NULL), nullptr);

	ResizeControls();

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

		ConvertAndSort(m_pDxLibSpinePlayer->GetSlotList());
		CreateSingleList(m_hSlotListView, wstrTemps);

		ConvertAndSort(m_pDxLibSpinePlayer->GetSkinList());
		CreateSingleList(m_hSkinListView, wstrTemps);

		ConvertAndSort(m_pDxLibSpinePlayer->GetanimationList());
		CreateSingleList(m_hAnimationListView, wstrTemps);
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
/*WM_COMMAND*/
LRESULT CSpineManipulatorDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int iControlWnd = LOWORD(lParam);
	if (iControlWnd == 0)
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
	std::vector<std::string> checkedItems;

	const auto PickupCheckedItems =
		[this, &checkedItems](HWND hListView)
		-> void
		{
			int iCount = GetListViewItemCount(hListView);
			if (iCount == -1 || iCount == 0)
			{
				checkedItems.clear();
				return;
			}

			if (checkedItems.empty())checkedItems.reserve(iCount);
			else checkedItems.clear();

			for (int i = 0; i < iCount; ++i)
			{
				UINT uiRet = ListView_GetCheckState(hListView, i);
				if (uiRet == 1)
				{
					std::wstring wstr = GetListViewItemText(hListView, i, 0);
					if (!wstr.empty())
					{
						checkedItems.push_back(win_text::NarrowANSI(wstr));
					}
				}
			}
		};

	if (m_pDxLibSpinePlayer != nullptr)
	{
		PickupCheckedItems(m_hSlotListView);
		m_pDxLibSpinePlayer->SetSlotsToExclude(checkedItems);

		PickupCheckedItems(m_hSkinListView);
		if (!checkedItems.empty())
		{
			m_pDxLibSpinePlayer->MixSkins(checkedItems);
		}

		PickupCheckedItems(m_hAnimationListView);
		m_pDxLibSpinePlayer->MixAnimations(checkedItems);
	}
}
/*寸法・位置調整*/
LRESULT CSpineManipulatorDialogue::ResizeControls()
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);

	long clientWidth = rect.right - rect.left;
	long clientHeight = rect.bottom - rect.top;

	long x_space = clientWidth / 96;
	long y_space = clientHeight / 96;

	long x = x_space;
	long y = y_space;
	long w = clientWidth - x_space * 2;
	long h = clientHeight * 3 / 10;

	if (m_hSlotListView != nullptr)
	{
		::MoveWindow(m_hSlotListView, x, y, w, h, TRUE);
		AdjustListViewWidth(m_hSlotListView, static_cast<int>(m_slotColumnNames.size()));
	}
	y += h + y_space;
	h = clientHeight * 1 / 5;
	if (m_hSkinListView != nullptr)
	{
		::MoveWindow(m_hSkinListView, x, y, w, h, TRUE);
		AdjustListViewWidth(m_hSkinListView, static_cast<int>(m_skinColumnNames.size()));
	}
	y += h + y_space;
	h = clientHeight * 3 / 10;
	if (m_hAnimationListView != nullptr)
	{
		::MoveWindow(m_hAnimationListView, x, y, w, h, TRUE);
		AdjustListViewWidth(m_hAnimationListView, static_cast<int>(m_skinColumnNames.size()));
	}
	y += h + y_space;
	h = clientHeight / 16;
	w = clientWidth / 4;
	if (n_hApplyButton != nullptr)
	{
		::MoveWindow(n_hApplyButton, x, y, w, h, TRUE);
	}

	return 0;
}
/*ListView作成*/
void CSpineManipulatorDialogue::CreateListView(HWND* pListViewHandle, const std::vector<std::wstring>& columnNames)
{
	*pListViewHandle = ::CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_ALIGNLEFT | WS_TABSTOP | LVS_SINGLESEL, 0, 0, 0, 0, m_hWnd, nullptr, ::GetModuleHandle(nullptr), nullptr);
	if (*pListViewHandle != nullptr)
	{
		ListView_SetExtendedListViewStyle(*pListViewHandle, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES | LVS_EX_HEADERDRAGDROP);

		LVCOLUMNW lvColumn{};
		lvColumn.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		for (size_t i = 0; i < columnNames.size(); ++i)
		{
			lvColumn.iSubItem = static_cast<int>(i);
			lvColumn.pszText = const_cast<LPWSTR>(columnNames.at(i).data());
			ListView_InsertColumn(*pListViewHandle, i, &lvColumn);
		}
	}
}
/*ListView幅調整*/
void CSpineManipulatorDialogue::AdjustListViewWidth(HWND hListView, int iColumnCount)
{
	if (hListView != nullptr)
	{
		int iScrollWidth = static_cast<int>(::GetSystemMetrics(SM_CXVSCROLL) * ::GetDpiForSystem() / 96.f);

		RECT rect;
		::GetClientRect(hListView, &rect);
		int iWindowWidth = rect.right - rect.left;

		LVCOLUMNW lvColumn{};
		lvColumn.mask = LVCF_WIDTH;
		lvColumn.cx = iWindowWidth / iColumnCount - iScrollWidth;
		for (int i = 0; i < iColumnCount; ++i)
		{
			ListView_SetColumn(hListView, i, &lvColumn);
		}
	}
}
/*項目数取得*/
int CSpineManipulatorDialogue::GetListViewItemCount(HWND hListView)
{
	LRESULT lResult = ::SendMessage(hListView, LVM_GETITEMCOUNT, 0, 0);
	return static_cast<int>(lResult);
}
/*項目追加*/
bool CSpineManipulatorDialogue::AddItemToListView(HWND hListView, const std::vector<std::wstring>& columns, bool bToEnd)
{
	int iItem = GetListViewItemCount(hListView);
	if (iItem == -1)return false;

	LRESULT lResult = -1;
	for (size_t i = 0; i < columns.size(); ++i)
	{
		LVITEM lvItem{};
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;

		lvItem.iItem = bToEnd ? iItem : 0;
		lvItem.iSubItem = static_cast<int>(i);
		lvItem.pszText = const_cast<wchar_t*>(columns.at(i).c_str());

		if (i == 0)
		{
			lResult = ::SendMessage(hListView, LVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&lvItem));
			if (lResult == -1)return false;
			iItem = static_cast<int>(lResult);
		}
		else
		{
			lResult = ::SendMessage(hListView, LVM_SETITEMTEXT, iItem, reinterpret_cast<LPARAM>(&lvItem));
			if (lResult == -1)return false;
		}
	}

	return true;
}
/*リスト項目消去*/
void CSpineManipulatorDialogue::ClearListView(HWND hListView)
{
	ListView_DeleteAllItems(hListView);
}
/*単要素リスト構築*/
void CSpineManipulatorDialogue::CreateSingleList(HWND hListView, const std::vector<std::wstring> &names)
{
	if (hListView != nullptr)
	{
		ClearListView(hListView);
		for (const auto& name : names)
		{
			std::vector<std::wstring> columns;
			columns.push_back(name);
			AddItemToListView(hListView, columns, true);
		}
	}
}
/*指定項目の文字列取得*/
std::wstring CSpineManipulatorDialogue::GetListViewItemText(HWND hListView, int iRow, int iColumn)
{
	std::wstring wstrResult;
	if (hListView != nullptr)
	{
		LV_ITEM lvItem{};
		lvItem.iSubItem = iColumn;

		for (int iSize = 256; iSize < 1025; iSize *= 2)
		{
			std::vector<wchar_t> vBuffer;
			vBuffer.resize(iSize);

			lvItem.cchTextMax = iSize;
			lvItem.pszText = vBuffer.data();
			int iLen = static_cast<int>(::SendMessage(hListView, LVM_GETITEMTEXT, iRow, reinterpret_cast<LPARAM>(&lvItem)));
			if (iLen < iSize - 1)
			{
				wstrResult = vBuffer.data();
				break;
			}
		}
	}
	return wstrResult;
}
