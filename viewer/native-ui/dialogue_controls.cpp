
#include "dialogue_controls.h"


/* ==================== ListView ====================　*/


bool CListView::create(HWND hParentWnd, const wchar_t** columnNames, size_t columnCount, bool hasCheckBox)
{
	m_hWnd = ::CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_ALIGNLEFT | WS_TABSTOP | LVS_SINGLESEL, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(nullptr), nullptr);
	if (m_hWnd != nullptr)
	{
		::SendMessageW(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | (hasCheckBox ? LVS_EX_CHECKBOXES : 0) | LVS_EX_HEADERDRAGDROP);

		LVCOLUMNW lvColumn{};
		lvColumn.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		for (size_t i = 0; i < columnCount; ++i)
		{
			lvColumn.iSubItem = static_cast<int>(i);
			lvColumn.pszText = const_cast<LPWSTR>(columnNames[i]);
			::SendMessageW(m_hWnd, LVM_INSERTCOLUMN, i, reinterpret_cast<LPARAM>(&lvColumn));
		}
	}
	return m_hWnd != nullptr;
}

void CListView::adjustWidth()
{
	if (m_hWnd != nullptr)
	{
		int iColumnCount = getColumnCount();
		if (iColumnCount != -1)
		{
			RECT rect;
			::GetClientRect(m_hWnd, &rect);
			int iWindowWidth = rect.right - rect.left;

			LVCOLUMNW lvColumn{};
			lvColumn.mask = LVCF_WIDTH;
			lvColumn.cx = iWindowWidth / iColumnCount;
			for (int i = 0; i < iColumnCount; ++i)
			{
				::SendMessageW(m_hWnd, LVM_SETCOLUMN, i, reinterpret_cast<LPARAM>(&lvColumn));
			}
		}
	}
}

bool CListView::add(const wchar_t** columns, size_t columnCount, bool toBottom)
{
	if (m_hWnd == nullptr)return false;

	int iItem = getItemCount();
	if (iItem == -1)return false;

	LRESULT lResult = -1;
	for (size_t i = 0; i < columnCount; ++i)
	{
		LVITEMW lvItem{};
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;

		lvItem.iItem = toBottom ? iItem : 0;
		lvItem.iSubItem = static_cast<int>(i);
		lvItem.pszText = const_cast<wchar_t*>(columns[i]);

		if (i == 0)
		{
			lResult = ::SendMessageW(m_hWnd, LVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&lvItem));
			if (lResult == -1)return false;
			iItem = static_cast<int>(lResult);
		}
		else
		{
			lResult = ::SendMessageW(m_hWnd, LVM_SETITEMTEXT, iItem, reinterpret_cast<LPARAM>(&lvItem));
			if (lResult == -1)return false;
		}
	}

	return true;
}

bool CListView::add(const std::vector<std::wstring>& columns, bool toBottom)
{
	size_t nSize = columns.size();
	if (nSize == 0)return false;

	const wchar_t** pBuffer = static_cast<const wchar_t**>(malloc(nSize * sizeof(wchar_t*)));
	if (pBuffer == nullptr)return false;

	for (size_t i = 0; i < nSize; ++i)
	{
		pBuffer[i] = columns[i].data();
	}
	bool bRet = add(pBuffer, nSize);
	free(pBuffer);

	return bRet;
}

void CListView::clear() const
{
	if (m_hWnd != nullptr)
	{
		::SendMessageW(m_hWnd, LVM_DELETEALLITEMS, 0, 0);
	}
}

void CListView::createSingleList(const std::vector<std::wstring>& items)
{
	if (m_hWnd != nullptr)
	{
		clear();
		for (const auto& item : items)
		{
			const wchar_t* pData = item.data();
			const wchar_t** singleValue = &pData;
			add(singleValue, 1);
		}
	}
}
void CListView::createSingleList(const wchar_t** items, size_t itemCount)
{
	if (m_hWnd != nullptr)
	{
		clear();
		for (size_t i = 0; i < itemCount; ++i)
		{
			const wchar_t** singleValue = &items[i];
			add(singleValue, 1);
		}
	}
}

