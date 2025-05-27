#ifndef SPINE_MANIPULATOR_DIALOGUE_H_
#define SPINE_MANIPULATOR_DIALOGUE_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "spine_player_shared.h"
#include "dialogue_controls.h"

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
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDblClk(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16 };
	enum Controls
	{
		kApplyButton = 1, 
	};

	HFONT m_hFont = nullptr;

	CListView m_slotListView;
	CListView m_skinListView;
	CListView m_animationListView;

	CButton m_applyButton;

	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	void OnApplyButton();

	LRESULT ResizeControls();

	CDxLibSpinePlayer* m_pDxLibSpinePlayer = nullptr;
};

#endif // !SPINE_MANIPULATOR_DIALOGUE_H_
