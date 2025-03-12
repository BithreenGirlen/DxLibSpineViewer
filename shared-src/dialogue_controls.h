#ifndef DIALOGUE_CONTROLS_H_
#define DIALOGUE_CONTROLS_H_

#include <string>
#include <vector>

#include <Windows.h>

class CListView
{
public:
	CListView();
	~CListView();

	bool Create(HWND hParentWnd, const std::vector<std::wstring>& columnNames, bool bHasCheckBox = false);
	HWND GetHwnd()const { return m_hWnd; }

	void AdjustWidth();
	bool Add(const std::vector<std::wstring>& columns, bool ToBottom = true);
	void Clear();

	void CreateSingleList(const std::vector<std::wstring>& names);

	std::vector<std::wstring> PickupCheckedItems();
private:
	HWND m_hWnd = nullptr;

	int GetColumnCount();
	int GetItemCount();
	std::wstring GetItemText(int iRow, int iColumn);
};

class CListBox
{
public:
	CListBox();
	~CListBox();

	bool Create(HWND hParentWnd);
	HWND GetHwnd()const { return m_hWnd; }

	void Add(const wchar_t* szText, bool ToBottom = true);
	void Clear();
	std::wstring GetSelectedItemName();
private:
	HWND m_hWnd = nullptr;

	long long GetSelectedItemIndex();
};

class CComboBox
{
public:
	CComboBox();
	~CComboBox();

	bool Create(HWND hParentWnd);
	HWND GetHwnd()const { return m_hWnd; }

	void Setup(const std::vector<std::wstring>& itemTexts);

	int GetSelectedItemIndex();
	std::wstring GetSelectedItemText();
private:
	HWND m_hWnd = nullptr;

	void Clear();
	bool SetSelectedItem(int iIndex);
};

class CButton
{
public:
	CButton();
	~CButton();

	bool Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, bool bHasCheckBox = false);
	HWND GetHwnd()const { return m_hWnd; }

	void SetCheckBox(bool bToBeChecked);
	bool IsChecked();
private:
	HWND m_hWnd = nullptr;
};

#endif // !DIALOGUE_CONTROLS_H_