std::vector<std::wstring> CListView::pickupCheckedItems()
{
	std::vector<std::wstring> checkedItems;
	if (m_hWnd != nullptr)
	{
		int iCount = getItemCount();
		if (iCount != -1 && iCount != 0)
		{
			checkedItems.reserve(iCount);
			for (int i = 0; i < iCount; ++i)
			{
				UINT uiRet = ListView_GetCheckState(m_hWnd, i);
				if (uiRet == 1)
				{
					std::wstring wstr = getItemText(i, 0);
					if (!wstr.empty())
					{
						checkedItems.push_back(std::move(wstr));
					}
				}
			}
		}
	}
	return checkedItems;
}

int CListView::getColumnCount() const
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessageW(m_hWnd, LVM_GETHEADER, 0, 0);
		if (lResult != 0)
		{
			HWND hHeaderWnd = reinterpret_cast<HWND>(lResult);

			lResult = ::SendMessageW(hHeaderWnd, HDM_GETITEMCOUNT, 0, 0);
			return static_cast<int>(lResult);
		}
	}
	return -1;
}

int CListView::getItemCount() const
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessageW(m_hWnd, LVM_GETITEMCOUNT, 0, 0);
		return static_cast<int>(lResult);
	}
	return -1;
}

std::wstring CListView::getItemText(int iRow, int iColumn) const
{
	std::wstring result;
	if (m_hWnd != nullptr)
	{
		LV_ITEMW lvItem{};
		lvItem.iSubItem = iColumn;

		for (int iSize = 256; iSize < 1025; iSize *= 2)
		{
			result.resize(iSize, L'\0');

			lvItem.cchTextMax = iSize;
			lvItem.pszText = &result[0];
			int iLen = static_cast<int>(::SendMessageW(m_hWnd, LVM_GETITEMTEXT, iRow, reinterpret_cast<LPARAM>(&lvItem)));
			if (iLen < iSize - 1)
			{
				result.resize(iLen);
				break;
			}
		}
	}
	return result;
}

/* ==================== ListBox ==================== */
/*
* ListBox lacks the equivalent to ListView's LVS_EX_DOUBLEBUFFER,
* so takes longer time than ListView in its scrolling.
*/


bool CListBox::create(HWND hParentWnd)
{
	m_hWnd = ::CreateWindowExW(0, WC_LISTBOXW, L"ListBox", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_SORT | LBS_NOINTEGRALHEIGHT | WS_VSCROLL, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(nullptr), nullptr);
	return m_hWnd != nullptr;
}

void CListBox::add(const wchar_t* text, bool toBottom) const
{
	if (m_hWnd != nullptr)
	{
		if (toBottom)
		{
			::SendMessageW(m_hWnd, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text));
		}
		else
		{
			::SendMessageW(m_hWnd, LB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(text));
		}
	}
}

void CListBox::clear() const
{
	if (m_hWnd != nullptr)
	{
		::SendMessageW(m_hWnd, LB_RESETCONTENT, 0, 0);
	}
}

std::wstring CListBox::getSelectedItemName()
{
	if (m_hWnd != nullptr)
	{
		long long llSelected = getSelectedItemIndex();
		if (llSelected != LB_ERR)
		{
			LRESULT length = ::SendMessageW(m_hWnd, LB_GETTEXTLEN, static_cast<WPARAM>(llSelected), 0);
			if (length != LB_ERR)
			{
				std::wstring result(static_cast<size_t>(length) + 1, L'\0');
				LRESULT written = ::SendMessageW(m_hWnd, LB_GETTEXT, static_cast<WPARAM>(llSelected), reinterpret_cast<LPARAM>(&result[0]));
				if (written != LB_ERR)
				{
					result.resize(written);
					return result;
				}
			}
		}
	}
	return {};
}

long long CListBox::getSelectedItemIndex() const
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessageW(m_hWnd, LB_GETCURSEL, 0, 0);

		return lResult;
	}
	return LB_ERR;
}

/* ==================== ComboBox ==================== */


bool CComboBox::create(HWND hParentWnd)
{
	m_hWnd = ::CreateWindowExW(0, WC_COMBOBOXW, L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(NULL), nullptr);
	return m_hWnd != nullptr;
}

