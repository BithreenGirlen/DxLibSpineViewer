#ifndef DIALOGUE_CONTROLS_H_
#define DIALOGUE_CONTROLS_H_

#include <string>
#include <vector>

#include <Windows.h>
#include <CommCtrl.h>

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

	int FindIndex(const wchar_t* szName);
	bool SetSelectedItem(int iIndex);
private:
	HWND m_hWnd = nullptr;

	void Clear();
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

class CSlider
{
public:
	CSlider();
	~CSlider();

	bool Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, unsigned short usMin, unsigned short usMax, unsigned int uiRange, bool bVertical = false);
	HWND GetHwnd()const { return m_hWnd; }

	long long GetPosition() const;
	void SetPosition(long long llPos) const;

	HWND GetToolTipHandle();
private:
	HWND m_hWnd = nullptr;
};

class CFloatSlider
{
public:
	CFloatSlider();
	~CFloatSlider();

	bool Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, float fMin, float fMax, float fRange, unsigned int uiRatio = kDefaultRatio, bool bVertical = false);
	HWND GetHwnd()const { return m_hWnd; }

	float GetPosition() const;
	void SetPosition(float fPos) const;

	HWND GetToolTipHandle();
	void OnToolTipNeedText(LPNMTTDISPINFOW pNmtTextDispInfo);

	unsigned int GetRatio()const { return m_uiRatio; }
private:
	static constexpr unsigned int kDefaultRatio = 10;
	HWND m_hWnd = nullptr;

	unsigned int m_uiRatio = kDefaultRatio;
};

class CStatic
{
public:
	CStatic();
	~CStatic();

	bool Create(const wchar_t* szText, HWND hParentWnd);
	HWND GetHwnd()const { return m_hWnd; }
private:
	HWND m_hWnd = nullptr;
};

class CEdit
{
public:
	CEdit();
	~CEdit();

	bool Create(const wchar_t* initialText, HWND hParentWnd, bool bReadOnly = false, bool bBorder = true, bool bNumber = false, bool bPassword = false);
	HWND GetHwnd()const { return m_hWnd; }

	std::wstring GetText();
	bool SetText(size_t textLength, const wchar_t* text);
private:
	HWND m_hWnd = nullptr;
};

class CSpin
{
public:
	CSpin();
	~CSpin();

	bool Create(HWND hParentWnd, unsigned short usMin, unsigned short usMax);
	HWND GetHwnd()const { return m_hWnd; }

	long GetValue() const;
	void SetValue(long value) const;

	HWND GetBuddyHandle() const;
	void AdjustPosition(int x, int y, int width, int height);
private:
	HWND m_hWnd = nullptr;
	CEdit m_buddy;
};

#endif // !DIALOGUE_CONTROLS_H_
