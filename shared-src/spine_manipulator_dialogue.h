#ifndef SPINE_MANIPULATOR_DIALOGUE_H_
#define SPINE_MANIPULATOR_DIALOGUE_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "spine_player_shared.h"

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

	HWND m_hApplyButton = nullptr;

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

	CDxLibSpinePlayer* m_pDxLibSpinePlayer = nullptr;
};

#endif // !SPINE_MANIPULATOR_DIALOGUE_H_