void CComboBox::setup(const std::vector<std::wstring>& itemTexts)
{
	clear();

	if (m_hWnd != nullptr)
	{
		for (const auto& itemText : itemTexts)
		{
			::SendMessageW(m_hWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(itemText.c_str()));
		}
		setSelectedItem(0);
	}
}

void CComboBox::setup(const wchar_t** itemTexts, size_t itemCount)
{
	clear();

	if (m_hWnd != nullptr)
	{
		for (size_t i = 0; i < itemCount; ++i)
		{
			::SendMessageW(m_hWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(itemTexts[i]));
		}
		setSelectedItem(0);
	}
}

int CComboBox::getSelectedItemIndex() const
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessageW(m_hWnd, CB_GETCURSEL, 0, 0);
		return static_cast<int>(lResult);
	}
	return CB_ERR;
}

std::wstring CComboBox::getSelectedItemText() const
{
	if (m_hWnd != nullptr)
	{
		int selected = getSelectedItemIndex();
		if (selected != CB_ERR)
		{
			LRESULT length = ::SendMessageW(m_hWnd, CB_GETLBTEXTLEN, static_cast<WPARAM>(selected), 0);
			if (length != CB_ERR)
			{
				std::wstring result(length + 1, L'\0');
				LRESULT written = ::SendMessageW(m_hWnd, CB_GETLBTEXT, static_cast<WPARAM>(selected), reinterpret_cast<LPARAM>(&result[0]));
				if (written != CB_ERR)
				{
					result.resize(written);
					return result;
				}
			}
		}
	}

	return {};
}

int CComboBox::findIndex(const wchar_t* itemName) const
{
	if (m_hWnd != nullptr)
	{
		return static_cast<int>(::SendMessageW(m_hWnd, CB_FINDSTRING, -1, reinterpret_cast<LPARAM>(itemName)));
	}
	return CB_ERR;
}

bool CComboBox::setSelectedItem(int itemIndex) const
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessageW(m_hWnd, CB_SETCURSEL, itemIndex, 0);
		return itemIndex == -1 ? lResult == CB_ERR : lResult == itemIndex;
	}
	return false;
}

void CComboBox::clear() const
{
	if (m_hWnd != nullptr)
	{
		::SendMessageW(m_hWnd, CB_RESETCONTENT, 0, 0);
	}
}

/* ==================== Button ==================== */


bool CButton::create(const wchar_t* text, HWND hParentWnd, HMENU hMenu, bool hasCheckBox)
{
	m_hWnd = ::CreateWindowExW(0, WC_BUTTONW, text, WS_VISIBLE | WS_CHILD | WS_TABSTOP | (hasCheckBox ? BS_CHECKBOX | BS_AUTOCHECKBOX : 0), 0, 0, 0, 0, hParentWnd, hMenu, ::GetModuleHandle(NULL), nullptr);
	return m_hWnd != nullptr;
}

void CButton::setCheckBox(bool checked) const
{
	if (m_hWnd != nullptr)
	{
		/* BM_SETCHECK always return 0 */
		::SendMessageW(m_hWnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
	}
}

bool CButton::isChecked() const
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessageW(m_hWnd, BM_GETCHECK, 0, 0);
		return lResult == BST_CHECKED;
	}

	return false;
}

/* ==================== Integer trackbar ==================== */


bool CSlider::create(const wchar_t* text, HWND hParentWnd, HMENU hMenu, unsigned short usMin, unsigned short usMax, unsigned int uiRange, bool toBeVertical)
{
	m_hWnd = ::CreateWindowExW(
		0, TRACKBAR_CLASSW, text,
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_TOOLTIPS | TBS_BOTH | (toBeVertical ? TBS_VERT : 0),
		0, 0, 0, 0, hParentWnd, hMenu, ::GetModuleHandleA(nullptr), nullptr
	);

	if (m_hWnd != nullptr)
	{
		::SendMessage(m_hWnd, TBM_SETRANGE, TRUE, MAKELONG(usMin, usMax));
		::SendMessage(m_hWnd, TBM_SETPAGESIZE, TRUE, uiRange);
	}

	return m_hWnd != nullptr;
}

long long CSlider::getPosition() const
{
	return ::SendMessageW(m_hWnd, TBM_GETPOS, 0, 0);
}

