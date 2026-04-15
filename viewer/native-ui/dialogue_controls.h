#ifndef DIALOGUE_CONTROLS_H_
#define DIALOGUE_CONTROLS_H_

#include <string>
#include <vector>

#include <Windows.h>
#include <CommCtrl.h>

class CListView
{
public:
	CListView() = default;
	~CListView() = default;

	bool create(HWND hParentWnd, const wchar_t** columnNames, size_t columnCount, bool hasCheckBox = false);
	template <size_t columnCount>
	void create(HWND hParentWnd, const wchar_t*(&&columnNames)[columnCount], bool hasCheckBox = false)
	{
		create(hParentWnd, columnNames, columnCount, hasCheckBox);
	}

	HWND getHwnd()const { return m_hWnd; }

	void adjustWidth();

	bool add(const wchar_t** columns, size_t columnCount, bool toBottom = true);
	template<size_t columnCount>
	bool add(const wchar_t*(&&columns)[columnCount], bool toBottom = true)
	{
		add(columns, columnCount, toBottom);
	}
	bool add(const std::vector<std::wstring>& columns, bool toBottom = true);

	void clear() const;

	void createSingleList(const std::vector<std::wstring>& items);
	void createSingleList(const wchar_t** items, size_t itemCount);

	std::vector<std::wstring> pickupCheckedItems();
private:
	HWND m_hWnd = nullptr;

	int getColumnCount() const;
	int getItemCount() const;
	std::wstring getItemText(int iRow, int iColumn) const;
};

class CListBox
{
public:
	CListBox() = default;
	~CListBox() = default;

	bool create(HWND hParentWnd);
	HWND getHwnd()const { return m_hWnd; }

	void add(const wchar_t* text, bool toBottom = true) const;
	void clear() const;
	std::wstring getSelectedItemName();
private:
	HWND m_hWnd = nullptr;

	long long getSelectedItemIndex() const;
};

class CComboBox
{
public:
	CComboBox() = default;
	~CComboBox() = default;

	bool create(HWND hParentWnd);
	HWND getHwnd()const { return m_hWnd; }

	void setup(const std::vector<std::wstring>& itemTexts);
	void setup(const wchar_t** itemTexts, size_t itemCount);
	template<size_t itemCount>
	void setup(const wchar_t* (&& itemTexts)[itemCount])
	{
		setup(itemTexts, itemCount);
	}

	int getSelectedItemIndex() const;
	std::wstring getSelectedItemText() const;

	int findIndex(const wchar_t* itemName) const;
	bool setSelectedItem(int itemIndex) const;
private:
	HWND m_hWnd = nullptr;

	void clear() const;
};

class CButton
{
public:
	CButton() = default;
	~CButton() = default;

	bool create(const wchar_t* text, HWND hParentWnd, HMENU hMenu, bool hasCheckBox = false);
	HWND getHwnd()const { return m_hWnd; }

	void setCheckBox(bool checked) const;
	bool isChecked() const;
private:
	HWND m_hWnd = nullptr;
};

class CSlider
{
public:
	CSlider() = default;
	~CSlider() = default;

	bool create(const wchar_t* text, HWND hParentWnd, HMENU hMenu, unsigned short usMin, unsigned short usMax, unsigned int uiRange, bool toBeVertical = false);
	HWND getHwnd()const { return m_hWnd; }

	long long getPosition() const;
	void setPosition(long long llPos) const;

	HWND getToolTipHandle() const;
private:
	HWND m_hWnd = nullptr;
};

class CFloatSlider
{
public:
	CFloatSlider() = default;
	~CFloatSlider() = default;

	bool create(const wchar_t* text, HWND hParentWnd, HMENU hMenu, float fMin, float fMax, float fRange, unsigned int uiRatio = kDefaultRatio, bool toBeVertical = false);
	HWND getHwnd()const { return m_hWnd; }

	float getPosition() const;
	void setPosition(float fPos) const;

	HWND getToolTipHandle() const;
	void onToolTipNeedText(LPNMTTDISPINFOW pNmtTextDispInfo) const;

	unsigned int getRatio()const { return m_uiRatio; }
private:
	static constexpr unsigned int kDefaultRatio = 10;
	HWND m_hWnd = nullptr;

	unsigned int m_uiRatio = kDefaultRatio;
};

class CStatic
{
public:
	CStatic() = default;
	~CStatic() = default;

	bool create(const wchar_t* text, HWND hParentWnd, bool hasEdge = false);
	HWND getHwnd()const { return m_hWnd; }
private:
	HWND m_hWnd = nullptr;
};

class CEdit
{
public:
	CEdit() = default;
	~CEdit() = default;

	bool create(const wchar_t* initialText, HWND hParentWnd, bool toBeReadOnly = false, bool hasBorder = true, bool onlyDigits = false, bool passwordField = false);
	HWND getHwnd()const { return m_hWnd; }

	std::wstring getText() const;
	bool setText(size_t textLength, const wchar_t* text) const;
	
	bool setHint(const wchar_t* text, bool toBeHidden = true) const;
private:
	HWND m_hWnd = nullptr;
};

class CSpin
{
public:
	CSpin() = default;
	~CSpin() = default;

	bool create(HWND hParentWnd, unsigned short usMin, unsigned short usMax);
	HWND getHwnd()const { return m_hWnd; }

	long getValue() const;
	void setValue(long value) const;

	HWND getBuddyHandle() const;
	void adjustPosition(int x, int y, int width, int height);
private:
	HWND m_hWnd = nullptr;
	CEdit m_buddy;
};

class CTab
{
public:
	CTab() = default;
	~CTab() = default;

	bool create(HWND hParentWnd);
	HWND getHwnd()const { return m_hWnd; }

	bool add(const wchar_t* name);
	int getTabCount() const;
	int getSelectedTabIndex() const;

	void adjust() const;
	int getItemHeight() const;

private:
	HWND m_hWnd = nullptr;
};

#endif // !DIALOGUE_CONTROLS_H_
