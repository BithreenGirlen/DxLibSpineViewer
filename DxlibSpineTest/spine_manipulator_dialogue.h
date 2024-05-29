#ifndef SPINE_MANIPULATOR_DIALOGUE_H_
#define SPINE_MANIPULATOR_DIALOGUE_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "dxlib_spine_player.h"

class CSpineManipulatorDialogue
{
public:
	CSpineManipulatorDialogue();
	~CSpineManipulatorDialogue();
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, CDxLibSpinePlayer* pPlayer);
	HWND GetHwnd()const { return m_hWnd; }
private:
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnInit(HWND hWnd);
	LRESULT OnClose();
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16 };
	enum Controls
	{
		kApplyButton = 1, 
	};

	std::wstring m_wstrWindowName = L"Dialogue";
	const std::vector<std::wstring> m_slotColumnNames = { L"Slots to exclude"};
	const std::vector<std::wstring> m_skinColumnNames = { L"Skins to mix"};
	const std::vector<std::wstring> m_animationColumnNames = { L"Animations to mix" };

	HFONT m_hFont = nullptr;

	HWND m_hSlotListView = nullptr;
	HWND m_hSkinListView = nullptr;
	HWND m_hAnimationListView = nullptr;

	HWND n_hApplyButton = nullptr;

	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	void OnApplyButton();

	LRESULT ResizeControls();

	void CreateListView(HWND* pListViewHandle, const std::vector<std::wstring>& columnNames);
	void AdjustListViewWidth(HWND hListView, int iColumnCount);
	int GetListViewItemCount(HWND hListView);
	bool AddItemToListView(HWND hListView, const std::vector<std::wstring>& columns, bool bToEnd);
	void ClearListView(HWND hListView);
	void CreateSingleList(HWND hListView, const std::vector<std::wstring>& names);
	std::wstring GetListViewItemText(HWND hListView, int iRow, int iColumn);

	/*
	* Dialogue template with no controls.
	* https://learn.microsoft.com/en-us/windows/win32/dlgbox/dlgtemplateex
	*/
	const unsigned char m_DialogueTemplate[76] =
	{
		0x01, 0x00, // dlgVer
		0xff, 0xff, // signature
		0x00, 0x00, 0x00, 0x00, // helpID
		0x00, 0x00, 0x00, 0x00, // exStyle
		0xc8, 0x00, 0xc8, 0x80, // style
		0x00, 0x00, // cDlgItems
		0x00, 0x00, // x
		0x00, 0x00, // y
		0x80, 0x00, // cx
		0xf0, 0x00, // cy
		0x00, 0x00, // menu
		0x00, 0x00, // windowClass
		0x44, 0x00, 0x69, 0x00, 0x61, 0x00, 0x6c, 0x00, // title[7]; here "Dialog"
		0x6f, 0x00, 0x67, 0x00, 0x00, 0x00,
		0x08, 0x00, // pointsize
		0x90, 0x01, // weight
		0x00, 0x01, // italic
		0x4d, 0x00, // characterset
		0x53, 0x00, 0x20, 0x00, 0x53, 0x00, 0x68, 0x00, // typeface[13]; here "MS Shell Dlg"
		0x65, 0x00, 0x6c, 0x00, 0x6c, 0x00, 0x20, 0x00,
		0x44, 0x00, 0x6c, 0x00, 0x67, 0x00, 0x00, 0x00,
	};

	CDxLibSpinePlayer* m_pDxLibSpinePlayer = nullptr;
};

#endif // !SPINE_MANIPULATOR_DIALOGUE_H_