void CSlider::setPosition(long long llPos) const
{
	::SendMessageW(m_hWnd, TBM_SETPOS, TRUE, llPos);
}

HWND CSlider::getToolTipHandle() const
{
	return reinterpret_cast<HWND>(::SendMessageW(m_hWnd, TBM_GETTOOLTIPS, 0, 0));
}

/* ==================== Float trackBar ==================== */


bool CFloatSlider::create(const wchar_t* text, HWND hParentWnd, HMENU hMenu, float fMin, float fMax, float fRange, unsigned int uiRatio, bool toBeVertical)
{
	m_hWnd = ::CreateWindowExW(
		0, TRACKBAR_CLASSW, text,
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_TOOLTIPS | TBS_BOTH | (toBeVertical ? TBS_VERT : 0),
		0, 0, 0, 0, hParentWnd, hMenu, ::GetModuleHandleA(nullptr), nullptr
	);

	if (uiRatio > 0)m_uiRatio = uiRatio;

	if (m_hWnd != nullptr)
	{
		unsigned int uiMin = static_cast<unsigned int>(fMin * m_uiRatio);
		unsigned int uiMax = static_cast<unsigned int>(fMax * m_uiRatio);
		unsigned int uiRange = static_cast<unsigned int>(fRange * m_uiRatio);

		::SendMessageW(m_hWnd, TBM_SETRANGE, TRUE, MAKELONG(uiMin, uiMax));
		::SendMessageW(m_hWnd, TBM_SETPAGESIZE, TRUE, uiRange);
	}

	return m_hWnd != nullptr;
}

float CFloatSlider::getPosition() const
{
	return ::SendMessageW(m_hWnd, TBM_GETPOS, 0, 0) / static_cast<float>(m_uiRatio);
}

void CFloatSlider::setPosition(float fPos) const
{
	::SendMessageW(m_hWnd, TBM_SETPOS, TRUE, static_cast<LPARAM>(fPos * m_uiRatio));
}

HWND CFloatSlider::getToolTipHandle() const
{
	return reinterpret_cast<HWND>(::SendMessageW(m_hWnd, TBM_GETTOOLTIPS, 0, 0));
}

void CFloatSlider::onToolTipNeedText(LPNMTTDISPINFOW pNmtTextDispInfo) const
{
	if (pNmtTextDispInfo != nullptr)
	{
		long n = 0;
		for (long l = m_uiRatio; l > 1; ++n, l = l / kDefaultRatio);
		/* LPNMTTDISPINFOW::szText is 80 chars in length. */
		swprintf_s(pNmtTextDispInfo->szText, L"%0.*f", n, getPosition());
	}

}

/* ==================== Static ==================== */


bool CStatic::create(const wchar_t* text, HWND hParentWnd, bool hasEdge)
{
	m_hWnd = ::CreateWindowExW(0, WC_STATICW, text, WS_VISIBLE | WS_CHILD | (hasEdge ? SS_ETCHEDHORZ : 0), 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(NULL), nullptr);

	return m_hWnd != nullptr;
}

/* ==================== Edit ==================== */


bool CEdit::create(const wchar_t* initialText, HWND hParentWnd, bool toBeReadOnly, bool hasBorder, bool onlyDigits, bool passwordField)
{
	m_hWnd = ::CreateWindowExW(0, WC_EDITW, initialText,
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | (toBeReadOnly ? ES_READONLY : 0x00) | (hasBorder ? WS_BORDER : 0x00) | (onlyDigits ? ES_NUMBER : 0x00) | (passwordField ? ES_PASSWORD : 0x00),
		0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(NULL), nullptr);
	return m_hWnd != nullptr;
}

std::wstring CEdit::getText() const
{
	int iLen = ::GetWindowTextLengthW(m_hWnd);
	if (iLen == 0)return {};
	++iLen;
	std::wstring result(iLen, L'\0');
	LRESULT lWritten = ::SendMessageW(m_hWnd, WM_GETTEXT, static_cast<WPARAM>(result.size()), reinterpret_cast<LPARAM>(&result[0]));
	result.resize(lWritten);

	return result;
}

bool CEdit::setText(size_t textLength, const wchar_t* text) const
{
	LRESULT lResult = ::SendMessageW(m_hWnd, WM_SETTEXT, textLength, reinterpret_cast<LPARAM>(text));
	return lResult == TRUE;
}

bool CEdit::setHint(const wchar_t* text, bool toBeHidden) const
{
	LRESULT lResult = ::SendMessageW(m_hWnd, EM_SETCUEBANNER, toBeHidden ? TRUE : FALSE, reinterpret_cast<LPARAM>(text));
	return lResult == TRUE;
}

/* ==================== Up-down control ==================== */


bool CSpin::create(HWND hParentWnd, unsigned short usMin, unsigned short usMax)
{
	m_buddy.create(L"", hParentWnd, false, true, true, false);

	m_hWnd = ::CreateWindowExW(0, UPDOWN_CLASSW, L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
		0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(NULL), nullptr);

	if (m_hWnd != nullptr)
	{
		::SendMessageW(m_hWnd, UDM_SETRANGE, TRUE, MAKELONG(usMax, usMin));
		::SendMessageW(m_hWnd, UDM_SETPOS, 0, usMax);
	}

	return m_hWnd != nullptr && m_buddy.getHwnd() != nullptr;
}

long CSpin::getValue() const
{
	/*
	* Document say 'lParam` is pointer to BOOL, which becomes 0 on success, non-zero on failure,
	* but must be NULL on cross-process situation.
	* Historical background being unsure, this tedious error reporting should be left untouched.
	*/

	//if (result)
	//{
	//	BOOL iResult = -1;
	//	long lPosition = ::SendMessage(m_hWnd, UDM_GETPOS32, 0, reinterpret_cast<LPARAM>(&iResult));
	//	if (result != nullptr)*result = (iResult != 0);
	//	return lPosition;
	//}

	return static_cast<long>(::SendMessageW(m_hWnd, UDM_GETPOS32, 0, 0));
}

void CSpin::setValue(long value) const
{
	::SendMessageW(m_hWnd, UDM_SETPOS32, 0, value);
}

HWND CSpin::getBuddyHandle() const
{
	return m_buddy.getHwnd();
}

void CSpin::adjustPosition(int x, int y, int width, int height)
{
	::MoveWindow(m_buddy.getHwnd(), x, y, width, height, TRUE);
	::MoveWindow(m_hWnd, x + width, y, width / 2, height, TRUE);
}

/* ==================== Tab control ==================== */


bool CTab::create(HWND hParentWnd)
{
	m_hWnd = ::CreateWindowExW(0, WC_TABCONTROLW, L"", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hParentWnd, nullptr, ::GetModuleHandle(nullptr), nullptr);
	return m_hWnd != nullptr;
}

bool CTab::add(const wchar_t* name)
{
	if (m_hWnd != nullptr)
	{
		TCITEMW tcItem{};
		tcItem.mask = TCIF_TEXT;
		tcItem.pszText = const_cast<wchar_t*>(name);

		int index = getTabCount();

		return ::SendMessageW(m_hWnd, TCM_INSERTITEM, static_cast<WPARAM>(index), reinterpret_cast<LPARAM>(&tcItem)) != -1;
	}
	return false;
}

int CTab::getTabCount() const
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessageW(m_hWnd, TCM_GETITEMCOUNT, 0, 0);
		return static_cast<int>(lResult);
	}
	return 0;
}

int CTab::getSelectedTabIndex() const
{
	if (m_hWnd != nullptr)
	{
		LRESULT lResult = ::SendMessageW(m_hWnd, TCM_GETCURSEL, 0, 0);
		return static_cast<int>(lResult);
	}
	return -1;
}

void CTab::adjust() const
{
	if (m_hWnd != nullptr)
	{
		RECT rect{};
		::GetClientRect(m_hWnd, &rect);

		::SendMessageW(m_hWnd, TCM_ADJUSTRECT, TRUE, reinterpret_cast<LPARAM>(&rect));
	}
}

int CTab::getItemHeight() const
{
	if (m_hWnd != nullptr)
	{
		RECT rect{};
		::SendMessageW(m_hWnd, TCM_GETITEMRECT, getSelectedTabIndex(), reinterpret_cast<LPARAM>(&rect));
		return (rect.bottom - rect.top);
	}
	return 0;
}
